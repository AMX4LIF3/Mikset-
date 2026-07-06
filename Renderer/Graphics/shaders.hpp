#pragma once 
#include<wrl/client.h> 
#include "../../Core/core.hpp"
#include<windows.h>
#include<d3dcompiler.h>
#include<d3d11.h>
#include<iostream>
#include<string>
#include<sstream>

using Microsoft::WRL::ComPtr; 

enum class ShaderType {
	VertexShader = 0, 
	PixelShader // i really wanna call this Fragment shader for some reason
};

struct ShaderBinary {
	const char* SourceName{};
	const void* SourceCode{}; 
	size_t SourceCodeSize{};
	const char* SourceEntryPoint{}; 
	ShaderType Type{};
};


class Shader {
	public:
		ShaderBinary Binary; 

		Shader(); 
		~Shader(); 

		void Compile(ID3D11Device* device, ShaderBinary binary);
		bool VSCompileFromFile(Core& core, ID3D11Device* device, std::string path, std::string name);
		bool PSCompileFromFile(Core& core, ID3D11Device* device, std::string path, std::string name);
		ComPtr<ID3DBlob> GetBlob();
		ComPtr<ID3D11VertexShader> GetVS();
		ComPtr<ID3D11PixelShader>  GetPS();
		ComPtr<ID3D11InputLayout>  GetIL();
	//	void  SetIL(ComPtr<ID3D11InputLayout> input);

		ComPtr<ID3D11InputLayout> m_IL;





	private:
		ComPtr<ID3DBlob> m_blob{};
		ComPtr<ID3D11VertexShader> m_vs;
		ComPtr<ID3D11PixelShader>  m_ps;

		
};

