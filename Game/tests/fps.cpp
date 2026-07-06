#include "game.hpp"
#include <chrono>

Game::Game()
	: m_isRunning(false),
	camera({ 8.0f, 13.0f, -20.0f })
{
	// attempt initialization Run() depends on this flag
	m_isRunning = Initialize();
}

Game::~Game() { // do i really need this?
}

bool Game::Initialize() {
	core.LET_THERE_BE_DX11();
	SDL_SetRelativeMouseMode(SDL_TRUE);	
	this->scene.SetUpBuffers(core);

	// compile vertex shader first because the blob will be flushed
	MainShader.VSCompileFromFile(core, core.GetDevice(), "Renderer/Graphics/shaders/main.hlsl", "MainShader");

	core.m_device->CreateInputLayout(
		scene.MainShaderIE_desc, _countof(scene.MainShaderIE_desc), // life saver istg
		MainShader.GetBlob()->GetBufferPointer(),
		MainShader.GetBlob()->GetBufferSize(),
		MainShader.m_IL.GetAddressOf()
	);

	MainShader.PSCompileFromFile(core, core.GetDevice(), "Renderer/Graphics/shaders/main.hlsl", "MainShader");
	core.m_deviceCtx->IASetInputLayout(MainShader.m_IL.Get()); // set input layout
	colliderDebug.Init(core);


	// UI stuff
	//ctx.lightPos = &scene.gb.LightPos;
	//ctx.cameraPos = &camera.pos;
	//ctx.entities = &scene.GetEntities();
	ctx.viewModel = &viewModel;
	ctx.scene = &scene;
	ctx.camera = &camera;

	this->Interface = std::make_unique<InterFace>(core.window, core);
/*
	moon = scene.AddEntity(core, "Moon", "Assets/3D/moon_surface/Moon_-_Giordano_Bruno_Crater.obj", true);
//	moon = scene.AddEntity(core, "sea", "Assets/3D/sea.glb", true);
	moon->material.Roughness = 0.25f;
	moon->material.AmbientOcclusion = 0.5f;
	moon->transf.scale = { 1.0f, 1.0f, 1.0f };
	moon->transf.rotation = { 0.0f, 0.0f, 0.0f };
	Collider moon_BVH; 
	moon_BVH.Type = CollisionType::TriangleMesh;
	moon->colliders.push_back(moon_BVH);
*/
	
	//Collider mooncollider;
    //mooncollider.Type = CollisionType::Box;
	//mooncollider.Min = XMFLOAT3(-15, -4, -15);
//	mooncollider.Max = XMFLOAT3(15, 0, 15);
//	moon->colliders.push_back(mooncollider);
	

	viewModel = std::make_unique<Entity>("View Model", scene.GetModel(core, "Assets/3D/ak12.glb", true), false);
	//viewModel.get()->material.Metallic = 1.0f;
	//viewModel.get()->material.Roughness = 0.45f;
	//viewModel.get()->material.AmbientOcclusion = 0.75f;
//	viewModel.get()->transf.scale = { 0.5f, 0.5f, 0.5f };
	//viewModel.get()->transf.rotation = { 3.04f, 0.268f, -1.75f };
	//viewModel.get()->transf.position = { 1.17f, -1.41f, 0.496f };
	viewModel.get()->material.Metallic = 0.0f;
	viewModel.get()->material.Roughness = 0.8f;
	viewModel.get()->material.AmbientOcclusion = 1.0f;
	viewModel.get()->transf.scale = { 1.0f, 1.0f, 1.0f };
	//viewModel.get()->transf.rotation = { 0.117f, -1.339f, 0.0f };
	//viewModel.get()->transf.position = { 1.5f, 0.623f, 0.211f };

	viewModel.get()->transf.position = { 0.0f, 0.6f, 0.75f};
	viewModel.get()->transf.rotation = { 0.121f, - 1.331f, -0.138f };

	viewModel.get()->GetModel()->PlayAnimation(1, true);

	//viewModel.get()->GetModel()->PlayAnimationTimeStamp(0, 2.0f, 5.0f, true);
	
	Entity* house = scene.AddEntity(core, "house", "Assets/3D/house.glb", true);
	house->transf.rotation.x = 1.57;
	Collider housecol;
	housecol.Type = CollisionType::TriangleMesh;
	house->colliders.push_back(housecol);


	//auto robot = scene.AddEntity(core, "Robot", "Assets/3D/steampunk_robot.glb", true);
	//Collider RobotCollider;
//	RobotCollider.Type = CollisionType::TriangleMesh;
//	robot->colliders.push_back(RobotCollider);

	
	
	//viewModel.get()->material.Metallic = 1.0f;
	//viewModel.get()->material.Roughness = 0.45f;
	//viewModel.get()->material.AmbientOcclusion = 0.75f;
//	viewModel.get()->transf.scale = { 0.5f, 0.5f, 0.5f };
	//viewModel.get()->transf.rotation = { 3.04f, 0.268f, -1.75f };
	//viewModel.get()->transf.position = { 1.17f, -1.41f, 0.496f };
	viewModel.get()->material.Metallic = 0.0f;
	viewModel.get()->material.Roughness = 0.8f;
	viewModel.get()->material.AmbientOcclusion = 1.0f;
	viewModel.get()->transf.scale = { 1.0f, 1.0f, 1.0f };
	//viewModel.get()->transf.rotation = { 0.117f, -1.339f, 0.0f };
	//viewModel.get()->transf.position = { 1.5f, 0.623f, 0.211f };

	viewModel.get()->transf.position = { -0.276f, 1.6f, 0.750f };
	viewModel.get()->transf.rotation = { 0.121f, -1.331f, -0.138f };

	viewModel.get()->GetModel()->PlayAnimation(1, true);

	player = std::make_unique<Player>(1.6f);
	player.get()->transf.position = { 45.0f, 16.0f, -123.0f }; // 3.0, 7.0, -6.0
	player.get()->AddChildCamera(camera);
	

	skybox = CubeMap::Create(core, L"Assets/2D/Cubemaps/Sunset/skybox_temp.dds");
	scene.SetSkyBox(skybox.get());
	return true;
}

SDL_Event event;
const Uint8* KeyScan = SDL_GetKeyboardState(nullptr);
void Game::ProcessInput() {
	static float speed = 20.0f;
	ctx.Flyspeed = &speed;

	float flyspeed = speed * this->deltaTime;


	ImGuiIO& io = ImGui::GetIO();
	static bool relativeMouse = true;
	static bool ignoreUI = false;
	while (SDL_PollEvent(&event)) {
		Interface->ProcessEvent(event);

		if (event.type == SDL_QUIT) {
			std::cout << "AHRGRHGGHHEAHHR-\n";
			m_isRunning = false;
		}
		if (event.type == SDL_KEYDOWN) {
		//	if (event.key.keysym.scancode == SDL_SCANCODE_Q) m_isRunning = false;
			if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
				relativeMouse = !relativeMouse;
				SDL_SetRelativeMouseMode(relativeMouse ? SDL_TRUE : SDL_FALSE);
			}
			if (event.key.keysym.scancode == SDL_SCANCODE_C) system("cls");
			
			if (event.key.keysym.scancode == SDL_SCANCODE_R) {
				//viewModel.get()->GetModel()->PlayAnimationTimeStamp(0, 0.4f, 2.75f, false);
				viewModel.get()->GetModel()->PlayAnimation(3, false);
			}

			if (event.key.keysym.scancode == SDL_SCANCODE_I) ignoreUI = !ignoreUI;
			if (relativeMouse && event.key.keysym.scancode == SDL_SCANCODE_1) {
				player.get()->RemoveChildCamera();
				viewModel.get()->transf.position.y = 100.0f;
			}
			if (relativeMouse && event.key.keysym.scancode == SDL_SCANCODE_2) {
				player.get()->AddChildCamera(camera);
				viewModel.get()->transf.position.y = -1.41;
			}
			if (relativeMouse && event.key.keysym.scancode == SDL_SCANCODE_3) {
				player.get()->transf.position = {1, 10, 1};
				player.get()->velocity.y = 0.0f;
			}
		}
		if (relativeMouse && event.type == SDL_MOUSEBUTTONDOWN) {
			if (event.button.button == SDL_BUTTON_LEFT) {
				//viewModel.get()->GetModel()->PlayAnimationTimeStamp(0, 0.0f, 0.4f, false);
				viewModel.get()->GetModel()->PlayAnimation(2,false);
			}
			else {
				viewModel.get()->GetModel()->PlayAnimationTimeStamp(0, 7.8f, 8.9f, true);

			}
		}
	}
	
	if (!ignoreUI && !io.WantCaptureMouse && relativeMouse) {
		io.WantCaptureMouse = false;
		int mouseX = 0, mouseY = 0;
		SDL_GetRelativeMouseState(&mouseX, &mouseY);
		static float sensitivity = 0.003f;
		ctx.sensitivity = &sensitivity;
		sensitivity = *ctx.sensitivity;

		if (mouseX != 0 || mouseY != 0) {
			camera.Rotate((float)mouseX * sensitivity, (float)mouseY * sensitivity);
		}

	}
	else if (ignoreUI) { // because UI stops the mouse i do this.
		io.WantCaptureMouse = false;
		int mouseX = 0, mouseY = 0;
		SDL_GetRelativeMouseState(&mouseX, &mouseY);
		static float sensitivity = 0.003f;
		ctx.sensitivity = &sensitivity;
		sensitivity = *ctx.sensitivity;
		//relativeMouse = true;

		if (mouseX != 0 || mouseY != 0) {
			camera.Rotate((float)mouseX * sensitivity, (float)mouseY * sensitivity);
		}
	}

}

float ClearColor[4] = { 0.0f, 0.0f, 0.1f, 1.0f };

void Game::Render() {
	core.m_deviceCtx->ClearRenderTargetView(core.m_rtv.Get(), ClearColor);
	core.ClearDepth();


	scene.UpdateShadowMap(core, player.get());
	if (scene.Begin(core, camera, MainShader))
	{
		for (const auto& entity : this->scene.GetEntities()) {
			scene.Submit(core, entity.get(), entity->GetWorldMat());
		}

		skybox.get()->Draw(core, camera, MainShader, MainShader.m_IL.Get()); 		 
		
		static bool colliderview = false; 
		ctx.ColliderView = &colliderview;
		XMMATRIX viewProj = camera.GetViewProjection();
		if (colliderview) {
			colliderDebug.Draw(
				core.m_deviceCtx.Get(), viewProj,
				scene.GetEntities(), player.get(),
				MainShader, MainShader.m_IL.Get(),
				scene.GetGlobalBuffer().Get()
			);
		}

		core.ClearDepth();
		scene.Submit(core, viewModel.get(), viewModelMat);
		
	}
	scene.End(core, MainShader);


	Interface->Draw(ctx);
	core.m_swapChain->Present(core.vsync, 0);
}

float accumulator = 0.0f;
float fixedDt = 1.0f / 115.0f; 
void Game::Update() {
	TickUpdate();
	XMMATRIX camRot = camera.GetRotationMatrix(false);
	XMVECTOR camRight = camRot.r[0];
	XMVECTOR camForward = camRot.r[2];
	player.get()->HandleInput(deltaTime, *ctx.Flyspeed, event, KeyScan, camForward, camRight);
	player.get()->Update(deltaTime);
	

	std::vector<Entity*> collidableEntities;
	for (const auto& entity : scene.GetEntities()) {
		entity->Update(this->ctx.dt);
		entity->GetModel()->Update(deltaTime);
		if (!entity->colliders.empty()) collidableEntities.push_back(entity.get());

	}
	
	accumulator += deltaTime;
	while (accumulator >= fixedDt) {
		collisionHandler.ResolvePlayer(*player.get(), collidableEntities, fixedDt); // run the physics engine at a const 115fps
		accumulator -= fixedDt;
	}


	camera.UpdateViewMatrix();
	
	viewModelMat = XMMatrixIdentity();
	viewModelMat *= viewModel.get()->GetWorldMat(); 
	camRot = camera.GetRotationMatrix(false);
	XMMATRIX pos = XMMatrixTranslationFromVector(XMLoadFloat3(&camera.pos));
	XMMATRIX offsetRot = XMMatrixRotationRollPitchYaw(2.9f, 1.56f, -1.8f);
	viewModelMat *= XMMatrixScaling(1.0f, 1.0f, 1.0f) * offsetRot * XMMatrixTranslation(0.7f, -1.685f, 1.5f) * camRot * pos;
	
	viewModel.get()->GetModel()->Update(deltaTime);
//0.7f, -1.685f, 0.381f
}

void Game::TickUpdate() {
	auto currentFrameTime = std::chrono::steady_clock::now();
	std::chrono::duration<float> duration = currentFrameTime - this->lastframeTime;
	this->deltaTime = duration.count();
	lastframeTime = currentFrameTime;
	
	// cap incase of lag spikes 
	if (this->deltaTime > 0.1f) this->deltaTime = 0.1f;

	this->ctx.dt = this->deltaTime;
}


void Game::Run() {
	while (m_isRunning) {
		ProcessInput();
		Update();
		Render();
	}
}

