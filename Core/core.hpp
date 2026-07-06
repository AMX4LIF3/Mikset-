#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <d3d11.h>
#include <wrl/client.h> // for them fancy smart pointers 
#include <iostream>
#include <memory>
constexpr auto WINDOW_WIDTH = 1280;
constexpr auto WINDOW_HEIGHT = 720;

using Microsoft::WRL::ComPtr; // YEAAAA BOIIII

class Core {

	public:
		SDL_Window* window = nullptr;

		HWND m_targetwin = nullptr;
		SDL_SysWMinfo m_wminfo;
		DXGI_SWAP_CHAIN_DESC m_scd;
		ComPtr<ID3D11Device> m_device;
		ComPtr<ID3D11DeviceContext> m_deviceCtx;
		ComPtr<IDXGISwapChain> m_swapChain;
		HRESULT m_result;
		ComPtr<ID3D11RenderTargetView> m_rtv;
		ComPtr<ID3D11Texture2D> m_backbuffer;
		D3D11_VIEWPORT m_viewPort;
		ComPtr<ID3D11DepthStencilView> m_dsv;
		ComPtr<ID3D11Texture2D> m_depthStencil;
		ComPtr<ID3D11SamplerState> m_sampler;
		bool vsync = true; 

		ComPtr<ID3D11ShaderResourceView> defaultAlbedo;
		ComPtr<ID3D11ShaderResourceView> defaultNormal;
		ComPtr<ID3D11ShaderResourceView> defaultRoughness;
		ComPtr<ID3D11ShaderResourceView> defaultMetallic;
		ComPtr<ID3D11ShaderResourceView> defaultAO;

		void CreateDefaultTextures();


		Core(); 
		~Core();
		
		void LET_THERE_BE_DX11();
		ID3D11Device* GetDevice() const { return m_device.Get(); }
		void ClearDepth(); 

	private:

		void InitWindow(); 
};

