#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <filesystem>

class ColorShaderClass {
 public:
  bool Initialize(ID3D11Device* device, const HWND hwnd);
  void Shutdown();
  void Render(ID3D11DeviceContext* device_context, const int32_t index_count,
              DirectX::XMMATRIX world, DirectX::XMMATRIX view,
              DirectX::XMMATRIX projection);

 private:
  struct MatrixBufferType {
    DirectX::XMMATRIX world_;
    DirectX::XMMATRIX view_;
    DirectX::XMMATRIX projection_;
  };

  bool InitializeShader(ID3D11Device* device, const HWND hwnd,
                        const std::filesystem::path& vs_path,
                        const std::filesystem::path& ps_path);
  void ShutdownShader();
  void OutputShaderErrorMessage(ID3DBlob* error_message, const HWND hwnd,
                                const std::filesystem::path& path);

  void SetShaderParameters(ID3D11DeviceContext* device_context,
                           DirectX::XMMATRIX& world, DirectX::XMMATRIX& view,
                           DirectX::XMMATRIX& projection);
  void RenderShader(ID3D11DeviceContext* device_context,
                    const int32_t index_count);

  ID3D11VertexShader* vertex_shader_ = nullptr;
  ID3D11PixelShader* pixel_shader_ = nullptr;
  ID3D11InputLayout* layout_ = nullptr;
  ID3D11Buffer* matrix_buffer_ = nullptr;
};
