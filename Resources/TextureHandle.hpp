#pragma once
#include<iostream>
#include<d3d11.h>
#include<wrl/client.h>

using Microsoft::WRL::ComPtr;

typedef struct Texture
{
	ComPtr<ID3D11ShaderResourceView> srv;

	std::string path; 
	unsigned int id; 
	std::string type;

	uint_fast32_t width, height;
};


