#pragma once
#include<DirectXMath.h>
#include "../../Resources/ModelHandle.hpp"

class Core;
class Scene;
class Shader;
class Model;

using namespace DirectX; 

typedef struct Transform { 
	XMFLOAT3 position = { 0,0,0 };
	XMFLOAT3 rotation = { 0,0,0 };
	XMFLOAT3 scale	  =	{ 1,1,1 };
};

typedef struct alignas(16) MaterialBuffer 
{
	DirectX::XMFLOAT4 BaseColor;    
	float Metallic;                 
	float Roughness;                
	float AmbientOcclusion;         
	float Emissive;
};


class Entity {
public:

	Entity(const std::string& name, std::shared_ptr<Model> model, bool IsCollidable);

	~Entity();


	void Update(float dt);
	XMMATRIX GetWorldMat() const;
	Model* GetModel(); 
	void SetName(const std::string& namestr);

	Transform transf;
	std::string Name; // ID 
	MaterialBuffer material;
	// TODO: Collision Box 

private:
	std::shared_ptr<Model> model; 
	bool Collidable = false; 
};


