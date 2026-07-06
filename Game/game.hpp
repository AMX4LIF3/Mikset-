# pragma once 
#include "../Core/core.hpp"
#include "../Renderer/Graphics/shaders.hpp"
#include "../Renderer/Camera/camera.hpp"
#include "../../Resources/ModelHandle.hpp"
#include "../../Renderer/Graphics/Scene/scene.hpp"
#include "../../Renderer/Graphics/Debug/Collider/ColliderDebug.hpp"
#include "../Collider/CollisionHandler.hpp"
#include "../UI/interface.hpp"
#include "../Entities/Entity/entity.hpp"
#include "../Entities/Player/player.hpp"
#include <chrono>


class Game {
	public:
		Game();
		~Game();

		bool Initialize();
		void Run();
        void ProcessInput();
		void Update();
		void Render();

		float deltaTime = 0.0f; 
		std::chrono::steady_clock::time_point lastframeTime;
		void TickUpdate();
		
		Core core; 
		bool m_isRunning;

		Shader MainShader;
		Shader ColliderShader;
		CollisionHandler collisionHandler;
		ColliderDebug colliderDebug;
		ComPtr<ID3D11Buffer> m_buffobj;

		ComPtr<ID3D11Buffer> m_cbuff;
		ComPtr<ID3D11Buffer> m_indexbuff;
		
		Scene scene;
		Camera camera; 
		std::unique_ptr<CubeMap> skybox;

		std::unique_ptr<InterFace> Interface; 
		InterfaceCtx ctx; 
		
		std::unique_ptr<Player> player;
		Player* playerPtr;
		
		//std::unique_ptr<Model> playerModel; // TODO: actually be able to use this

		std::unique_ptr<Entity> viewModel; 
		XMMATRIX viewModelMat;
};
