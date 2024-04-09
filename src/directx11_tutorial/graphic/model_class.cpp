#include "pch.h"
#include "model_class.h"

#include "com_throw.h"

bool ModelClass::Initialize(ID3D11Device* device) {
  // 정점 및 인덱스 버퍼를 초기화합니다
  return InitializeBuffers(device);
}

void ModelClass::Shutdown() {
  ShutdownBuffers();
}

void ModelClass::Render(ID3D11DeviceContext* device_context) {
  // 그리기를 준비하기 위해 파이프 라인에 정점, 인덱스 버퍼를 놓습니다.
  RenderBuffers(device_context);
}

int ModelClass::GetIndexCount() { return index_count_; }

bool ModelClass::InitializeBuffers(ID3D11Device* device) {
  // 정점 배열의 정점 수를 설정합니다
  vertex_count_ = 3;

  // 인덱스 배열의 인덱스 수를 설정합니다
  index_count_ = 3;

  // 정점 배열을 만듭니다.
  VertexType* vertices = new VertexType[vertex_count_];
  if (vertices == nullptr) return false;

  // 정점 배열에 데이터를 설정합니다
  vertices[0].position_ =
      DirectX::XMFLOAT3(-1.0f, -1.0f, 0.0f);  // Bottom left.
  vertices[0].color_ = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

  vertices[1].position_ = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);  // Top middle.
  vertices[1].color_ = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

  vertices[2].position_ =
      DirectX::XMFLOAT3(1.0f, -1.0f, 0.0f);  // Bottom right.
  vertices[2].color_ = DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);

  // 인덱스 배열을 만듭니다
  uint32_t* indices = new uint32_t[index_count_];
  if (!indices) return false;

  // 인덱스 배열의 값을 설정합니다
  indices[0] = 0;  // Bottom left.
  indices[1] = 1;  // Top middle.
  indices[2] = 2;  // Bottom right.

  // 정적 정점 버퍼의 description 을 설정합니다
  D3D11_BUFFER_DESC vertex_buffer_desc{};
  vertex_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
  vertex_buffer_desc.ByteWidth = sizeof(VertexType) * vertex_count_;
  vertex_buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  vertex_buffer_desc.CPUAccessFlags = 0;
  vertex_buffer_desc.MiscFlags = 0;
  vertex_buffer_desc.StructureByteStride = 0;

  // subresource 구조에 정점 데이터에 대한 포인터를 제공합니다
  D3D11_SUBRESOURCE_DATA vertex_data;
  vertex_data.pSysMem = vertices;
  vertex_data.SysMemPitch = 0;
  vertex_data.SysMemSlicePitch = 0;

  // 이제 정점 버퍼를 만듭니다
  com::ThrowIfFailed(
      device->CreateBuffer(&vertex_buffer_desc, &vertex_data, &vertex_buffer_));

  // 정적 인덱스 버퍼의 description 을 설정합니다
  D3D11_BUFFER_DESC index_buffer_desc{};
  index_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
  index_buffer_desc.ByteWidth = sizeof(unsigned long) * index_count_;
  index_buffer_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
  index_buffer_desc.CPUAccessFlags = 0;
  index_buffer_desc.MiscFlags = 0;
  index_buffer_desc.StructureByteStride = 0;

  // 인덱스 데이터를 가리키는 subresource 를 작성합니다
  D3D11_SUBRESOURCE_DATA index_data;
  index_data.pSysMem = indices;
  index_data.SysMemPitch = 0;
  index_data.SysMemSlicePitch = 0;

  // 인덱스 버퍼를 생성합니다
  com::ThrowIfFailed(
      device->CreateBuffer(&index_buffer_desc, &index_data, &index_buffer_));

  // 생성되고 값이 할당된 정점 버퍼와 인덱스 버퍼를 해제합니다
  delete[] vertices;
  vertices = nullptr;

  delete[] indices;
  indices = nullptr;

  return true;
}

void ModelClass::ShutdownBuffers() {
  // 인덱스 버퍼를 해제합니다.
  if (index_buffer_) {
    index_buffer_->Release();
    index_buffer_ = nullptr;
  }

  // 정점 버퍼를 해제합니다.
  if (vertex_buffer_) {
    vertex_buffer_->Release();
    vertex_buffer_ = nullptr;
  }
}

void ModelClass::RenderBuffers(ID3D11DeviceContext* device_context) {
  // 정점 버퍼의 단위와 오프셋을 설정합니다.
  uint32_t stride = sizeof(VertexType);
  uint32_t offset = 0;

  // 렌더링 할 수 있도록 Input Assembler 에서 정점 버퍼를 활성으로 설정합니다.
  device_context->IASetVertexBuffers(0, 1, &vertex_buffer_, &stride, &offset);

  // 렌더링 할 수 있도록 Input Assembler 에서 인덱스 버퍼를 활성으로 설정합니다.
  device_context->IASetIndexBuffer(index_buffer_, DXGI_FORMAT_R32_UINT, 0);

  // 정점 버퍼로 그릴 기본형을 설정합니다. 여기서는 삼각형으로 설정합니다.
  device_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}
