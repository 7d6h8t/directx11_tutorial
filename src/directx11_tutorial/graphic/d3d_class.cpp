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

  // vsync(��������ȭ) ������ �����մϴ�.
  vsync_enabled_ = vsync;

  // DirectX �׷��� �������̽� ���丮�� ����ϴ�.
  result = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory);
  if (FAILED(result)) return false;

  // ���丮 ��ü�� ����Ͽ� ù��° �׷��� ī�� �������̽��� ���� �ƴ��͸�
  // ����ϴ�.
  result = factory->EnumAdapters(0, &adapter);
  if (FAILED(result)) return false;

  // ���(�����)�� ���� ù��° �ƴ��͸� �����մϴ�.
  result = adapter->EnumOutputs(0, &adapterOutput);
  if (FAILED(result)) return false;

  // DXGI_FORMAT_R8G8B8A8_UNORM ����� ��� ���÷��� ���˿� �´� ����� ������
  // ���մϴ�.
  result = adapterOutput->GetDisplayModeList(
      DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_ENUM_MODES_INTERLACED, &numModes, NULL);
  if (FAILED(result)) return false;

  // ������ ��� ����Ϳ� �׷���ī�� ������ ������ ����Ʈ�� �����մϴ�.
  displayModeList = new DXGI_MODE_DESC[numModes];
  if (!displayModeList) return false;

  // ���÷��� ��忡 ���� ����Ʈ ������ ä���ֽ��ϴ�.
  result = adapterOutput->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM,
                                             DXGI_ENUM_MODES_INTERLACED,
                                             &numModes, displayModeList);
  if (FAILED(result)) return false;

  // ���� ��� ���÷��� ��忡 ���� ȭ�� �ʺ�/���̿� �´� ���÷��� ��带
  // ã���ϴ�. ������ ���� ã���� ������� ���ΰ�ħ ������ �и�� ���� ����
  // �����մϴ�.
  for (i = 0; i < numModes; i++) {
    if (displayModeList[i].Width == (uint32_t)width) {
      if (displayModeList[i].Height == (uint32_t)height) {
        numerator = displayModeList[i].RefreshRate.Numerator;
        denominator = displayModeList[i].RefreshRate.Denominator;
      }
    }
  }

  // �����(�׷���ī��)�� description�� �����ɴϴ�.
  result = adapter->GetDesc(&adapterDesc);
  if (FAILED(result)) return false;

  // ���� �׷���ī���� �޸� �뷮�� �ް�����Ʈ ������ �����մϴ�.
  video_card_memory_ = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

  // �׷���ī���� �̸��� char�� ���ڿ� �迭�� �ٲ� �� �����մϴ�.
  error = wcstombs_s(&stringLength, video_card_description_, 128,
                     adapterDesc.Description, 128);
  if (error != 0) return false;

  // ���÷��� ��� ����Ʈ�� �Ҵ��� �����մϴ�.
  delete[] displayModeList;
  displayModeList = 0;

  // ��� �ƴ��͸� �Ҵ� �����մϴ�.
  adapterOutput->Release();
  adapterOutput = 0;

  // �ƴ��͸� �Ҵ� �����մϴ�.
  adapter->Release();
  adapter = 0;

  // ���丮 ��ü�� �Ҵ� �����մϴ�.
  factory->Release();
  factory = 0;

  // ���� ü�� description�� �ʱ�ȭ�մϴ�.
  ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));

  // �ϳ��� ����۸��� ����ϵ��� �մϴ�.
  swapChainDesc.BufferCount = 1;

  // ������� �ʺ�� ���̸� �����մϴ�.
  swapChainDesc.BufferDesc.Width = width;
  swapChainDesc.BufferDesc.Height = height;

  // ����۷� �Ϲ����� 32bit�� �����̽��� �����մϴ�.
  swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

  // ������� ���ΰ�ħ ������ �����մϴ�.
  if (vsync_enabled_) {
    swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
  } else {
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
  }

  // ������� �뵵�� �����մϴ�.
  swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

  // �������� �̷���� �������� �ڵ��� �����մϴ�.
  swapChainDesc.OutputWindow = hwnd;

  // ��Ƽ���ø��� ���ϴ�.
  swapChainDesc.SampleDesc.Count = 1;
  swapChainDesc.SampleDesc.Quality = 0;

  // ������ ��� �Ǵ� Ǯ��ũ�� ��带 �����մϴ�.
  if (fullscreen)
    swapChainDesc.Windowed = false;
  else
    swapChainDesc.Windowed = true;

  // ��ĵ������ ���İ� ��ĵ���̴��� �������� ��������(unspecified) �����մϴ�.
  swapChainDesc.BufferDesc.ScanlineOrdering =
      DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
  swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

  // ��µ� ������ ������� ������ �������� �մϴ�.
  swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  // �߰� �ɼ� �÷��׸� ������� �ʽ��ϴ�.
  swapChainDesc.Flags = 0;

  // ���� ������ DirectX 11�� �����մϴ�.
  featureLevel = D3D_FEATURE_LEVEL_11_0;

  // ���� ü��, Direct3D ����̽�, Direct3D ����̽� ���ؽ�Ʈ�� �����մϴ�.
  result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
                                         0, &featureLevel, 1, D3D11_SDK_VERSION,
                                         &swapChainDesc, &swap_chain_, &device_,
                                         NULL, &device_context_);
  if (FAILED(result)) return false;

  // ������� �����͸� �����ɴϴ�.
  result = swap_chain_->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                  (LPVOID*)&backBufferPtr);
  if (FAILED(result)) return false;

  // ������� �����ͷ� ����Ÿ�� �並 �����մϴ�. 
  result = device_->CreateRenderTargetView(backBufferPtr, NULL,
                                           &render_target_view_);
  if (FAILED(result)) return false;

  // ����� �����͸� ���̻� ������� �����Ƿ� �Ҵ� �����մϴ�.
  backBufferPtr->Release();
  backBufferPtr = 0;

  // ���� ������ description�� �ʱ�ȭ�մϴ�.
  ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

  // ���� ������ description�� �ۼ��մϴ�.
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

  // description�� ����Ͽ� ���� ������ �ؽ��ĸ� �����մϴ�.
  result =
      device_->CreateTexture2D(&depthBufferDesc, NULL, &depth_stencil_buffer_);
  if (FAILED(result)) return false;

  // ���ٽ� ������ description�� �ʱ�ȭ�մϴ�.
  ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

  // ���ٽ� ������ description�� �ۼ��մϴ�.
  depthStencilDesc.DepthEnable = true;
  depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

  depthStencilDesc.StencilEnable = true;
  depthStencilDesc.StencilReadMask = 0xFF;
  depthStencilDesc.StencilWriteMask = 0xFF;

  // front �ȼ��� ���ٽ��� �����մϴ�.
  depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
  depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

  // back �ȼ��� ���ٽ��� �����մϴ�.
  depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
  depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
  depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

  // ����-���ٽ� ���¸� �����մϴ�.
  result = device_->CreateDepthStencilState(&depthStencilDesc,
                                            &depth_stencil_state_);
  if (FAILED(result)) return false;

  // ����-���ٽ� ���¸� �����մϴ�.
  device_context_->OMSetDepthStencilState(depth_stencil_state_, 1);

  // ����-���ٽ� ���� description�� �ʱ�ȭ�մϴ�.
  ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

  // ����-���ٽ� ���� description�� �ۼ��մϴ�.
  depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
  depthStencilViewDesc.Texture2D.MipSlice = 0;

  // ����-���ٽ� �並 �����մϴ�.
  result = device_->CreateDepthStencilView(
      depth_stencil_buffer_, &depthStencilViewDesc, &depth_stencil_view_);
  if (FAILED(result)) return false;

  // ����Ÿ�� ��� ����-���ٽ� ���۸� ���� ��� ���������ο� ���ε��մϴ�.
  device_context_->OMSetRenderTargets(1, &render_target_view_,
                                      depth_stencil_view_);

  // � ������ ��� �׸� ������ �����ϴ� ������ȭ�� description��
  // �ۼ��մϴ�.
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

  // �ۼ��� description���κ��� ������ȭ�� ���¸� �����մϴ�.
  result = device_->CreateRasterizerState(&rasterDesc, &raster_state_);
  if (FAILED(result)) return false;

  // ������ȭ�� ���¸� �����մϴ�.
  device_context_->RSSetState(raster_state_);

  // �������� ���� ����Ʈ�� �����մϴ�.
  viewport.Width = (float)width;
  viewport.Height = (float)height;
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;
  viewport.TopLeftX = 0.0f;
  viewport.TopLeftY = 0.0f;

  // ����Ʈ�� �����մϴ�.
  device_context_->RSSetViewports(1, &viewport);

  // ���� ����� �����մϴ�.
  fieldOfView = (float)DirectX::XM_PI / 4.0f;
  screenAspect = (float)width / (float)height;

  // 3D �������� ���� ���� ����� �����մϴ�.
  projection_matrix_ = DirectX::XMMatrixPerspectiveFovLH(
      fieldOfView, screenAspect, screen_near, screen_depth);

  // ���� ����� ���� ��ķ� �ʱ�ȭ�մϴ�.
  world_matrix_ = DirectX::XMMatrixIdentity();

  // 2D �������� ���� ���� ���� ����� �����մϴ�.
  ortho_matrix_ = DirectX::XMMatrixOrthographicLH((float)width, (float)height,
                                                  screen_near, screen_depth);

  return true;
}

void D3DClass::Shutdown() {
  // �����ϱ� ���� �̷��� ������ ���� �ٲ��� ������ ����ü���� �Ҵ� ������ �� ���ܰ� �߻��մϴ�.
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

  // ���۸� � �������� ���� ������ �����մϴ�.
  color[0] = red;
  color[1] = green;
  color[2] = blue;
  color[3] = alpha;

  // ������� ������ ����ϴ�.
  device_context_->ClearRenderTargetView(render_target_view_, color);

  // ���� ���۸� ����ϴ�.
  device_context_->ClearDepthStencilView(depth_stencil_view_, D3D11_CLEAR_DEPTH,
                                         1.0f, 0);
}

void D3DClass::EndScene() {
  // �������� �Ϸ�Ǿ����Ƿ� ������� ������ ȭ�鿡 ǥ���մϴ�.
  if (vsync_enabled_)
    // ���ΰ�ħ ������ �����մϴ�.
    swap_chain_->Present(1, 0);
  else
    // ������ �� ������ ǥ���մϴ�.
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
