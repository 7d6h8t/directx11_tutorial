#pragma once
#include <cstdint>

class InputClass;
class GraphicsClass;

class SystemClass {
 public:
  bool Initialize();
  void Shutdown();
  void Run();

  LRESULT CALLBACK MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam,
                                  LPARAM lparam);

 private:
  bool Frame();
  void InitialzieWindows(int32_t& width, int32_t& height);
  void ShutdownWindows();

  LPCWSTR app_name_;
  HINSTANCE hinstance_;
  HWND hwnd_;

  InputClass* input_ = nullptr;
  GraphicsClass* graphics_ = nullptr;
};

static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wparam,
                                LPARAM lparam);

static SystemClass* application_handle = nullptr;