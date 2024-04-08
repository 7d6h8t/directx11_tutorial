#include "pch.h"
#include "graphics_class.h"

GraphicsClass::GraphicsClass() {}

GraphicsClass::GraphicsClass(const GraphicsClass& rhs) {}

GraphicsClass::~GraphicsClass() {}

bool GraphicsClass::Initialize(const int32_t width, const int32_t height,
                               HWND hwnd) {
  d3d_ = new D3DClass{};
  if (d3d_ == nullptr) return false;

  bool result = d3d_->Initialize(width, height, VSYNC_ENABLED, hwnd,
                                 FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
  if (result == false) {
    ::MessageBox(hwnd, L"Could not initialzie Direct3D", L"Error", MB_OK);
    return false;
  }
  return true;
}

void GraphicsClass::Shutdown() {
  if (d3d_) {
    d3d_->Shutdown();
    delete d3d_;
    d3d_ = nullptr;
  }
}

bool GraphicsClass::Frame() { return Render() ? true : false; }

bool GraphicsClass::Render() {
  d3d_->BeginScene(0.5f, 0.5f, 0.5f, 1.0f);
  d3d_->EndScene();
  return true;
}
