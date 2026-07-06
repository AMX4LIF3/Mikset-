#include "player.hpp"

Player::Player(float height) : 
Entity(), 
Height(height)
{
	this->Update(0.0f);
}

//static float Yvel = 0.0f;
void Player::Update(float dt) {
	this->Collider.base = this->transf.position;
	this->Collider.tip = XMFLOAT3(
		this->transf.position.x,
		this->transf.position.y + this->Height,
		this->transf.position.z
	);

	if (this->childCam) {
		this->childCam->pos.x = this->transf.position.x + 0.1f; 
		this->childCam->pos.y = this->transf.position.y + (this->Height * 0.9f);
		this->childCam->pos.z = this->transf.position.z;
	}	
	
	//if (!grounded) {
		//velocity.y -= 1.5f * dt;
		//this->transf.position.y += velocity.y;
	//}
	


}
/*
void Player::HandleInput(float dt, int movespeed, const SDL_Event& event, const Uint8* KeyScan, XMVECTOR camForward, XMVECTOR camRight) {
	XMVECTOR forward = XMVector3Normalize(XMVectorSet(XMVectorGetX(camForward), 0.0f, XMVectorGetZ(camForward), 0.0f));  
	XMVECTOR right = XMVector3Normalize(XMVectorSet(XMVectorGetX(camRight), 0.0f, XMVectorGetZ(camRight), 0.0f));  

	XMVECTOR movDir = XMVectorSet(0, 0, 0, 0);

	//movespeed = 20.0f;
	if (KeyScan[SDL_SCANCODE_W]) {
		movDir += forward;
	}

	if (KeyScan[SDL_SCANCODE_S]) {
		movDir -= forward;
	}

	if (KeyScan[SDL_SCANCODE_D]) {
		movDir += right;
	}

	if (KeyScan[SDL_SCANCODE_A]) {
		movDir -= right;
	}

	if (KeyScan[SDL_SCANCODE_SPACE] && grounded) {
		//this->transf.position.y += movespeed * dt;
		velocity.y = 0.5f;
		grounded = false;
	}
	if (KeyScan[SDL_SCANCODE_LCTRL]) {
		this->transf.position.y -= movespeed * dt;
	}

	// apply movement 
//	if (XMVectorGetX(XMVector3LengthSq(movDir)) > 0.0f) {
	//	movDir = XMVector3Normalize(movDir);
		//XMVECTOR pos = XMLoadFloat3(&this->transf.position);
		//XMStoreFloat3(&this->transf.position, pos + (movDir * movespeed * dt));
	//}
}
*/

void Player::HandleInput(float dt, int movespeed, const SDL_Event& event, const Uint8* KeyScan, XMVECTOR camForward, XMVECTOR camRight) {
	XMVECTOR forward = XMVector3Normalize(XMVectorSet(XMVectorGetX(camForward), 0.0f, XMVectorGetZ(camForward), 0.0f));
	XMVECTOR right = XMVector3Normalize(XMVectorSet(XMVectorGetX(camRight), 0.0f, XMVectorGetZ(camRight), 0.0f));

	XMVECTOR movDir = XMVectorSet(0, 0, 0, 0);

	if (KeyScan[SDL_SCANCODE_W]) movDir += forward;
	if (KeyScan[SDL_SCANCODE_S]) movDir -= forward;
	if (KeyScan[SDL_SCANCODE_D]) movDir += right;
	if (KeyScan[SDL_SCANCODE_A]) movDir -= right;

	if (KeyScan[SDL_SCANCODE_LCTRL]) this->transf.position.y -= movespeed * dt;
	if (KeyScan[SDL_SCANCODE_M])	 this->transf.position.y += movespeed * dt;


	// ALWAYS set velocity including zero if no keys
	XMVECTOR newVelocity = XMVectorSet(0, velocity.y, 0, 0); 

	if (XMVectorGetX(XMVector3LengthSq(movDir)) > 0.0f) {
		movDir = XMVector3Normalize(movDir);
		newVelocity = movDir * movespeed;
		newVelocity = XMVectorSetY(newVelocity, velocity.y); // preserve Y
	}

	XMStoreFloat3(&velocity, newVelocity);

	if (KeyScan[SDL_SCANCODE_SPACE] && grounded) {
		velocity.y = 14.0f;   
		grounded = false;
	}
}

void Player::AddChildCamera(Camera& camera) {
	this->childCam = &camera;

	this->childCam->pos.x = this->transf.position.x; 
	this->childCam->pos.y = this->transf.position.y + (this->Height * 0.9f);
	this->childCam->pos.z = this->transf.position.z;
}

void Player::RemoveChildCamera() {
	if (childCam == nullptr) {
		MessageBoxA(NULL, "No Child Camera Attached.\nCalling RemoveChildCamera() too many times!", "Invalid RemoveChildCamera() call", MB_OK | MB_ICONERROR);
		return;
	}
	else {
		this->childCam = nullptr;
	}
}




Player::~Player() {

}