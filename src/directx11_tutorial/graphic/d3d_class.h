#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

class D3DClass {
 public:
  bool Initialize(const int32_t width, const int32_t height, bool vsync,
                  HWND hwnd, bool fullscreen, float screen_depth,
                  float screen_near);
  void Shutdown();

  void BeginScene(float red, float green, float blue, float alpha);
  void EndScene();

  ID3D11Device* GetDevice();
  ID3D11DeviceContext* GetDeviceContext();

  void GetProjectionMatrix(DirectX::XMMATRIX& projection_matrix);
  void GetWorldMatrix(DirectX::XMMATRIX& world_matrix);
  void GetOrthoMatrix(DirectX::XMMATRIX& ortho_matrix);

  void GetVideoCardInfo(char* card_name, int& memory);

 private:
  bool vsync_enabled_ = false;
  int32_t video_card_memory_ = 0;
  char video_card_description_[128];
  IDXGISwapChain* swap_chain_ = nullptr;
  ID3D11Device* device_ = nullptr;
  ID3D11DeviceContext* device_context_ = nullptr;
  ID3D11RenderTargetView* render_target_view_ = nullptr;
  ID3D11Texture2D* depth_stencil_buffer_ = nullptr;
  ID3D11DepthStencilState* depth_stencil_state_ = nullptr;
  ID3D11DepthStencilView* depth_stencil_view_ = nullptr;
  ID3D11RasterizerState* raster_state_ = nullptr;
  DirectX::XMMATRIX projection_matrix_;
  DirectX::XMMATRIX world_matrix_;
  DirectX::XMMATRIX ortho_matrix_;
};