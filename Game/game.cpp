#include "game.hpp"
#include <chrono>
#include <math.h>

static float REFRESH_RATE = 60.0f;
Game::Game()
	: m_isRunning(false),
	camera({ 8.0f, 13.0f, -20.0f })
{
	// attempt initialization Run() depends on this flag
	m_isRunning = Initialize();
	
	// Note: probably Core should handle these stuff.
	DEVMODE devmode;
	devmode.dmSize = sizeof(DEVMODE);
	if (EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devmode)) REFRESH_RATE = (float)devmode.dmDisplayFrequency;
	else MessageBoxA(NULL, "Failed to Capture monitor's refresh rate..\n", "Failed to Capture Monitor profile!", MB_OK | MB_ICONWARNING);
}

Game::~Game() { // do i really need this?
}

bool Game::Initialize() {
	core.LET_THERE_BE_DX11();
	SDL_SetRelativeMouseMode(SDL_TRUE);	
	this->scene.SetUpBuffers(core);

	// compile vertex shader first because the blob will be flushed
	// i assume this is how its supposed to work and my implementation is not flawed.. yeah i think the answer is clear but fuck it
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
	
	//Entity* robot = scene.AddEntity(core, "robot", "Assets/3D/steampunk_robot.glb", true);
	Entity* platform = scene.AddEntity(core, "plat", "Assets/3D/cube.glb", true);
	platform->transf.position.y = -4;
	platform->transf.scale = {10, 1, 10};
	Collider platCol;
	platCol.Type = CollisionType::TriangleMesh;
	platform->colliders.push_back(platCol);

	Entity* map = scene.AddEntity(core, "map", "Assets/3D/sponza.glb", true, true);
	//map->MulByRootNodeTransf = true;
	map->transf.scale = {3,3,3};
	map->material.Roughness = 0.9f;
	map->material.Metallic = 1;
	Collider mapcol;
	mapcol.Type = CollisionType::TriangleMesh;
	map->colliders.push_back(mapcol);

/*
	Entity* moon = scene.AddEntity(core, "MOON", "Assets/3D/moon_surface/Moon_-_Giordano_Bruno_Crater.obj", true);
	Collider moonCol;
	moon->transf.scale = {2.0f, 2.0f, 2.0f};
	moonCol.Type = CollisionType::TriangleMesh;
//	moon->colliders.push_back(moonCol);
*/




	this->viewModel = std::make_unique<Entity>("empty", scene.GetModel(core, "Assets/3D/cube.glb", false), false);

	player = std::make_unique<Player>(2.6f);
	ctx.player = player.get();
	player.get()->transf.position = { 0.0f, 30.0f, 0.0f }; // 3.0, 7.0, -6.0
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
			
			if (!io.WantCaptureKeyboard && event.key.keysym.scancode == SDL_SCANCODE_C) system("cls");
			
			if (event.key.keysym.scancode == SDL_SCANCODE_I) ignoreUI = !ignoreUI;
			if (!io.WantCaptureKeyboard && relativeMouse && event.key.keysym.scancode == SDL_SCANCODE_1) player.get()->RemoveChildCamera();
			if (!io.WantCaptureKeyboard && relativeMouse && event.key.keysym.scancode == SDL_SCANCODE_2) player.get()->AddChildCamera(camera);
			if (!io.WantCaptureKeyboard && relativeMouse && event.key.keysym.scancode == SDL_SCANCODE_3) {
				player.get()->transf.position = {1, 10, 1};
				player.get()->velocity.y = 0.0f;
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
		
	
	}
	scene.End(core, MainShader);
	
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
		core.ClearDepth();
	}	
	
	Interface->DrawEditor(ctx);
	core.m_swapChain->Present(core.vsync, 0);
}
float accumulator = 0.0f;
float fixedDt = 1.0f / REFRESH_RATE; 

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
		collisionHandler.ResolvePlayer(*player.get(), collidableEntities, fixedDt); // run the physics engine at a const value
		accumulator -= fixedDt;
	}
	
	

	camera.UpdateViewMatrix();
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
