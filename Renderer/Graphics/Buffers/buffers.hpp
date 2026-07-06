#pragma once
#include<iostream>
#include<DirectXMath.h>
using namespace DirectX;

// need it to be 16 otherwise FUCK
typedef struct alignas(16) GlobalBuffer   
{
	XMMATRIX viewProj;
	XMMATRIX lightViewProj;
	XMFLOAT4 CameraPos; 
	XMFLOAT4 LightDir;
};

typedef struct alignas(16) ObjectBuffer 
{
	XMMATRIX World;
	XMMATRIX WVP;
	unsigned int HasBones;
	float padding[3];    
};

typedef struct alignas(16) SkinningBuffer  
{
	XMMATRIX BoneMatrices[128]; // should do fine for now.. i hope so 
};

typedef struct SkyboxBuffer { //  64 bytes
	XMMATRIX ViewProj;
};

typedef struct SkyBoxVertex {
	XMFLOAT3 pos;
};

