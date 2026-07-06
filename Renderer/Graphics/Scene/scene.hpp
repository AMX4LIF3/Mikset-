#pragma once
#include <wrl/client.h>
#include <d3d11.h>
#include <unordered_map>

#include "../Buffers/buffers.hpp"
#include "../CubeMap/cubemap.hpp"
#include "../../Resources/ModelHandle.hpp"
#include "../../Entities/Entity/entity.hpp" 
#include "../ShadowMap/shadowmap.hpp"

using namespace Microsoft::WRL;

class Scene
{
	public:
		Scene(); 
		~Scene(); 

		void UpdateShadowMap(Core& core, Player* player);
		bool SetUpBuffers(Core& core); 
		bool Begin(Core& core, Camera& cam, Shader& shader); 
		void Submit(Core& core, Entity* entity, XMMATRIX transform); 
		void SubmitWithCustomView(Core& core, Entity* entity, XMMATRIX transform, XMMATRIX view);

		void End(Core& core, Shader shader); 

		Entity* AddEntity(Core& core, const std::string& name, const std::string& path, bool IsCollidable, bool BakeModelTransformations = false);
		void RemoveEntity(Entity* entity);
		void RemoveEntityByName(std::string name);
		std::vector<std::unique_ptr<Entity>>& GetEntities();
		std::shared_ptr<Model> GetModel(Core& core, const std::string& path, bool IsCollidable);

		GlobalBuffer gb;
		ObjectBuffer oj;

		ComPtr<ID3D11Buffer>& GetGlobalBuffer();
		ComPtr<ID3D11Buffer>& GetObjectBuffer();
		ComPtr<ID3D11Buffer>& GetMaterialBuffer();
		void SetSkyBox(CubeMap* cubemap) { SkyBox = cubemap; }
		
		static constexpr D3D11_INPUT_ELEMENT_DESC MainShaderIE_desc[] =
		{
			{
				"POSITION",				   	 
				0,							 
				DXGI_FORMAT_R32G32B32_FLOAT, 
				0,							 
				D3D11_APPEND_ALIGNED_ELEMENT,
				D3D11_INPUT_PER_VERTEX_DATA, 
				0							 
			},

			{
				"COLOR",
				0,
				DXGI_FORMAT_R32G32B32_FLOAT,
				0,
				D3D11_APPEND_ALIGNED_ELEMENT, 
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			},

			{
				"NORMAL",
				0,
				DXGI_FORMAT_R32G32B32_FLOAT,
				0,
				D3D11_APPEND_ALIGNED_ELEMENT,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			},


			{
				"TEXCOORD",
				0,
				DXGI_FORMAT_R32G32_FLOAT,
				0,
				D3D11_APPEND_ALIGNED_ELEMENT,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			},

			{
				"TANGENT",
				0,
				DXGI_FORMAT_R32G32B32_FLOAT,
				0,
				D3D11_APPEND_ALIGNED_ELEMENT,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			},

			{
				"WEIGHTS",
				0,
				DXGI_FORMAT_R32G32B32A32_FLOAT,
				0,
				D3D11_APPEND_ALIGNED_ELEMENT,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			},

			{
				"BONEIDS",
				0,
				DXGI_FORMAT_R32G32B32A32_UINT,
				0,
				D3D11_APPEND_ALIGNED_ELEMENT,
				D3D11_INPUT_PER_VERTEX_DATA,
				0
			},
		};

	private: 
		ComPtr<ID3D11Buffer> globBuff; 
		ComPtr<ID3D11Buffer> objBuff; 
		ComPtr<ID3D11Buffer> matBuff;
		ComPtr<ID3D11Buffer> skinningBuff;

		std::unordered_map <std::string, std::shared_ptr<Model>> modelCache;
		XMMATRIX viewProjCache;

		Camera* camera = nullptr; 
		std::vector<std::unique_ptr<Entity>> entities;

		CubeMap* SkyBox = nullptr;

		ShadowMap shadowmap;
		Shader shadowShader;
		XMFLOAT4X4 lightViewProj;

		std::vector<Entity*> renderQueue; 
		// track the last model we uploaded skinning data for this frame to avoid redundant uploads
		Model* lastSkinnedModelUploaded = nullptr;

	};

