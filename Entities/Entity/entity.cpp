#include "entity.hpp"

Entity::Entity() : Collidable(false), model(nullptr), Name("un-named") {}

Entity::Entity(const std::string& name, std::shared_ptr<Model> model, bool IsCollidable) 
	: model(model), Name(name)
{
	this->model.get()->ClearData = !this->Collidable;

	material.BaseColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); 
	material.Metallic = 0.0f;           
	material.Roughness = 0.5f;          
	material.AmbientOcclusion = 1.0f;   
	material.Emissive = 0.0f;

	// for some reason models with bones get rotated weirdly hence this
	// also HasBones has weird behaviour so HasAnimations() is fine for now
	if (model.get()->HasAnimations()) transf.rotation.x = 1.57f; 
}
Entity::~Entity() {}


void Entity::Update(float dt) {
	if (this->Collidable) {
		// do zum stuff

	}
	else {
		// also do zum stuff

	}
	
	if (script) script(this, dt);
}

void Entity::SetName(const std::string& namestr) {
	this->Name = namestr;
}

// eventually this would get dirty but thats IF i load a shit ton of stuff like 10k entities..for now it works.
XMMATRIX Entity::GetWorldMat() const {
	XMVECTOR vPos   = XMLoadFloat3(&this->transf.position);
	XMVECTOR vRot   = XMLoadFloat3(&this->transf.rotation);
	XMVECTOR vScale = XMLoadFloat3(&this->transf.scale);

	// now we convert em to matrixes
	XMMATRIX mTranslate = XMMatrixTranslationFromVector(vPos); 
	XMMATRIX mScale		= XMMatrixScalingFromVector(vScale);
	XMMATRIX mRot		= XMMatrixRotationRollPitchYawFromVector(vRot);

	// SRT
	return mScale * mRot * mTranslate;
}

Model* Entity::GetModel() {
	return this->model.get();
}





