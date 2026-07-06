#include "shadowmap.hpp"

void ShadowMap::Init(Core& core) {

	D3D11_TEXTURE2D_DESC texdesc{};
	texdesc.Format = DXGI_FORMAT_R32_TYPELESS;
	texdesc.ArraySize = 1;
	texdesc.Height = 1024;
	texdesc.Width = 1024;
	texdesc.MipLevels = 1;
	texdesc.SampleDesc.Count = 1;
	texdesc.SampleDesc.Quality = 0;
	texdesc.Usage = D3D11_USAGE_DEFAULT;
	texdesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	texdesc.CPUAccessFlags = 0;
	texdesc.MiscFlags = 0;

	HRESULT hr = core.m_device->CreateTexture2D(&texdesc, nullptr, this->tex.GetAddressOf());
	if (FAILED(hr)) {
		MessageBoxA(NULL, "ShadowMap: CreateTexture2D failed", "ShadowMap::Init", MB_OK | MB_ICONERROR);
		return;
	}

	this->viewport.Width = (float)texdesc.Width;
	this->viewport.Height = (float)texdesc.Height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;


	D3D11_DEPTH_STENCIL_VIEW_DESC dsv_desc{};
	dsv_desc.Format = DXGI_FORMAT_D32_FLOAT; // dsv wants the D
	dsv_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsv_desc.Flags = 0;
	dsv_desc.Texture2D.MipSlice = 0;

	hr = core.m_device->CreateDepthStencilView(this->tex.Get(), &dsv_desc, this->dsv.GetAddressOf());
	if (FAILED(hr)) {
		MessageBoxA(NULL, "ShadowMap: CreateDepthStencilView failed", "ShadowMap::Init", MB_OK | MB_ICONERROR);
		return;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc{};
	srv_desc.Format = DXGI_FORMAT_R32_FLOAT; // shader-readable
	srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srv_desc.Texture2D.MostDetailedMip = 0;
	srv_desc.Texture2D.MipLevels = 1;

	hr = core.m_device->CreateShaderResourceView(this->tex.Get(), &srv_desc, this->srv.GetAddressOf());
	if (FAILED(hr)) {
		MessageBoxA(NULL, "ShadowMap: CreateShaderResourceView failed", "ShadowMap::Init", MB_OK | MB_ICONERROR);
		return;
	}

	D3D11_SAMPLER_DESC samplerDesc{};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0.0f;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;

	hr = core.m_device->CreateSamplerState(&samplerDesc, this->sampler.GetAddressOf());
	if (FAILED(hr)) {
		MessageBoxA(NULL, "ShadowMap: CreateSamplerState failed", "ShadowMap::Init", MB_OK | MB_ICONERROR);
		return;
	}
}

void ShadowMap::BindAsTarget(Core& core) {
	ID3D11ShaderResourceView* nullSRV = nullptr;
	core.m_deviceCtx->PSSetShaderResources(6, 1, &nullSRV);
	core.m_deviceCtx->OMSetRenderTargets(0, nullptr, this->dsv.Get()); // only want depth no colors 
	core.m_deviceCtx->ClearDepthStencilView(this->dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	core.m_deviceCtx->RSSetViewports(1, &this->viewport);
}

void ShadowMap::BindAsResource(Core& core) {
	core.m_deviceCtx->PSSetShaderResources(6, 1, this->srv.GetAddressOf());
	core.m_deviceCtx->PSSetSamplers(1, 1, this->sampler.GetAddressOf());
	core.m_deviceCtx->PSSetSamplers(2, 1, this->comparisionSampler.GetAddressOf());
}

