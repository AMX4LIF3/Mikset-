#pragma once
#include "../../Resources/ModelHandle.hpp"
#include "../../Collider/collider.hpp"
#include <functional> 

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
		Entity();

		virtual ~Entity();


		void Update(float dt); 
		std::function<void(Entity*, float)> script; // this allows for a lambda style script, the float is there for delta time or whatever shit you want
													// looks cool af aswell									 
												   /// TODO: id say a context struct would be much better to pass for the script similar to the UI 
	
		void AddColliderComponent(CollisionType type, XMFLOAT3 min = { 0.0f, 0.0f, 0.0f }, XMFLOAT3 max = { 1.0f, 1.0f, 1.0f }) {
			if (type == CollisionType::TriangleMesh) {
				Collider col; 
				col.Type = CollisionType::TriangleMesh;
				this->colliders.push_back(col);
			}
			else if (type == CollisionType::Box) {
				Collider col;
				col.Min = min;
				col.Max = max;
				this->colliders.push_back(col);
			}
		}

		XMMATRIX GetWorldMat() const;
		

		Model* GetModel(); 
		void SetName(const std::string& namestr);

		Transform transf;
		XMMATRIX worldMat = XMMatrixIdentity();
		std::string Name = "UnNamed"; // ID 
		MaterialBuffer material;
		std::vector<Collider> colliders;
		bool MulByRootNodeTransf = false;
	private:
		std::shared_ptr<Model> model = nullptr; 
		bool Collidable = false; 
};


