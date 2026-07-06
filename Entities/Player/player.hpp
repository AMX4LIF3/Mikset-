#pragma once
#include "../Entity/entity.hpp"
#include <SDL2/SDL.h>

struct Capsule {
	XMFLOAT3 base; // (world space)
	XMFLOAT3 tip;  // freaky.. (also world space)
	float radius = 0.4f;
	Capsule() : base(0, 0, 0), tip(0, 1, 0), radius(0.5f) {}; // default constructor so this shit doesnt scream at me

	Capsule(XMFLOAT3 B, XMFLOAT3 T, float R)
		: base(B), tip(T), radius(R) { /* (. Y. ) delicioso */ }
};

class Player : public Entity{
	public: 
		Player() : Height(1.0f) {};
		Player(float height);


		~Player();

		void Update(float dt); 
		void HandleInput(float dt, int movespeed, const SDL_Event& event, const Uint8* KeyScan, XMVECTOR camForward, XMVECTOR camRight);

		void SetModel(Model* model) {
			model = model;
		}
		
		void AddChildCamera(Camera& camera);
		void RemoveChildCamera();
		Capsule& GetCollider() { return this->Collider; }
		
		float Height = 1.0f; 
		XMFLOAT3 velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
		bool grounded = false;
	private:
		Capsule Collider;
		Camera* childCam;
};