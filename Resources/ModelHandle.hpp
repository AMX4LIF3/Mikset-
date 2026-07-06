#pragma once

#include <vector>
#include <map>
#include <string>
#include <unordered_map>

#include "../../Core/common.hpp"
#include "../../Core/core.hpp"
#include "../../Renderer/Graphics/shaders.hpp"
#include "../../Renderer/Camera/camera.hpp"
#include "../Collider/collider.hpp"
#include <d3d11.h>
#include "TextureHandle.hpp"
#include <wrl/client.h>
#include <DirectXTex.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using Microsoft::WRL::ComPtr;

#define MAX_BONE_COUNT 128

struct BoneInfo {
	uint32_t ID; // index
	XMMATRIX MatrixOffset; // inverse bind pose
	std::string NodeName;
};

struct KeyPosition { double Time; XMVECTOR Value; };
struct KeyRotation { double Time; XMVECTOR Value; };
struct KeyScaling  { double Time; XMVECTOR Value; };

struct NodeAnimChannel {
	std::string NodeName;
	std::vector<KeyPosition> Positions;
	std::vector<KeyRotation> Rotations;
	std::vector<KeyScaling>  Scalings;
};

struct AnimationClip {
	std::string Name;
	double DurationTicks = 0.0;
	double TicksPerSecond = 25.0; // fallback
	std::vector<NodeAnimChannel> Channels;
};

class Mesh {
public:
	Mesh(Core& core, std::vector<vertex>& vertices, std::vector<unsigned int>& indices, std::vector<Texture>& textures, bool ClearMem);
	~Mesh();

	void Draw(Core& core);
	const std::vector<vertex>& GetVertexData() const;

	std::vector<vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	std::vector<BVHNode> BVHNodes; 

private:
	void SetUpMesh(Core& core, bool ClearMem);
	ComPtr<ID3D11Buffer> VertexBuff, IndexBuff;
	UINT indexCount;
};

class Model {
public:
	Model(Core& core, const std::string path, bool clearData, bool BakeModelTransformations = false);
	~Model();

	void LoadModel(const std::string& path);
	std::vector<Texture> LoadTexture(aiMaterial* mat, aiTextureType textyp, std::string typeName, const aiScene* scene);
	void Draw();
	void Update(float dt); // TODO: might be better to move the animation update stuff in UpdateAnimation() soon. for now couldnt care less

	bool ClearData;
	void ClearVertexData();
	const std::vector<vertex>& GetVertexData();
	XMMATRIX* GetBonePallete();
	std::string GetPath() {
		if (this->pathcpy.empty()) {
			MessageBoxA(NULL, "The path is empty.. i honestly dont know what you did to get this error", "Invalid GetPath() call!", MB_OK | MB_ICONERROR);
			return std::string(std::to_string((size_t)INT_MAX)); // if it crashes.. welp thats a YOU problem 
		}

		return this->pathcpy;
	}

	XMMATRIX GlobalInverseTransf;
	bool HasBones;

	// animation API
	bool HasAnimations() const { return !m_Animations.empty(); }
	void PlayAnimation(size_t index = 0, bool loop = true);
	void PlayAnimationTimeStamp(size_t index, float startTime, float endTime, bool loop = true); 
	void StopAnimation();
	void SetAnimationSpeed(float speed);
	void UpdateAnimation(float deltaTime);
	bool IsPlaying() const { return m_CurrentAnim != SIZE_MAX; }
	bool IsAnimating = false; 
	bool BakeTransformations = false; 
	std::vector<Mesh> mesh;

	float Anim_StartTick;
	float Anim_EndTick;

	bool IsValid() {
		return Valid;
	}

	XMMATRIX GetRootNodeTransformXM() { return this->RootNodeTransf; }

private:
	bool Valid = false;
	void Subdivide(Mesh& mesh, int nodeIndex, int depth);
	std::vector<BoneInfo> BoneData;
	std::map<std::string, uint32_t> BoneMap;
	XMMATRIX BonePallete[MAX_BONE_COUNT];
	std::string pathcpy;

	//void ProcessNode(aiNode* node, const aiScene* scene);
	const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const std::string NodeName);
	Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene);
	Mesh ProcessStaticMeshAsSkinned(aiMesh* mesh, const aiScene* scene, const std::string& parentBoneName);
	void ReadNodeHierarchy(float AnimationTime, aiNode* pNode, XMMATRIX ParentTransform);
	void UpdateHierarchy(aiNode* node, XMMATRIX parentTransf);
	// node selection that prefers meshes with skinning information
	void ProcessNodeSelectBestMesh(aiNode* node, const aiScene* scene);
	aiNode* RootNode;
	XMMATRIX RootNodeTransf;

	Assimp::Importer importer;
	std::string dir;
	Core& core;
	std::vector<Texture> loadedTextures;

	std::vector<AnimationClip> m_Animations;
	size_t m_CurrentAnim = SIZE_MAX;

	double m_AnimTimeSec = 0.0;
	float  m_AnimSpeed	 = 1.0f;
	bool   m_AnimLoop	 = true;
	float  TimeStamped	 = false;

	std::unordered_map<std::string, XMMATRIX> m_AnimatedLocalTransforms;

	XMMATRIX SampleNodeTransform(const NodeAnimChannel& channel, double timeSeconds) const;
	XMVECTOR InterpPosition(const std::vector<KeyPosition>& keys, double time) const;
	XMVECTOR InterpRotation(const std::vector<KeyRotation>& keys, double time) const;
	XMVECTOR InterpScaling(const std::vector<KeyScaling>& keys, double time) const;
};

