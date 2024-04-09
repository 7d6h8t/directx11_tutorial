#include "pch.h"
#include "graphics_class.h"

#include "d3d_class.h"
#include "color_shader_class.h"

bool GraphicsClass::Initialize(const int32_t width, const int32_t height,
                               HWND hwnd) {
  d3d_ = new D3DClass{};
  if (d3d_ == nullptr) return false;

  if (d3d_->Initialize(width, height, VSYNC_ENABLED, hwnd, FULL_SCREEN,
                       SCREEN_DEPTH, SCREEN_NEAR) == false) {
    ::MessageBox(hwnd, L"Could not initialzie Direct3D", L"Error", MB_OK);
    return false;
  }

  color_shader_ = new ColorShaderClass{};
  if (color_shader_ == nullptr) return false;

  if (color_shader_->Initialize(d3d_->GetDevice(), hwnd) == false) {
    ::MessageBox(hwnd, L"Could not initialzie the color shader object.", L"Error", MB_OK);
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

  if (color_shader_) {
    color_shader_->Shutdown();
    delete color_shader_;
    color_shader_ = nullptr;
  }
}

bool GraphicsClass::Frame() { return Render(); }

bool GraphicsClass::Render() {
  d3d_->BeginScene(0.5f, 0.5f, 0.5f, 1.0f);

  d3d_->EndScene();
  return true;
}
