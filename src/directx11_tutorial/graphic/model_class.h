#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

class ModelClass {
 public:
  bool Initialize(ID3D11Device* device);
  void Shutdown();
  void Render(ID3D11DeviceContext* device_context);

  int GetIndexCount();

 private:
  struct VertexType {
    DirectX::XMFLOAT3 position_;
    DirectX::XMFLOAT4 color_;
  };

  bool InitializeBuffers(ID3D11Device* device);
  void ShutdownBuffers();
  void RenderBuffers(ID3D11DeviceContext* device_context);

 private:
  ID3D11Buffer* vertex_buffer_ = nullptr;
  ID3D11Buffer* index_buffer_ = nullptr;
  int32_t vertex_count_ = 0;
  int32_t index_count_ = 0;
};
