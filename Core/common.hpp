#pragma once
#include <string>
#include <fstream>
#include <windows.h>
#include <DirectXMath.h>
using namespace DirectX;


std::string ReadFileToString(const std::string& location);

typedef struct vertex{
	float xyz[3]; 
	float rgb[3]; 
	float Normal[3];
	float uv[2];
	float tangent[3];
	XMFLOAT4 Weights;	 // 4 weights per vertex
	uint32_t BoneIds[4]; // 4 bone ID's per vertex
};

XMFLOAT3 XMFLOAT3MUL(XMFLOAT3 A, XMFLOAT3 B);





