#pragma once
#include <cstdint>

// GLOBALS
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class D3DClass;
class ColorShaderClass;

class GraphicsClass {
 public:
  bool Initialize(const int32_t width, const int32_t height, HWND hwnd);
  void Shutdown();
  bool Frame();

 private:
  bool Render();

  D3DClass* d3d_ = nullptr;
  ColorShaderClass* color_shader_ = nullptr;
};
