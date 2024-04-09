#include "pch.h"
#include "graphics_class.h"

#include "d3d_class.h"
#include "camera_class.h"
#include "model_class.h"
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

  camera_ = new CameraClass{};
  if (camera_ == nullptr) return false;
  camera_->SetPosition(0.0f, 0.0f, -5.0f);

  model_ = new ModelClass{};
  if (model_ == nullptr) return false;
  if (model_->Initialize(d3d_->GetDevice()) == false) {
    MessageBox(hwnd, L"Could not initialize the model object.", L"Error",
               MB_OK);
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

  if (model_) {
    model_->Shutdown();
    delete model_;
    model_ = nullptr;
  }

  if (camera_) {
    delete camera_;
    camera_ = nullptr;
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

  // 카메라의 위치에 따라 뷰 행렬을 생성합니다
  camera_->Render();

  // 카메라 및 d3d 객체에서 월드, 뷰 및 투영 행렬을 가져옵니다
  DirectX::XMMATRIX world_matrix{}, view_matrix{}, projection_matrix{};
  d3d_->GetWorldMatrix(world_matrix);
  camera_->GetViewMatrix(view_matrix);
  d3d_->GetProjectionMatrix(projection_matrix);

  // 모델 정점, 인덱스 버퍼를 그래픽 파이프 라인에 배치하여 드로잉을 준비합니다
  model_->Render(d3d_->GetDeviceContext());

  // 색상 쉐이더를 사용하여 모델을 렌더링합니다
  color_shader_->Render(d3d_->GetDeviceContext(), model_->GetIndexCount(),
                        world_matrix, view_matrix, projection_matrix);

  d3d_->EndScene();
  return true;
}
