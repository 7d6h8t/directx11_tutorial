#include "pch.h"
#include "camera_class.h"

void CameraClass::SetPosition(const float x, const float y, const float z) {
  position_.x = x;
  position_.y = y;
  position_.z = z;
}

void CameraClass::SetRotation(const float x, const float y, const float z) {
  rotation_.x = x;
  rotation_.y = y;
  rotation_.z = z;
}

DirectX::XMFLOAT3 CameraClass::GetPosition() { return position_; }

DirectX::XMFLOAT3 CameraClass::GetRotation() { return rotation_; }

void CameraClass::Render() {
  DirectX::XMFLOAT3 up{}, position{}, look_at{};
  DirectX::XMVECTOR up_vector{}, position_vector{}, look_at_vector{};
  float yaw = 0, pitch = 0, roll = 0;
  DirectX::XMMATRIX rotation_matrix{};

  // 위쪽을 가리키는 벡터를 설정합니다
  up.x = 0.0f;
  up.y = 1.0f;
  up.z = 0.0f;

  // XMVECTOR 구조체에 로드한다
  up_vector = XMLoadFloat3(&up);

  // 3D월드에서 카메라의 위치를 ​​설정합니다
  position = position_;

  // XMVECTOR 구조체에 로드한다
  position_vector = XMLoadFloat3(&position);

  // 기본적으로 카메라가 찾고있는 위치를 설정합니다
  look_at.x = 0.0f;
  look_at.y = 0.0f;
  look_at.z = 1.0f;

  // XMVECTOR 구조체에 로드한다
  look_at_vector = XMLoadFloat3(&look_at);

  // yaw (Y 축), pitch (X 축) 및 roll (Z 축)의 회전값을 라디안 단위로
  // 설정합니다
  pitch = rotation_.x * 0.0174532925f;
  yaw = rotation_.y * 0.0174532925f;
  roll = rotation_.z * 0.0174532925f;

  //  yaw, pitch, roll 값을 통해 회전 행렬을 만듭니다
  rotation_matrix = DirectX::XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

  // look_at 및 up 벡터를 회전 행렬로 변형하여 뷰가 원점에서 올바르게 회전되도록
  // 합니다
  look_at_vector = XMVector3TransformCoord(look_at_vector, rotation_matrix);
  up_vector = XMVector3TransformCoord(up_vector, rotation_matrix);

  // 회전 된 카메라 위치를 뷰어 위치로 변환합니다
  look_at_vector = DirectX::XMVectorAdd(position_vector, look_at_vector);

  // 마지막으로 세 개의 업데이트 된 벡터에서 뷰 행렬을 만듭니다.
  view_matrix_ =
      DirectX::XMMatrixLookAtLH(position_vector, look_at_vector, up_vector);
}

void CameraClass::GetViewMatrix(DirectX::XMMATRIX& view_matrix) {
  view_matrix = view_matrix_;
}
