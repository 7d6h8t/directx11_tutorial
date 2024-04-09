#include "pch.h"
#include "color_shader_class.h"

#include <d3dcompiler.h>

#include "com_throw.h"

bool ColorShaderClass::Initialize(ID3D11Device* device, const HWND hwnd) {
  // 정점 및 픽셀 셰이더를 초기화 합니다
  return InitializeShader(device, hwnd, L"shader/vertex.hlsl", L"shader/pixel.hlsl");
}

void ColorShaderClass::Shutdown() { ShutdownShader(); }

void ColorShaderClass::Render(ID3D11DeviceContext* device_context,
                              const int32_t index_count,
                              DirectX::XMMATRIX world, DirectX::XMMATRIX view,
                              DirectX::XMMATRIX projection) {
  // 렌더링에 사용할 셰이더 매개 변수를 설정합니다
  SetShaderParameters(device_context, world, view, projection);

  // 설정된 버퍼를 셰이더로 렌더링합니다
  RenderShader(device_context, index_count);
}

bool ColorShaderClass::InitializeShader(ID3D11Device* device, const HWND hwnd,
                                        const std::filesystem::path& vs_path,
                                        const std::filesystem::path& ps_path) {
  ID3DBlob* error_message = nullptr;

  // 정점 셰이더 코드를 컴파일한다
  ID3DBlob* vertex_shader_buffer = nullptr;
  if (FAILED(D3DCompileFromFile(vs_path.c_str(), nullptr, nullptr,
                                "ColorVertexShader", "vs_5_0",
                                D3D10_SHADER_ENABLE_STRICTNESS, 0,
                                &vertex_shader_buffer, &error_message))) {
    if (error_message)
      OutputShaderErrorMessage(error_message, hwnd, vs_path);
    else
      MessageBox(hwnd, vs_path.c_str(), L"Missing Shader File", MB_OK);

    return false;
  }

  // 픽셀 셰이더 코드를 컴파일한다
  ID3DBlob* pixel_shader_buffer = nullptr;
  if (FAILED(D3DCompileFromFile(ps_path.c_str(), nullptr, nullptr,
                                "ColorPixelShader", "ps_5_0",
                                D3D10_SHADER_ENABLE_STRICTNESS, 0,
                                &pixel_shader_buffer, &error_message))) {
    if (error_message)
      OutputShaderErrorMessage(error_message, hwnd, ps_path);
    else
      MessageBox(hwnd, ps_path.c_str(), L"Missing Shader File", MB_OK);

    return false;
  }

  // 버퍼로부터 정점 셰이더를 생성한다
  com::ThrowIfFailed(device->CreateVertexShader(
      vertex_shader_buffer->GetBufferPointer(),
      vertex_shader_buffer->GetBufferSize(), nullptr, &vertex_shader_));

  // 버퍼로부터 픽셀 셰이더를 생성한다
  com::ThrowIfFailed(device->CreatePixelShader(
      pixel_shader_buffer->GetBufferPointer(),
      pixel_shader_buffer->GetBufferSize(), nullptr, &pixel_shader_));

  // 정점 input layout description을 설정합니다
  // 이 설정은 ModelClass 와 셰이더의 VertexType 구조와 일치해야합니다
  D3D11_INPUT_ELEMENT_DESC polygon_layout[2]{};
  polygon_layout[0].SemanticName = "POSITION";
  polygon_layout[0].SemanticIndex = 0;
  polygon_layout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  polygon_layout[0].InputSlot = 0;
  polygon_layout[0].AlignedByteOffset = 0;
  polygon_layout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  polygon_layout[0].InstanceDataStepRate = 0;

  polygon_layout[1].SemanticName = "COLOR";
  polygon_layout[1].SemanticIndex = 0;
  polygon_layout[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
  polygon_layout[1].InputSlot = 0;
  polygon_layout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
  polygon_layout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
  polygon_layout[1].InstanceDataStepRate = 0;

  // layout 의 요소 수를 가져옵니다
  const int32_t count = ARRAYSIZE(polygon_layout);

  // 정점 input layout 을 만듭니다
  com::ThrowIfFailed(device->CreateInputLayout(
      polygon_layout, count, vertex_shader_buffer->GetBufferPointer(),
      vertex_shader_buffer->GetBufferSize(), &layout_));

  // 더 이상 사용되지 않는 정점, 픽셀 셰이더 버퍼를 해제합니다
  vertex_shader_buffer->Release();
  vertex_shader_buffer = nullptr;

  pixel_shader_buffer->Release();
  pixel_shader_buffer = nullptr;

  // 정점 셰이더에 있는 행렬 상수 버퍼의 description 을 작성합니다
  D3D11_BUFFER_DESC matrix_buffer_desc{};
  matrix_buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
  matrix_buffer_desc.ByteWidth = sizeof(MatrixBufferType);
  matrix_buffer_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  matrix_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  matrix_buffer_desc.MiscFlags = 0;
  matrix_buffer_desc.StructureByteStride = 0;

  // 상수 버퍼 포인터를 만들어 이 클래스에서 정점 셰이더 상수 버퍼에 접근할 수 있게 합니다
  com::ThrowIfFailed(
      device->CreateBuffer(&matrix_buffer_desc, nullptr, &matrix_buffer_));

  return true;
}

void ColorShaderClass::ShutdownShader() {
  if (matrix_buffer_) {
    matrix_buffer_->Release();
    matrix_buffer_ = nullptr;
  }

  if (layout_) {
    layout_->Release();
    layout_ = nullptr;
  }

  if (pixel_shader_) {
    pixel_shader_->Release();
    pixel_shader_ = nullptr;
  }

  if (vertex_shader_) {
    vertex_shader_->Release();
    vertex_shader_ = nullptr;
  }
}

void ColorShaderClass::OutputShaderErrorMessage(
    ID3DBlob* error_message, const HWND hwnd,
    const std::filesystem::path& path) {
  // 출력창에 에러 메시지를 표시합니다
  OutputDebugString(
      reinterpret_cast<const wchar_t*>(error_message->GetBufferPointer()));

  error_message->Release();
  error_message = nullptr;

  MessageBox(hwnd, L"Error copiling shader.", path.c_str(), MB_OK);
}

void ColorShaderClass::SetShaderParameters(ID3D11DeviceContext* device_context,
                                           DirectX::XMMATRIX& world,
                                           DirectX::XMMATRIX& view,
                                           DirectX::XMMATRIX& projection) {
  // 행렬을 transpose 하여 셰이더에서 사용할 수 있게 합니다
  world = DirectX::XMMatrixTranspose(world);
  view = DirectX::XMMatrixTranspose(view);
  projection = DirectX::XMMatrixTranspose(projection);

  // 상수 버퍼의 내용을 쓸 수 있도록 잠급니다
  D3D11_MAPPED_SUBRESOURCE mapped_resource{};
  com::ThrowIfFailed(device_context->Map(
      matrix_buffer_, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource));

  // 상수 버퍼 데이터에 대한 포인터를 가져옵니다
  MatrixBufferType* data =
      reinterpret_cast<MatrixBufferType*>(mapped_resource.pData);

  // 상수 버퍼에 행렬을 복사합니다
  data->world_ = world;
  data->view_ = view;
  data->projection_ = projection;

  // 상수 버퍼의 잠금을 풉니다
  device_context->Unmap(matrix_buffer_, 0);

  uint32_t buffer_number = 0;
  device_context->VSSetConstantBuffers(buffer_number, 1, &matrix_buffer_);
}

void ColorShaderClass::RenderShader(ID3D11DeviceContext* device_context,
                                    const int32_t index_count) {
  // 정점 입력 레이아웃을 설정합니다
  device_context->IASetInputLayout(layout_);

  // 삼각형을 그릴 정점 셰이더와 픽셀 셰이더를 설정합니다
  device_context->VSSetShader(vertex_shader_, nullptr, 0);
  device_context->PSSetShader(pixel_shader_, nullptr, 0);

  // 삼각형을 그립니다
  device_context->DrawIndexed(index_count, 0, 0);
}
