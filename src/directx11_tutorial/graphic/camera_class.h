#pragma once
#include <DirectXMath.h>

class CameraClass {
 public:
  void SetPosition(const float x, const float y, const float z);
  void SetRotation(const float x, const float y, const float z);

  DirectX::XMFLOAT3 GetPosition();
  DirectX::XMFLOAT3 GetRotation();

  void Render();
  void GetViewMatrix(DirectX::XMMATRIX& view_matrix);

 private:
  DirectX::XMFLOAT3 position_{0.0f, 0.0f, 0.0f};
  DirectX::XMFLOAT3 rotation_{0.0f, 0.0f, 0.0f};
  DirectX::XMMATRIX view_matrix_;
};
