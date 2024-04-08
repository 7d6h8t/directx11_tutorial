#include "pch.h"
#include "system_class.h"

SystemClass::SystemClass() {
  input_ = 0;
  graphics_ = 0;
}

SystemClass::SystemClass(const SystemClass& rhs) {}

SystemClass::~SystemClass() {}

bool SystemClass::Initialize() {
  int32_t width = 0, height = 0;
  bool result = false;

  InitialzieWindows(width, height);

  input_ = new InputClass{};
  if (input_ == nullptr) return false;

  input_->Initialize();

  graphics_ = new GraphicsClass{};
  if (graphics_ == nullptr) return false;

  return graphics_->Initialize(width, height, hwnd_) ? true : false;
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
  bool done = false, result = false;

  while (done == false) {
    if (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
    }

    if (msg.message == WM_QUIT) {
      done = true;
    } else {
      if (Frame() == false) done = true;
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

  return graphics_->Frame() ? true : false;
}

void SystemClass::InitialzieWindows(int32_t& width, int32_t& height) {
  WNDCLASSEX wc{};
  DEVMODE screen_setting{};
  int32_t pos_x = 0, pos_y = 0;

  application_handle = this;
  hinstance_ = GetModuleHandle(nullptr);
  app_name_ = L"Engine";

  wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wc.lpfnWndProc = WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hinstance_;
  wc.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
  wc.hIconSm = wc.hIcon;
  wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(BLACK_BRUSH));
  wc.lpszMenuName = nullptr;
  wc.lpszClassName = app_name_;
  wc.cbSize = sizeof(WNDCLASSEX);

  ::RegisterClassEx(&wc);

  width = ::GetSystemMetrics(SM_CXSCREEN);
  height = ::GetSystemMetrics(SM_CYSCREEN);

  if (FULL_SCREEN) {
    memset(&screen_setting, 0, sizeof(screen_setting));
    screen_setting.dmSize = sizeof(screen_setting);
    screen_setting.dmPelsWidth = static_cast<unsigned long>(width);
    screen_setting.dmPelsHeight = static_cast<unsigned long>(height);
    screen_setting.dmBitsPerPel = 32;
    screen_setting.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

    ::ChangeDisplaySettings(&screen_setting, CDS_FULLSCREEN);
    pos_x = pos_y = 0;
  } else {
    width = 800;
    height = 600;

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

  ::ShowCursor(false);
}

void SystemClass::ShutdownWindows() {
  ::ShowCursor(true);

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