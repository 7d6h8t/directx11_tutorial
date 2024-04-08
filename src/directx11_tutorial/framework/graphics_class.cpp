#include "pch.h"
#include "graphics_class.h"

GraphicsClass::GraphicsClass() {}

GraphicsClass::GraphicsClass(const GraphicsClass& rhs) {}

GraphicsClass::~GraphicsClass() {}

bool GraphicsClass::Initialize(const int32_t width, const int32_t height,
                               HWND hwnd) {
  return true;
}

void GraphicsClass::Shutdown() {}

bool GraphicsClass::Frame() { return true; }

bool GraphicsClass::Render() { return true; }
