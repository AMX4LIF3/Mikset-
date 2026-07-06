#include "cubemap.hpp"
#include "../Buffers/buffers.hpp"

ComPtr<ID3D11ShaderResourceView> LoadCubeMapResource(ID3D11Device& device, const std::wstring& path) {
	HRESULT hr;
	TexMetadata metadata;
	ScratchImage scratchimg;

	// load dds from disk
	hr = LoadFromDDSFile(path.c_str(), DDS_FLAGS_NONE, &metadata, scratchimg);
	if (FAILED(hr)) {
		MessageBoxW(NULL, (L"File not found: " + path).c_str(), L"Error", MB_OK);
		return nullptr;
	}

	// create the actual texture resource on the gpu
	ComPtr<ID3D11Resource> resource;
	hr = CreateTexture(&device, scratchimg.GetImages(), scratchimg.GetImageCount(), metadata, resource.GetAddressOf());
	if (FAILED(hr)) {
		MessageBoxA(NULL, "CreateTexture failed! Check format support.", "GPU Error", MB_OK);
		return nullptr;
	}

	// create the shader resource view (srv) specifically as a cube
	ComPtr<ID3D11ShaderResourceView> srv;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MostDetailedMip = 0;
	srvDesc.TextureCube.MipLevels = (UINT)metadata.mipLevels;

	hr = device.CreateShaderResourceView(resource.Get(), &srvDesc, srv.GetAddressOf());
	if (FAILED(hr)) {
		MessageBoxA(NULL, "SRV Creation failed! Force Cube failed.", "GPU Error", MB_OK);
		return nullptr;
	}

	return srv;
}


bool CubeMap::Init(Core& core, const std::wstring& path) {
	D3D11_INPUT_ELEMENT_DESC IEdesc[] = {
		{
			"POSITION",
			0,
			DXGI_FORMAT_R32G32B32_FLOAT,
			0,
			0,
			D3D11_INPUT_PER_VERTEX_DATA,
			0
		}
	};
	
	cubemap = LoadCubeMapResource(*core.GetDevice(), path);
	if (!cubemap) {
		MessageBoxA(NULL, "Failed to Load a CubeMap..", "LoadCubeMapResource() Failed..", MB_OK | MB_ICONERROR);
		return false;
	}

	SkyBoxVertex vertices[] = {
		{ XMFLOAT3(-1.0f,  1.0f, -1.0f) },
		{ XMFLOAT3(1.0f,  1.0f, -1.0f) },
		{ XMFLOAT3(1.0f, -1.0f, -1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f, -1.0f) },
		{ XMFLOAT3(-1.0f,  1.0f,  1.0f) },
		{ XMFLOAT3(1.0f,  1.0f,  1.0f) },
		{ XMFLOAT3(1.0f, -1.0f,  1.0f) },
		{ XMFLOAT3(-1.0f, -1.0f,  1.0f) }
	};


	uint32_t indices[] = {
		// front face
		0, 1, 2,
		2, 3, 0,
		// right face
		1, 5, 6,
		6, 2, 1,
		// back face
		7, 6, 5,
		5, 4, 7,
		// left face
		4, 0, 3,
		3, 7, 4,
		// top face
		4, 5, 1,
		1, 0, 4,
		// bottom face
		3, 2, 6,
		6, 7, 3
	};
	HRESULT hr;

	// set up de buffers
	D3D11_BUFFER_DESC Vbuffdesc{};
	Vbuffdesc.ByteWidth = sizeof(SkyBoxVertex) * 8;
	Vbuffdesc.Usage = D3D11_USAGE_IMMUTABLE; // cuz dat shi never changes
	Vbuffdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA Vsubres{};
	Vsubres.pSysMem = vertices;
	hr = core.GetDevice()->CreateBuffer(&Vbuffdesc, &Vsubres, VBuff.GetAddressOf());
	if (FAILED(hr)) {
		MessageBoxA(NULL, "Failed to create a vertex buffer for a cubemap...", "CreateBuffer() Failed!", MB_OK | MB_ICONERROR);
		return false;
	}

	D3D11_BUFFER_DESC Ibuffdesc{};
	Ibuffdesc.ByteWidth = sizeof(uint32_t) * 36;
	Ibuffdesc.Usage = D3D11_USAGE_IMMUTABLE; 
	Ibuffdesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA Isubres{};
	Isubres.pSysMem = indices;
	hr = core.GetDevice()->CreateBuffer(&Ibuffdesc, &Isubres, IBuff.GetAddressOf());
	if (FAILED(hr)) {
		MessageBoxA(NULL, "Failed to create a index buffer for a cubemap...", "CreateBuffer() Failed!", MB_OK | MB_ICONERROR);
		return false;
	}


	D3D11_BUFFER_DESC cbuffdesc{};
	cbuffdesc.ByteWidth = sizeof(XMMATRIX); // 64 bytes
	cbuffdesc.Usage = D3D11_USAGE_DYNAMIC; 
	cbuffdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbuffdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbuffdesc.MiscFlags = 0;
	cbuffdesc.StructureByteStride = 0;
	hr = core.GetDevice()->CreateBuffer(&cbuffdesc, nullptr, CBuff.GetAddressOf());
	if (FAILED(hr)) {
		MessageBoxA(NULL, "Failed to create a constant buffer for a cubemap...", "CreateBuffer() Failed!", MB_OK | MB_ICONERROR);
		return false;
	}

	hr = shader.VSCompileFromFile(core, core.GetDevice(), "C:/Users/User/Desktop/C++/3d shit/Mikset/Renderer/Graphics/shaders/skybox.hlsl", "SkyboxShader");
	if (FAILED(hr)) {
		MessageBoxA(NULL, "Failed to Compile a cubemap's vertex shader.\nmake sure the path and shader are valid..", "VSCompileFromFile() Failed!", MB_OK | MB_ICONERROR);
		return false;
	}

	hr = core.m_device->CreateInputLayout(
		IEdesc, _countof(IEdesc),
		shader.GetBlob()->GetBufferPointer(),
		shader.GetBlob()->GetBufferSize(),
		InpLayout.GetAddressOf()
	);
	if (FAILED(hr)) {
		MessageBoxA(NULL, "Failed to Compile a cubemap's pixel shader.\nmake sure the path and shader are valid..", "PSCompileFromFile() Failed!", MB_OK | MB_ICONERROR);
		return false;
	}
	
	hr = shader.PSCompileFromFile(core, core.GetDevice(), "C:/Users/User/Desktop/C++/3d shit/Mikset/Renderer/Graphics/shaders/skybox.hlsl", "SkyboxShader");
	if (FAILED(hr)) {
		MessageBoxA(NULL, "Failed to Compile a cubemap's pixel shader.\nmake sure the path and shader are valid..", "PSCompileFromFile() Failed!", MB_OK | MB_ICONERROR);
		return false;
	}
	core.m_deviceCtx->IASetInputLayout(InpLayout.Get());


	D3D11_RASTERIZER_DESC rsDesc{};
	rsDesc.FillMode = D3D11_FILL_SOLID; 
	rsDesc.CullMode = D3D11_CULL_NONE;
	core.GetDevice()->CreateRasterizerState(&rsDesc, &RasterizerState);
	
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL; // always at 1.0
	core.GetDevice()->CreateDepthStencilState(&dsDesc, &DepthState);


	D3D11_SAMPLER_DESC sampDesc = {};
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR; 
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP; 
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	core.GetDevice()->CreateSamplerState(&sampDesc, Sampler.GetAddressOf());

	core.m_deviceCtx->VSSetConstantBuffers(0, 1, CBuff.GetAddressOf());
	UINT strides = sizeof(SkyBoxVertex); 

	return true;
}


void CubeMap::Draw(Core& core, Camera& camera, Shader& prevShader, ID3D11InputLayout* prevIL) {
	auto* ctx = core.m_deviceCtx.Get();
	if (!IBuff || !CBuff || !ctx) return;

	XMMATRIX view = camera.GetViewMatrix();
	XMMATRIX proj = camera.GetProjMatrix();
	view.r[3] = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	viewProj = XMMatrixTranspose(viewProj); 

	D3D11_MAPPED_SUBRESOURCE mapped;
	if (SUCCEEDED(ctx->Map(CBuff.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
		memcpy(mapped.pData, &viewProj, sizeof(XMMATRIX));
		ctx->Unmap(CBuff.Get(), 0);
	}

	UINT stride = sizeof(SkyBoxVertex);
	UINT offset = 0;
	ctx->IASetVertexBuffers(0, 1, VBuff.GetAddressOf(), &stride, &offset);
	ctx->IASetIndexBuffer(IBuff.Get(), DXGI_FORMAT_R32_UINT, 0);
	ctx->IASetInputLayout(InpLayout.Get());
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ctx->VSSetShader(shader.GetVS().Get(), nullptr, 0);
	ctx->VSSetConstantBuffers(10, 1, CBuff.GetAddressOf());

	ctx->PSSetShader(shader.GetPS().Get(), nullptr, 0);
	ctx->PSSetSamplers(0, 1, Sampler.GetAddressOf());

	ID3D11ShaderResourceView* srv = cubemap.Get();
	ctx->PSSetShaderResources(0, 1, &srv);

	ctx->RSSetState(RasterizerState.Get());
	ctx->OMSetDepthStencilState(DepthState.Get(), 0);

	ctx->DrawIndexed(36, 0, 0);
	float blendFactor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	ctx->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
	ctx->OMSetDepthStencilState(nullptr, 0);  
	ctx->RSSetState(nullptr);                 

	ID3D11ShaderResourceView* nullSRV = nullptr;
	ctx->PSSetShaderResources(0, 1, &nullSRV); 

	ID3D11SamplerState* nullSampler = nullptr;
	ctx->PSSetSamplers(0, 1, &nullSampler);

	ctx->VSSetShader(prevShader.GetVS().Get(), nullptr, 0); 
	ctx->PSSetShader(prevShader.GetPS().Get(), nullptr, 0);
	ctx->IASetInputLayout(prevIL);
}
