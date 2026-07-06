#include "entity.hpp"
Entity::Entity(const std::string& name, std::shared_ptr<Model> model, bool IsCollidable) 
	: Collidable(IsCollidable), model(model), Name(name)
{

	material.BaseColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f); // Pure white tint
	material.Metallic = 0.0f;           // Default to non-metal
	material.Roughness = 0.5f;          // Default to a matte surface
	material.AmbientOcclusion = 1.0f;   // Default to fully lit
	material.Emissive = 0.0f;

	// for some reason models with bones get rotated weirdly hence this
	if (model.get()->HasBones) transf.rotation.x = 1.5f; 


	// the collision stuff will prob be here

}
Entity::~Entity() {}


void Entity::Update(float dt) {
	if (this->Collidable) {
		// do zum stuff

	}
	else {
		// also do zum stuff

	}

}

void Entity::SetName(const std::string& namestr) {
	this->Name = namestr;
}
// eventually this would get dirty but thats IF i load a shit ton of stuff like 10k entities..for now it works.
XMMATRIX Entity::GetWorldMat() const {
	XMVECTOR vPos = XMLoadFloat3(&this->transf.position);
	XMVECTOR vRot = XMLoadFloat3(&this->transf.rotation);
	XMVECTOR vScale = XMLoadFloat3(&this->transf.scale);

	// now we convert em to matrixes
	XMMATRIX mTranslate = XMMatrixTranslationFromVector(vPos); // "Pos" basically
	XMMATRIX mScale		= XMMatrixScalingFromVector(vScale);
	XMMATRIX mRot		= XMMatrixRotationRollPitchYawFromVector(vRot);

	// SRT
	return mScale * mRot * mTranslate;
}

Model* Entity::GetModel() {
	return this->model.get();
}





