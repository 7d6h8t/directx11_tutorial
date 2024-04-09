#include "pch.h"
#include "d3d_class.h"

#include "com_throw.h"

bool D3DClass::Initialize(const int32_t width, const int32_t height, bool vsync,
                          HWND hwnd, bool fullscreen, float screen_depth,
                          float screen_near) {
  // vsync(수직동기화) 상태를 저장합니다
  vsync_enabled_ = vsync;

  // DirectX 그래픽 인터페이스 팩토리를 생성합니다
  IDXGIFactory* factory = nullptr;
  com::ThrowIfFailed(
      CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory));

  // 팩토리 객체를 사용하여 첫번째 그래픽 카드 인터페이스 어댑터를 생성합니다
  IDXGIAdapter* adapter = nullptr;
  com::ThrowIfFailed(factory->EnumAdapters(0, &adapter));

  // 출력(모니터)에 대한 첫번째 어댑터를 지정합니다
  IDXGIOutput* adapter_output = nullptr;
  com::ThrowIfFailed(adapter->EnumOutputs(0, &adapter_output));

  // 출력 (모니터)에 대한 DXGI_FORMAT_R8G8B8A8_UNORM 형식에 맞는 모드 수를
  // 가져옵니다
  uint32_t num_modes = 0;
  com::ThrowIfFailed(adapter_output->GetDisplayModeList(
      DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &num_modes,
      nullptr));

  // 가능한 모든 모니터와 그래픽카드 조합을 저장할 리스트를 생성합니다
  DXGI_MODE_DESC* display_mode_list = nullptr;
  display_mode_list = new DXGI_MODE_DESC[num_modes];
  if (!display_mode_list) return false;

  // 디스플레이 모드에 대한 리스트를 채워넣습니다
  com::ThrowIfFailed(adapter_output->GetDisplayModeList(
      DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &num_modes,
      display_mode_list));

  // 이제 모든 디스플레이 모드에 대해 화면 너비/높이에 맞는 디스플레이 모드를
  // 찾습니다. 적합한 것을 찾으면 모니터의 새로고침 비율의 분모와 분자 값을
  // 저장합니다.
  uint32_t numerator = 0, denominator = 0;
  for (uint32_t i = 0; i < num_modes; i++) {
    if (display_mode_list[i].Width == static_cast<UINT>(width)) {
      if (display_mode_list[i].Height == static_cast<UINT>(height)) {
        numerator = display_mode_list[i].RefreshRate.Numerator;
        denominator = display_mode_list[i].RefreshRate.Denominator;
      }
    }
  }

  // 어댑터(그래픽카드)의 description을 가져옵니다
  DXGI_ADAPTER_DESC adapter_desc{};
  com::ThrowIfFailed(adapter->GetDesc(&adapter_desc));

  // 현재 그래픽카드의 메모리 용량을 메가바이트 단위로 저장합니다.
  video_card_memory_ =
      static_cast<int32_t>(adapter_desc.DedicatedVideoMemory / 1024 / 1024);

  // 그래픽카드의 이름을 저장합니다.
  video_card_name_.assign(adapter_desc.Description);

  // 디스플레이 모드 리스트의 할당을 해제합니다. 
  delete[] display_mode_list;
  display_mode_list = nullptr;

  // 출력 어댑터를 할당 해제합니다.
  adapter_output->Release();
  adapter_output = nullptr;

  // 어댑터를 할당 해제합니다.
  adapter->Release();
  adapter = nullptr;

  // 팩토리 객체를 할당 해제합니다.
  factory->Release();
  factory = nullptr;

  // 스왑 체인 description을 초기화합니다.
  DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
  swap_chain_desc.BufferCount = 1; // backbuffer 를 1개만 사용하도록 지정합니다
  swap_chain_desc.BufferDesc.Width = width; // backbuffer 의 넓이를 지정합니다
  swap_chain_desc.BufferDesc.Height = height; // backbuffer 의 높이를 지정합니다
  swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 32bit surface를 지정합니다.
  swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // backbuffer 의 사용 용도를 설정합니다
  swap_chain_desc.OutputWindow = hwnd; // 렌더링이 이루어질 윈도우의 핸들을 설정합니다
  swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD; // 출력된 이후의 backbuffer 의 내용을 버리도록 합니다
  swap_chain_desc.Flags = 0; // 추가 옵션 플래그를 사용하지 않습니다
  swap_chain_desc.Windowed = fullscreen ? false : true; // 창모드(true) or 풀스크린(false) 모드를 설정합니다
  swap_chain_desc.BufferDesc.ScanlineOrdering =
      DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED; // 스캔 라인의 정렬을 지정되지 않음으로(unspecified) 설정합니다
  swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED; // 스캔라이닝을 지정되지 않음으로(unspecified) 설정합니다

  // 멀티샘플링을 끕니다
  swap_chain_desc.SampleDesc.Count = 1;
  swap_chain_desc.SampleDesc.Quality = 0;

  // backbuffer 의 새로고침 비율을 설정합니다
  if (vsync_enabled_) {
    swap_chain_desc.BufferDesc.RefreshRate.Numerator = numerator;
    swap_chain_desc.BufferDesc.RefreshRate.Denominator = denominator;
  } else {
    swap_chain_desc.BufferDesc.RefreshRate.Numerator = 0;
    swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
  }

  // 피쳐 레벨을 DirectX 11로 설정합니다
  D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;

  // 스왑 체인, Direct3D 디바이스, Direct3D 디바이스 컨텍스트를 생성합니다
  com::ThrowIfFailed(D3D11CreateDeviceAndSwapChain(
      NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &feature_level, 1,
      D3D11_SDK_VERSION, &swap_chain_desc, &swap_chain_, &device_, NULL,
      &device_context_));

  // backbuffer 의 포인터를 가져옵니다
  ID3D11Texture2D* back_buffer = nullptr;
  com::ThrowIfFailed(swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                            (LPVOID*)&back_buffer));

  // backbuffer 의 포인터로 렌더 타겟 뷰를 생성합니다.
  com::ThrowIfFailed(device_->CreateRenderTargetView(back_buffer, nullptr,
                                                     &render_target_view_));

  // backbuffer 포인터를 더이상 사용하지 않으므로 할당 해제합니다
  back_buffer->Release();
  back_buffer = nullptr;

  // 깊이 버퍼의 description을 작성합니다
  D3D11_TEXTURE2D_DESC depth_buffer_desc{};
  depth_buffer_desc.Width = width;
  depth_buffer_desc.Height = height;
  depth_buffer_desc.MipLevels = 1;
  depth_buffer_desc.ArraySize = 1;
  depth_buffer_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depth_buffer_desc.SampleDesc.Count = 1;
  depth_buffer_desc.SampleDesc.Quality = 0;
  depth_buffer_desc.Usage = D3D11_USAGE_DEFAULT;
  depth_buffer_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  depth_buffer_desc.CPUAccessFlags = 0;
  depth_buffer_desc.MiscFlags = 0;

  // description을 사용하여 깊이 버퍼의 텍스쳐를 생성합니다.
  com::ThrowIfFailed(device_->CreateTexture2D(&depth_buffer_desc, nullptr,
                                              &depth_stencil_buffer_));

  // 스텐실 상태의 description을 작성합니다.
  D3D11_DEPTH_STENCIL_DESC depth_stencil_desc{};
  depth_stencil_desc.DepthEnable = true;
  depth_stencil_desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  depth_stencil_desc.DepthFunc = D3D11_COMPARISON_LESS;
  depth_stencil_desc.StencilEnable = true;
  depth_stencil_desc.StencilReadMask = 0xFF;
  depth_stencil_desc.StencilWriteMask = 0xFF;

  // front 픽셀의 스텐실을 설정합니다.
  depth_stencil_desc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  depth_stencil_desc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
  depth_stencil_desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depth_stencil_desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

  // back 픽셀의 스텐실을 설정합니다.
  depth_stencil_desc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  depth_stencil_desc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
  depth_stencil_desc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depth_stencil_desc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

  // 깊이-스텐실 상태를 생성합니다
  com::ThrowIfFailed(device_->CreateDepthStencilState(&depth_stencil_desc,
                                                      &depth_stencil_state_));

  // 깊이-스텐실 상태를 설정합니다
  device_context_->OMSetDepthStencilState(depth_stencil_state_, 1);

  // 깊이-스텐실 뷰의 description을 작성합니다
  D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
  depth_stencil_view_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  depth_stencil_view_desc.Texture2D.MipSlice = 0;

  // 깊이-스텐실 뷰를 생성합니다
  com::ThrowIfFailed(device_->CreateDepthStencilView(
      depth_stencil_buffer_, &depth_stencil_view_desc, &depth_stencil_view_));

  // 렌더 타겟 뷰와 깊이-스텐실 버퍼를 출력 파이프 라인에 바인딩합니다
  device_context_->OMSetRenderTargets(1, &render_target_view_,
                                      depth_stencil_view_);

  // 어떤 도형을 어떻게 그릴 것인지 결정하는 rasterizer description을
  // 작성합니다
  D3D11_RASTERIZER_DESC rasterizer_desc{};
  rasterizer_desc.AntialiasedLineEnable = false;
  rasterizer_desc.CullMode = D3D11_CULL_BACK;
  rasterizer_desc.DepthBias = 0;
  rasterizer_desc.DepthBiasClamp = 0.0f;
  rasterizer_desc.DepthClipEnable = true;
  rasterizer_desc.FillMode = D3D11_FILL_SOLID;
  rasterizer_desc.FrontCounterClockwise = false;
  rasterizer_desc.MultisampleEnable = false;
  rasterizer_desc.ScissorEnable = false;
  rasterizer_desc.SlopeScaledDepthBias = 0.0f;

  // 작성한 description 으로 부터 rasterizer 상태를 생성합니다
  com::ThrowIfFailed(
      device_->CreateRasterizerState(&rasterizer_desc, &rasterizer_state_));

  // rasterizer 상태를 설정합니다.
  device_context_->RSSetState(rasterizer_state_);

  // 렌더링을 위한 뷰포트를 설정합니다
  D3D11_VIEWPORT viewport{};
  viewport.Width = static_cast<float>(width);
  viewport.Height = static_cast<float>(height);
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;
  viewport.TopLeftX = 0.0f;
  viewport.TopLeftY = 0.0f;

  // 뷰포트를 생성합니다
  device_context_->RSSetViewports(1, &viewport);

  // 투영 행렬을 설정합니다
  float field_of_view = DirectX::XM_PI / 4.0f;
  float screen_aspect = static_cast<float>(width) / static_cast<float>(height);

  // 3D 렌더링을 위한 투영 행렬을 생성합니다
  projection_matrix_ = DirectX::XMMatrixPerspectiveFovLH(
      field_of_view, screen_aspect, screen_near, screen_depth);

  // 월드 행렬을 단위 행렬로 초기화합니다
  world_matrix_ = DirectX::XMMatrixIdentity();

  // 2D 렌더링을 위한 직교 투영 행렬을 생성합니다.
  ortho_matrix_ = DirectX::XMMatrixOrthographicLH(static_cast<float>(width),
                                                  static_cast<float>(height),
                                                  screen_near, screen_depth);

  return true;
}

void D3DClass::Shutdown() {
  // 종료하기 전에 이렇게 윈도우 모드로 바꾸지 않으면 스왑체인을 할당 해제할 때 예외가 발생합니다.
  if (swap_chain_) swap_chain_->SetFullscreenState(false, nullptr);

  if (rasterizer_state_) {
    rasterizer_state_->Release();
    rasterizer_state_ = nullptr;
  }

  if (depth_stencil_view_) {
    depth_stencil_view_->Release();
    depth_stencil_view_ = nullptr;
  }

  if (depth_stencil_state_) {
    depth_stencil_state_->Release();
    depth_stencil_state_ = nullptr;
  }

  if (depth_stencil_buffer_) {
    depth_stencil_buffer_->Release();
    depth_stencil_buffer_ = nullptr;
  }

  if (render_target_view_) {
    render_target_view_->Release();
    render_target_view_ = nullptr;
  }

  if (device_context_) {
    device_context_->Release();
    device_context_ = nullptr;
  }

  if (device_) {
    device_->Release();
    device_ = nullptr;
  }

  if (swap_chain_) {
    swap_chain_->Release();
    swap_chain_ = nullptr;
  }
}

void D3DClass::BeginScene(float red, float green, float blue, float alpha) {
  // 버퍼를 지울 색상을 설정합니다
  float color[4] = {red, green, blue, alpha};

  // backbuffer 의 내용을 지웁니다
  device_context_->ClearRenderTargetView(render_target_view_, color);

  // 깊이 버퍼를 지웁니다
  device_context_->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH,
                                         1.0f, 0);
}

void D3DClass::EndScene() {
  // 렌더링이 완료되었으므로 백버퍼의 내용을 화면에 표시합니다.
  if (vsync_enabled_)
    // 새로고침 비율을 고정합니다.
    swap_chain_->Present(1, 0);
  else
    // 가능한 한 빠르게 표시합니다.
    swap_chain_->Present(0, 0);
}

ID3D11Device* D3DClass::GetDevice() { return device_; }

ID3D11DeviceContext* D3DClass::GetDeviceContext() { return device_context_; }

void D3DClass::GetProjectionMatrix(DirectX::XMMATRIX& projection_matrix) {
  projection_matrix = projection_matrix_;
}

void D3DClass::GetWorldMatrix(DirectX::XMMATRIX& world_matrix) {
  world_matrix = world_matrix_;
}

void D3DClass::GetOrthoMatrix(DirectX::XMMATRIX& ortho_matrix) {
  ortho_matrix = ortho_matrix_;
}

void D3DClass::GetVideoCardInfo(std::wstring& card_name, int32_t& memory) {
  card_name = video_card_name_;
  memory = video_card_memory_;
}
