#pragma once

#include "../Core/core.hpp"
#include <SDL2/SDL.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <C:/vcpkg/buildtrees/imgui/src/v1.91.9-afb09617a6.clean/backends/imgui_impl_sdl2.h>
#include "../Renderer/Graphics/Scene/scene.hpp"

void MiksetText(std::string text, const ImVec4& color = { 1.0f, 1.0f, 1.0f, 1.0f }, const ImVec2& pos = { 0,0 });

typedef struct InterfaceCtx {
	float dt; 
	float* sensitivity;
	float* Flyspeed; 

	XMFLOAT4* lightPos; 
	XMFLOAT3* cameraPos;

	std::vector<std::unique_ptr<Entity>>* entities{};
	std::unique_ptr<Entity>* viewModel;

	bool* ColliderView;
	Player* player;
	Scene* scene;
	Camera* camera;
};

class InterFace {
	public:
		InterFace(SDL_Window* window, Core& core);
		~InterFace();

		void BeginFrame();
		void EndFrame(); 
			
		void DrawEditor(InterfaceCtx& ctx); 

		void ProcessEvent(const SDL_Event& event);
		

	private:
		ImFont* font16p; 
		Core& core; 

};