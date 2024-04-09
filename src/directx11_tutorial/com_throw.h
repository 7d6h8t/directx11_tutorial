#pragma once
#include <winerror.h>

#include <exception>
#include <format>

namespace com {
// Helper class for COM exceptions
class com_exception : public std::exception {
 public:
  com_exception(HRESULT hr) : result(hr) {}

  const char* what() const noexcept override {
    std::string s_str = std::format("Failure with HRESULT of {:08X}",
                                    static_cast<unsigned int>(result));

    return s_str.length() > 0 ? s_str.c_str() : "Unknown exception";
  }

 private:
  HRESULT result;
};

// Helper utility converts D3D API failures into exceptions.
inline void ThrowIfFailed(HRESULT hr) {
  if (FAILED(hr)) {
    throw com_exception(hr);
  }
}
}  // namespace com
