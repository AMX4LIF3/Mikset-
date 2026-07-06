/* AMX's bullshit inc. 
*  idk what to put here 
*  kys 
*/

#include "shaders.hpp"
#include <windows.h> // btw it will all work even if you remove this include because d3dcompiler.h includes it anyway.. the more you know
#include <sstream>
#include <string>
#include <iostream>
#include <cstring>
#include "../../Core/common.hpp"

Shader::Shader() {}
Shader::~Shader() {}

static const char* GetShaderType(ShaderType type) {
	switch (type) {
		case ShaderType::VertexShader: return "vs_5_0";
		case ShaderType::PixelShader:  return "ps_5_0";
		default: return "";
	}
}

// safely build a std::string from an ID3DBlob (does not assume NULL)
// btw all of this debug shit i didnt write ofc since idc about that
static std::string BlobToString(ID3DBlob* blob) {
	if (!blob || blob->GetBufferSize() == 0) return std::string();
	const char* data = static_cast<const char*>(blob->GetBufferPointer());
	return std::string(data, data + blob->GetBufferSize());
}

static void ReportAndThrowCompileError(HRESULT hr, ID3DBlob* err_blob,
	const char* sourceName, const char* entryPoint, const char* target,
	const void* srcData, size_t srcSize)
{
	std::ostringstream oss;
	oss << "D3DCompile failed. HRESULT: 0x" << std::hex << static_cast<unsigned long>(hr) << "\n";
	oss << "SourceName: " << (sourceName ? sourceName : "(null)") << "\n";
	oss << "EntryPoint: " << (entryPoint ? entryPoint : "(null)") << "\n";
	oss << "Target: "     << (target    ? target    : "(null)") << "\n";
	oss << "SourceSize: " << std::dec << srcSize << " bytes\n\n";

	std::string errmsg = BlobToString(err_blob);
	if (!errmsg.empty()) {
		oss << "Compiler output:\n" << errmsg;
	} else {
		oss << "No compiler message provided. Showing first 1024 bytes of source (if available):\n";
		if (srcData && srcSize > 0) {
			const char* src = static_cast<const char*>(srcData);
			size_t dumpSize = srcSize < 1024 ? srcSize : 1024;
			oss.write(src, dumpSize);
			if (dumpSize < srcSize) oss << "\n... (truncated)\n";
		} else {
			oss << "(no source data)\n";
		}
	}

	auto out = oss.str();
	OutputDebugStringA(out.c_str());
	std::cerr << out;

	MessageBoxA(NULL, out.c_str(), "Shader compile error", MB_OK | MB_ICONERROR);

	throw std::runtime_error(out);
}

void Shader::Compile(ID3D11Device* device, ShaderBinary binary) {
	if (!binary.SourceCode)	      throw std::invalid_argument("Shader::Compile: null SourceCode");
	if (!binary.SourceName)	      throw std::invalid_argument("Shader::Compile: null SourceName");
	if (!binary.SourceEntryPoint) throw std::invalid_argument("Shader::Compile: null SourceEntryPoint");
	if (!binary.SourceCodeSize) {
		
		// try to infer size from a C string
		const char* src = static_cast<const char*>(binary.SourceCode);
		if (src) {
			binary.SourceCodeSize = std::strlen(src);
		}
		if (!binary.SourceCodeSize) {
			throw std::invalid_argument("Shader::Compile: SourceCodeSize is zero and could not be inferred");
		}
	}

	unsigned int compileFlags = 0;
#ifdef _DEBUG
	compileFlags |= D3DCOMPILE_DEBUG;
#endif

	ComPtr<ID3DBlob> err_blob{};
	HRESULT hr = D3DCompile(
		binary.SourceCode,
		binary.SourceCodeSize,
		binary.SourceName,
		nullptr,        // pDefines
		nullptr,        // pInclude
		binary.SourceEntryPoint,
		GetShaderType(binary.Type),
		compileFlags,
		0,
		&m_blob,
		&err_blob
	);

	if (FAILED(hr)) {
		ReportAndThrowCompileError(
			hr, err_blob.Get(),
			binary.SourceName, binary.SourceEntryPoint,
			GetShaderType(binary.Type), binary.SourceCode, binary.SourceCodeSize
		);
	}

	// create shader object from m_blob
	HRESULT createHr = S_OK;
	if (binary.Type == ShaderType::VertexShader) {
		createHr = device->CreateVertexShader(
			m_blob->GetBufferPointer(),
			m_blob->GetBufferSize(),
			nullptr,
			&m_vs
		);
	} else if (binary.Type == ShaderType::PixelShader) {
		createHr = device->CreatePixelShader(
			m_blob->GetBufferPointer(),
			m_blob->GetBufferSize(),
			nullptr,
			&m_ps
		);
	} else {
		throw std::invalid_argument("Shader::Compile: unknown shader type");
	}

	if (FAILED(createHr)) {
		std::ostringstream oss;
		oss << "Failed to create shader object. HRESULT: 0x" << std::hex << static_cast<unsigned long>(createHr) << "\n";
		OutputDebugStringA(oss.str().c_str());
		throw std::runtime_error(oss.str());
	}
}

bool Shader::VSCompileFromFile(Core& core, ID3D11Device* device, std::string path, std::string name) {
	const std::string code = ReadFileToString(path);
	ShaderBinary binary = { name.c_str(), code.data(), code.size(), "VMain", ShaderType::VertexShader }; 
	try {
		this->Compile(core.m_device.Get(), binary);
	}
	catch (const std::exception& e) {
		MessageBoxA(NULL, e.what(), "VS Compilation error", MB_OK | MB_ICONERROR);
		return false;
	}

	return true;
	
}

bool Shader::PSCompileFromFile(Core& core, ID3D11Device* device, std::string path, std::string name) {
	const std::string code = ReadFileToString(path);
	ShaderBinary binary = { name.c_str(), code.data(), code.size(), "PMain", ShaderType::PixelShader }; 

	try {
		this->Compile(core.m_device.Get(), binary);
	}
	catch (const std::exception& e) {
		MessageBoxA(NULL, e.what(), "PS Compilation error", MB_OK | MB_ICONERROR);
		return false;
	}

	return true;
}

ComPtr<ID3DBlob> Shader::GetBlob()		   { return m_blob; } // lookz nizz
ComPtr<ID3D11VertexShader> Shader::GetVS() { return m_vs;   }
ComPtr<ID3D11PixelShader>  Shader::GetPS() { return m_ps;   }
ComPtr<ID3D11InputLayout>  Shader::GetIL() { return m_IL;   }
/*
void  Shader::SetIL(ComPtr<ID3D11InputLayout> input) {
	this->m_IL = input;
}
*/

