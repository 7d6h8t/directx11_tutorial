#pragma once
#include <cstdint>

class InputClass {
 public:
  InputClass();
  InputClass(const InputClass& rhs);
  ~InputClass();

  void Initialize();

  void KeyDown(const uint32_t input);
  void KeyUp(const uint32_t input);

  bool IsKeyDown(const uint32_t key);

 private:
  bool keys_[256];
};