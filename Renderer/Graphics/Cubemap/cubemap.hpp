#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include <wrl/client.h>
#include "Renderer/Graphics/shaders.hpp"
#include "../../Entities/Entity/entity.hpp" 
#include "../../Entities/Player/player.hpp" 

using namespace DirectX;
using Microsoft::WRL::ComPtr;

class CubeMap {
	public:
		static std::unique_ptr<CubeMap> Create(Core& core, const std::wstring& path) {
			auto sky = std::make_unique<CubeMap>();
			if (!sky->Init(core, path)) {
				MessageBoxA(NULL, "Create() Failed..\nMake sure the skybox's format and path are valid.", "Failed to Create a CubeMap", MB_OK | MB_ICONERROR);
				return nullptr;
			}
			return sky;
		}
		void Draw(Core& core, Camera& camera, Shader& prevShader, ID3D11InputLayout* prevIL);
		CubeMap() = default;

		ComPtr<ID3D11ShaderResourceView> GetSRV() { return cubemap; }

	private:
		bool Init(Core& core, const std::wstring& path);
		ComPtr<ID3D11Buffer> VBuff;
		ComPtr<ID3D11Buffer> IBuff;
		ComPtr<ID3D11Buffer> CBuff;
		ComPtr<ID3D11InputLayout> InpLayout;

		ComPtr<ID3D11RasterizerState> RasterizerState;
		ComPtr<ID3D11DepthStencilState> DepthState;
		ComPtr<ID3D11SamplerState> Sampler;
		

		Shader shader;
		ComPtr<ID3D11ShaderResourceView> cubemap;
	};

