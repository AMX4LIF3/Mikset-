
#pragma once
#include "../shaders.hpp"

using Microsoft::WRL::ComPtr;
class ShadowMap {

public:
	ComPtr<ID3D11ShaderResourceView> srv;
	ComPtr<ID3D11DepthStencilView> dsv;
	ComPtr<ID3D11SamplerState> sampler;
	ComPtr<ID3D11SamplerState> comparisionSampler;
	D3D11_VIEWPORT viewport;
	ComPtr<ID3D11Texture2D> tex;


	void Init(Core& core);
	void BindAsTarget(Core& core);
	void BindAsResource(Core& core);

};

