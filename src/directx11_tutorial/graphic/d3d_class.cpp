#include "pch.h"
#include "d3d_class.h"

bool D3DClass::Initialize(const int32_t width, const int32_t height, bool vsync,
                          HWND hwnd, bool fullscreen, float screen_depth,
                          float screen_near) {
  HRESULT result;
  IDXGIFactory* factory = nullptr;
  IDXGIAdapter* adapter = nullptr;
  IDXGIOutput* adapterOutput = nullptr;
  unsigned int numModes, i, numerator, denominator;
  size_t stringLength;
  DXGI_MODE_DESC* displayModeList = nullptr;
  DXGI_ADAPTER_DESC adapterDesc;
  int error;
  DXGI_SWAP_CHAIN_DESC swapChainDesc;
  D3D_FEATURE_LEVEL featureLevel;
  ID3D11Texture2D* backBufferPtr = nullptr;
  D3D11_TEXTURE2D_DESC depthBufferDesc;
  D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
  D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
  D3D11_RASTERIZER_DESC rasterDesc;
  D3D11_VIEWPORT viewport;
  float fieldOfView, screenAspect;

  // vsync(수직동기화) 설정을 저장합니다.
  vsync_enabled_ = vsync;

  // DirectX 그래픽 인터페이스 팩토리를 만듭니다.
  result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
  if (FAILED(result)) return false;

  // 팩토리 객체를 사용하여 첫번째 그래픽 카드 인터페이스에 대한 아답터를
  // 만듭니다.
  result = factory->EnumAdapters(0, &adapter);
  if (FAILED(result)) return false;

  // 출력(모니터)에 대한 첫번째 아답터를 나열합니다.
  result = adapter->EnumOutputs(0, &adapterOutput);
  if (FAILED(result)) return false;

  // DXGI_FORMAT_R8G8B8A8_UNORM 모니터 출력 디스플레이 포맷에 맞는 모드의 개수를
  // 구합니다.
  result = adapterOutput->GetDisplayModeList(
      DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
  if (FAILED(result)) return false;

  // 가능한 모든 모니터와 그래픽카드 조합을 저장할 리스트를 생성합니다.
  displayModeList = new DXGI_MODE_DESC[numModes];
  if (!displayModeList) return false;

  // 디스플레이 모드에 대한 리스트 구조를 채워넣습니다.
  result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM,
                                             DXGI_ENUM_MODES_INTERLACED,
                                             &numModes, displayModeList);
  if (FAILED(result)) return false;

  // 이제 모든 디스플레이 모드에 대해 화면 너비/높이에 맞는 디스플레이 모드를
  // 찾습니다. 적합한 것을 찾으면 모니터의 새로고침 비율의 분모와 분자 값을
  // 저장합니다.
  for (i = 0; i < numModes; i++) {
    if (displayModeList[i].Width == (uint32_t)width) {
      if (displayModeList[i].Height == (uint32_t)height) {
        numerator = displayModeList[i].RefreshRate.Numerator;
        denominator = displayModeList[i].RefreshRate.Denominator;
      }
    }
  }

  // 어댑터(그래픽카드)의 description을 가져옵니다.
  result = adapter->GetDesc(&adapterDesc);
  if (FAILED(result)) return false;

  // 현재 그래픽카드의 메모리 용량을 메가바이트 단위로 저장합니다.
  video_card_memory_ = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

  // 그래픽카드의 이름을 char형 문자열 배열로 바꾼 뒤 저장합니다.
  error = wcstombs_s(&stringLength, video_card_description_, 128,
                     adapterDesc.Description, 128);
  if (error != 0) return false;

  // 디스플레이 모드 리스트의 할당을 해제합니다.
  delete[] displayModeList;
  displayModeList = 0;

  // 출력 아답터를 할당 해제합니다.
  adapterOutput->Release();
  adapterOutput = 0;

  // 아답터를 할당 해제합니다.
  adapter->Release();
  adapter = 0;

  // 팩토리 객체를 할당 해제합니다.
  factory->Release();
  factory = 0;

  // 스왑 체인 description을 초기화합니다.
  ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

  // 하나의 백버퍼만을 사용하도록 합니다.
  swapChainDesc.BufferCount = 1;

  // 백버퍼의 너비와 높이를 설정합니다.
  swapChainDesc.BufferDesc.Width = width;
  swapChainDesc.BufferDesc.Height = height;

  // 백버퍼로 일반적인 32bit의 서페이스를 지정합니다.
  swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

  // 백버퍼의 새로고침 비율을 설정합니다.
  if (vsync_enabled_) {
    swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
  } else {
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
  }

  // 백버퍼의 용도를 설정합니다.
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

  // 렌더링이 이루어질 윈도우의 핸들을 설정합니다.
  swapChainDesc.OutputWindow = hwnd;

  // 멀티샘플링을 끕니다.
  swapChainDesc.SampleDesc.Count = 1;
  swapChainDesc.SampleDesc.Quality = 0;

  // 윈도우 모드 또는 풀스크린 모드를 설정합니다.
  if (fullscreen)
    swapChainDesc.Windowed = false;
  else
    swapChainDesc.Windowed = true;

  // 스캔라인의 정렬과 스캔라이닝을 지정되지 않음으로(unspecified) 설정합니다.
  swapChainDesc.BufferDesc.ScanlineOrdering =
      DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

  // 출력된 이후의 백버퍼의 내용을 버리도록 합니다.
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  // 추가 옵션 플래그를 사용하지 않습니다.
  swapChainDesc.Flags = 0;

  // 피쳐 레벨을 DirectX 11로 설정합니다.
  featureLevel = D3D_FEATURE_LEVEL_11_0;

  // 스왑 체인, Direct3D 디바이스, Direct3D 디바이스 컨텍스트를 생성합니다.
  result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
                                         0, &featureLevel, 1, D3D11_SDK_VERSION,
                                         &swapChainDesc, &swap_chain_, &device_,
                                         NULL, &device_context_);
  if (FAILED(result)) return false;

  // 백버퍼의 포인터를 가져옵니다.
  result = swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                  (LPVOID*)&backBufferPtr);
  if (FAILED(result)) return false;

  // 백버퍼의 포인터로 렌더타겟 뷰를 생성합니다. 
  result = device_->CreateRenderTargetView(backBufferPtr, NULL,
                                           &render_target_view_);
  if (FAILED(result)) return false;

  // 백버퍼 포인터를 더이상 사용하지 않으므로 할당 해제합니다.
  backBufferPtr->Release();
  backBufferPtr = 0;

  // 깊이 버퍼의 description을 초기화합니다.
  ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

  // 깊이 버퍼의 description을 작성합니다.
  depthBufferDesc.Width = width;
  depthBufferDesc.Height = height;
  depthBufferDesc.MipLevels = 1;
  depthBufferDesc.ArraySize = 1;
  depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthBufferDesc.SampleDesc.Count = 1;
  depthBufferDesc.SampleDesc.Quality = 0;
  depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
  depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  depthBufferDesc.CPUAccessFlags = 0;
  depthBufferDesc.MiscFlags = 0;

  // description을 사용하여 깊이 버퍼의 텍스쳐를 생성합니다.
  result =
      device_->CreateTexture2D(&depthBufferDesc, NULL, &depth_stencil_buffer_);
  if (FAILED(result)) return false;

  // 스텐실 상태의 description을 초기화합니다.
  ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

  // 스텐실 상태의 description을 작성합니다.
  depthStencilDesc.DepthEnable = true;
  depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

  depthStencilDesc.StencilEnable = true;
  depthStencilDesc.StencilReadMask = 0xFF;
  depthStencilDesc.StencilWriteMask = 0xFF;

  // front 픽셀의 스텐실을 설정합니다.
  depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
  depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

  // back 픽셀의 스텐실을 설정합니다.
  depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
  depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

  // 깊이-스텐실 상태를 생성합니다.
  result = device_->CreateDepthStencilState(&depthStencilDesc,
                                            &depth_stencil_state_);
  if (FAILED(result)) return false;

  // 깊이-스텐실 상태를 설정합니다.
  device_context_->OMSetDepthStencilState(depth_stencil_state_, 1);

  // 깊이-스텐실 뷰의 description을 초기화합니다.
  ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

  // 깊이-스텐실 뷰의 description을 작성합니다.
  depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  depthStencilViewDesc.Texture2D.MipSlice = 0;

  // 깊이-스텐실 뷰를 생성합니다.
  result = device_->CreateDepthStencilView(
      depth_stencil_buffer_, &depthStencilViewDesc, &depth_stencil_view_);
  if (FAILED(result)) return false;

  // 렌더타겟 뷰와 깊이-스텐실 버퍼를 각각 출력 파이프라인에 바인딩합니다.
  device_context_->OMSetRenderTargets(1, &render_target_view_,
                                      depth_stencil_view_);

  // 어떤 도형을 어떻게 그릴 것인지 결정하는 래스터화기 description을
  // 작성합니다.
  rasterDesc.AntialiasedLineEnable = false;
  rasterDesc.CullMode = D3D11_CULL_BACK;
  rasterDesc.DepthBias = 0;
  rasterDesc.DepthBiasClamp = 0.0f;
  rasterDesc.DepthClipEnable = true;
  rasterDesc.FillMode = D3D11_FILL_SOLID;
  rasterDesc.FrontCounterClockwise = false;
  rasterDesc.MultisampleEnable = false;
  rasterDesc.ScissorEnable = false;
  rasterDesc.SlopeScaledDepthBias = 0.0f;

  // 작성한 description으로부터 래스터화기 상태를 생성합니다.
  result = device_->CreateRasterizerState(&rasterDesc, &raster_state_);
  if (FAILED(result)) return false;

  // 래스터화기 상태를 설정합니다.
  device_context_->RSSetState(raster_state_);

  // 렌더링을 위한 뷰포트를 설정합니다.
  viewport.Width = (float)width;
  viewport.Height = (float)height;
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;
  viewport.TopLeftX = 0.0f;
  viewport.TopLeftY = 0.0f;

  // 뷰포트를 생성합니다.
  device_context_->RSSetViewports(1, &viewport);

  // 투영 행렬을 설정합니다.
  fieldOfView = (float)DirectX::XM_PI / 4.0f;
  screenAspect = (float)width / (float)height;

  // 3D 렌더링을 위한 투영 행렬을 생성합니다.
  projection_matrix_ = DirectX::XMMatrixPerspectiveFovLH(
      fieldOfView, screenAspect, screen_near, screen_depth);

  // 월드 행렬을 단위 행렬로 초기화합니다.
  world_matrix_ = DirectX::XMMatrixIdentity();

  // 2D 렌더링에 사용될 직교 투영 행렬을 생성합니다.
  ortho_matrix_ = DirectX::XMMatrixOrthographicLH((float)width, (float)height,
                                                  screen_near, screen_depth);

  return true;
}

void D3DClass::Shutdown() {
  // 종료하기 전에 이렇게 윈도우 모드로 바꾸지 않으면 스왑체인을 할당 해제할 때 예외가 발생합니다.
  if (swap_chain_) swap_chain_->SetFullscreenState(false, NULL);

  if (raster_state_) {
    raster_state_->Release();
    raster_state_ = nullptr;
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
  float color[4];

  // 버퍼를 어떤 색상으로 지울 것인지 설정합니다.
  color[0] = red;
  color[1] = green;
  color[2] = blue;
  color[3] = alpha;

  // 백버퍼의 내용을 지웁니다.
  device_context_->ClearRenderTargetView(render_target_view_, color);

  // 깊이 버퍼를 지웁니다.
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

void D3DClass::GetVideoCardInfo(char* card_name, int& memory) {
  strcpy_s(card_name, 128, video_card_description_);
  memory = video_card_memory_;
}
