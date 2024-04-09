#pragma once
#include <cstdint>

class InputClass {
 public:
  void Initialize();

  void KeyDown(const uint32_t input);
  void KeyUp(const uint32_t input);

  bool IsKeyDown(const uint32_t key);

 private:
  bool keys_[256];
};