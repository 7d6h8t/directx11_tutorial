#include "pch.h"
#include "system_class.h"

#include "input_class.h"
#include "graphic/graphics_class.h"

bool SystemClass::Initialize() {
  int32_t width = 0, height = 0;
  InitialzieWindows(width, height);

  input_ = new InputClass{};
  if (input_ == nullptr) return false;

  input_->Initialize();

  graphics_ = new GraphicsClass{};
  if (graphics_ == nullptr) return false;

  return graphics_->Initialize(width, height, hwnd_);
}

void SystemClass::Shutdown() {
  if (graphics_) {
    graphics_->Shutdown();
    delete graphics_;
    graphics_ = nullptr;
  }

  if (input_) {
    delete input_;
    input_ = nullptr;
  }

  ShutdownWindows();
}

void SystemClass::Run() {
  MSG msg{};

  while (true) {
    if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) break;

      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
    } else {
      if (Frame() == false) break;
    }
  }

  return;
}

LRESULT CALLBACK SystemClass::MessageHandler(HWND hwnd, UINT umsg,
                                             WPARAM wparam, LPARAM lparam) {
  switch (umsg) {
    case WM_KEYDOWN:
      input_->KeyDown(static_cast<uint32_t>(wparam));
      return 0;

    case WM_KEYUP:
      input_->KeyUp(static_cast<uint32_t>(wparam));
      return 0;

    default:
      return ::DefWindowProc(hwnd, umsg, wparam, lparam);
  }
}

bool SystemClass::Frame() {
  if (input_->IsKeyDown(VK_ESCAPE)) return false;

  return graphics_->Frame();
}

void SystemClass::InitialzieWindows(int32_t& width, int32_t& height) {
  application_handle = this;
  hinstance_ = ::GetModuleHandle(nullptr);
  app_name_ = L"Engine";

  WNDCLASSEX wc{};
  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc = WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hinstance_;
  wc.hIcon = ::LoadIcon(nullptr, IDI_WINLOGO);
  wc.hIconSm = wc.hIcon;
  wc.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
  wc.lpszMenuName = nullptr;
  wc.lpszClassName = app_name_;
  wc.cbSize = sizeof(WNDCLASSEX);

  ::RegisterClassEx(&wc);

  // 모니터 화면의 해상도를 읽어옵니다.
  width = ::GetSystemMetrics(SM_CXSCREEN);
  height = ::GetSystemMetrics(SM_CYSCREEN);

  int32_t pos_x = 0, pos_y = 0;

  if (FULL_SCREEN) {
    // 풀스크린 모드로 지정했다면 모니터 화면 해상도를 데스트톱 해상도로
    // 지정하고 색상을 32bit로 지정합니다.
    DEVMODE screen_setting{};
    screen_setting.dmSize = sizeof(screen_setting);
    screen_setting.dmPelsWidth = static_cast<unsigned long>(width);
    screen_setting.dmPelsHeight = static_cast<unsigned long>(height);
    screen_setting.dmBitsPerPel = 32;
    screen_setting.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

    // 풀스크린으로 디스플레이 설정을 변경합니다.
    ::ChangeDisplaySettings(&screen_setting, CDS_FULLSCREEN);
  } else {
    // 윈도우 모드의 경우 800*600 크기를 지정합니다.
    width = 800;
    height = 600;

    // 윈도우 창을 가로, 세로의 정 가운데 오도록 합니다.
    pos_x = (::GetSystemMetrics(SM_CXSCREEN) - width) / 2;
    pos_y = (::GetSystemMetrics(SM_CYSCREEN) - height) / 2;
  }

  hwnd_ = ::CreateWindowEx(WS_EX_APPWINDOW, app_name_, app_name_,
                           WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP, pos_x,
                           pos_y, width, height, nullptr, nullptr, hinstance_,
                           nullptr);

  ::ShowWindow(hwnd_, SW_SHOW);
  ::SetForegroundWindow(hwnd_);
  ::SetFocus(hwnd_);
}

void SystemClass::ShutdownWindows() {
  if (FULL_SCREEN)
    ::ChangeDisplaySettings(nullptr, 0);

  DestroyWindow(hwnd_);
  hwnd_ = nullptr;

  ::UnregisterClass(app_name_, hinstance_);
  hinstance_ = nullptr;

  application_handle = nullptr;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam,
                         LPARAM lparam) {
  switch (message) {
    case WM_DESTROY:
      PostQuitMessage(0);
      return 0;

    case WM_CLOSE:
      PostQuitMessage(0);
      return 0;

    default:
      return application_handle->MessageHandler(hwnd, message, wparam, lparam);
  }
}
