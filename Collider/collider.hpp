#pragma once
#include <DirectXMath.h>
#include <string>
using namespace DirectX;

enum class CollisionType{Box, TriangleMesh};

typedef struct BVHNode {
	XMFLOAT3 Min, Max;

	int left, right;
	int index; // triangle index and count
	int count;

	bool isLeaf = false;
};

typedef struct Collider { 
	std::string Id; // to get the exact collider that was hit since entites have more than 1 collider (usually) 
	CollisionType Type; 
	XMFLOAT3 Min, Max;

	void BuildBVHFromVertices(std::vector<float> vertices) {
		
	}
};

typedef struct CollisionInfo {
	XMVECTOR Normal = XMVectorZero();
	float Depth = 0.0f;
	bool Hit = false;
	XMVECTOR point = XMVectorZero();
};

typedef struct CollisionResult {
	float groundDepth = 0.0f;
	float wallDepth = 0.0f;
	XMVECTOR groundNormal = XMVectorZero();
	XMVECTOR wallNormal = XMVectorZero();
	XMVECTOR groundWorldNormal = XMVectorZero(); 
	XMVECTOR wallWorldNormal = XMVectorZero();   
	bool hitGround = false;
	bool hitWall = false;
};
