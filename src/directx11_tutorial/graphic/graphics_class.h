#pragma once
#include <cstdint>

#include "d3d_class.h"

// GLOBALS
const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 0.1f;

class GraphicsClass {
 public:
  GraphicsClass();
  GraphicsClass(const GraphicsClass& rhs);
  ~GraphicsClass();

  bool Initialize(const int32_t width, const int32_t height, HWND hwnd);
  void Shutdown();
  bool Frame();

 private:
  bool Render();

  D3DClass* d3d_ = nullptr;
};