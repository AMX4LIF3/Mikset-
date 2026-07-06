#include "core.hpp"

Core::Core() {

}

Core::~Core() {
	if (window) {
		SDL_DestroyWindow(window);
	}
	SDL_Quit();
}


void Core::LET_THERE_BE_DX11() {
	SDL_Init(SDL_INIT_VIDEO);
	InitWindow();

	m_targetwin = nullptr;
	SDL_VERSION(&m_wminfo.version);

	if (SDL_GetWindowWMInfo(window, &m_wminfo)) {
		m_targetwin = m_wminfo.info.win.window;
	}

	UINT msaaCount = 4;
	UINT msaaQuality = 0;

	//DXGI_SWAP_CHAIN_DESC scd = {}; 
	m_scd.BufferCount = 1;
	m_scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 32-bit
	m_scd.BufferDesc.Width = (float)WINDOW_WIDTH;
	m_scd.BufferDesc.Height = (float)WINDOW_HEIGHT;
	m_scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_scd.OutputWindow = m_targetwin;
	m_scd.SampleDesc.Count = 4;
	m_scd.SampleDesc.Quality = msaaQuality - 1;
	m_scd.Windowed = TRUE;
	m_scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	m_result = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createDeviceFlags,
		nullptr, 0,
		D3D11_SDK_VERSION,
		&m_scd,
		&m_swapChain,
		&m_device,
		nullptr,
		&m_deviceCtx
	);


	//ID3D11RenderTargetView* rtv;
	//ID3D11Texture2D* backbuffer;

	m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&m_backbuffer);
	m_device->CreateRenderTargetView(m_backbuffer.Get(), nullptr, &m_rtv);

	// depth stencil shit
	D3D11_TEXTURE2D_DESC depthDesc{};
	depthDesc.Width = WINDOW_WIDTH;
	depthDesc.Height = WINDOW_HEIGHT;
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 4;
	depthDesc.SampleDesc.Quality = msaaQuality - 1;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;


	CD3D11_SAMPLER_DESC sampDesc{};
	sampDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampDesc.MaxAnisotropy = 16;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

	m_device->CreateSamplerState(&sampDesc, m_sampler.GetAddressOf());

	//ComPtr<ID3D11Texture2D> depthBuffer;
	m_device->CreateTexture2D(&depthDesc, nullptr, m_depthStencil.GetAddressOf());

	D3D11_TEXTURE2D_DESC msaaDesc{};
	msaaDesc.Width = WINDOW_WIDTH;
	msaaDesc.Height = WINDOW_HEIGHT;
	msaaDesc.MipLevels = 1;
	msaaDesc.ArraySize = 1;
	msaaDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	msaaDesc.SampleDesc.Count = 4;
	msaaDesc.SampleDesc.Quality = msaaQuality - 1;
	msaaDesc.Usage = D3D11_USAGE_DEFAULT;
	msaaDesc.BindFlags = D3D11_BIND_RENDER_TARGET;
	//ID3D11Device* device; 
	//ID3D11DeviceContext* deviceCtx; 
	//IDXGISwapChain* swapChain;
	ComPtr<ID3D11Texture2D> msaaBuffer;
	m_device->CreateTexture2D(&msaaDesc, nullptr, &msaaBuffer);
	ComPtr<ID3D11RenderTargetView> msaaRTV;
	m_device->CreateRenderTargetView(msaaBuffer.Get(), nullptr, &msaaRTV);


	// CREATE DAT SHI
	m_device->CreateDepthStencilView(m_depthStencil.Get(), nullptr, m_dsv.GetAddressOf());
	m_deviceCtx->OMSetRenderTargets(1, msaaRTV.GetAddressOf(), m_dsv.Get());

	// init rasterizer
	D3D11_RASTERIZER_DESC rsDesc{};
	rsDesc.FillMode = D3D11_FILL_SOLID;
	rsDesc.CullMode = D3D11_CULL_BACK;
	rsDesc.MultisampleEnable = true;
	rsDesc.FrontCounterClockwise = false;

	ComPtr<ID3D11RasterizerState> pRasterState;
	m_device->CreateRasterizerState(&rsDesc, &pRasterState);

	// bind it 
	m_deviceCtx->RSSetState(pRasterState.Get());
	m_deviceCtx->PSSetSamplers(0, 1, m_sampler.GetAddressOf()); // bind the sampler

	D3D11_BLEND_DESC blendDesc = {};
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	ID3D11BlendState* pBlendState;
	m_device->CreateBlendState(&blendDesc, &pBlendState);

	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_deviceCtx->OMSetBlendState(pBlendState, blendFactor, 0xFFFFFFFF);

	//m_backbuffer->Release(); nah fuck dat shi we got a smart ptr

	float middleX = abs((WINDOW_WIDTH - 1280) / 2);
	float farleft = abs(WINDOW_WIDTH - 1280);
	m_viewPort = {
		middleX, 0.0f,
		WINDOW_WIDTH, WINDOW_HEIGHT,
		0.0f, 1.0f,
	};

	CreateDefaultTextures();
	m_deviceCtx->RSSetViewports(1, &m_viewPort);

}

void Core::CreateDefaultTextures() {
	auto CreateTex = [&](uint32_t color, ID3D11ShaderResourceView** srv) { // ngl this is goated lmao
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = 1; // iz a 1x1
		desc.Height = 1;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // ooohh 32-bit cuz r8 + b8 + g8 + a8 = 32.. alr alr

		D3D11_SUBRESOURCE_DATA data{};
		data.pSysMem = &color;
		data.SysMemPitch = 4;

		ComPtr<ID3D11Texture2D> tex;
		m_device->CreateTexture2D(&desc, &data, &tex);
		m_device->CreateShaderResourceView(tex.Get(), nullptr, srv);
		};
	CreateTex(0xFFFFFFFF, &defaultAlbedo);   // white
	CreateTex(0xFFFF7F7F, &defaultNormal);   // flat normal 
	CreateTex(0xFF808080, &defaultRoughness);// mid-roughness
	CreateTex(0xFF000000, &defaultMetallic); // non-metallic as default
	CreateTex(0xFFFFFFFF, &defaultAO);	     // no AO
}

void Core::InitWindow() {
	this->window = SDL_CreateWindow(
		"Mikset", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN
	);
}

void Core::ClearDepth() {
	this->m_deviceCtx->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

