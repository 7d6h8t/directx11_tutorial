#include "pch.h"
#include "input_class.h"

void InputClass::Initialize() {
  for (int32_t i = 0; i < 256; i++) keys_[i] = false;
}

void InputClass::KeyDown(const uint32_t input) { keys_[input] = true; }

void InputClass::KeyUp(const uint32_t input) { keys_[input] = false; }

bool InputClass::IsKeyDown(const uint32_t key) { return keys_[key]; }
