/*		***Mikset alpha***

 OAWWH MAE GOWWD DE LORRRRRDDD
		DE LOORRRDD
|___________________________|
	/|\-| Amx4lif3 |-/|\

*/

#include "ModelHandle.hpp"
#include <algorithm>
#include <limits>
#include <cmath>
#include <chrono>


using Microsoft::WRL::ComPtr;

Mesh::Mesh(Core& core, std::vector<vertex>& vertices, std::vector<unsigned int>& indices, std::vector<Texture>& textures, bool ClearMem)
	: vertices(vertices), indices(indices), textures(textures), indexCount(0)
{
	SetUpMesh(core, ClearMem);
}

Mesh::~Mesh() {}

void Mesh::SetUpMesh(Core& core, bool ClearMem) {
	if (this->vertices.empty() || this->indices.empty()) return;

	D3D11_BUFFER_DESC VertexBuff_desc{};
	VertexBuff_desc.ByteWidth = static_cast<UINT>(this->vertices.size() * sizeof(vertex));
	VertexBuff_desc.Usage = D3D11_USAGE_DEFAULT;
	VertexBuff_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	VertexBuff_desc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA SubResData{};
	SubResData.pSysMem = this->vertices.data();
	core.m_device->CreateBuffer(&VertexBuff_desc, &SubResData, VertexBuff.GetAddressOf());

	D3D11_BUFFER_DESC IndexBuff_desc{};
	IndexBuff_desc.ByteWidth = static_cast<UINT>(this->indices.size() * sizeof(UINT));
	IndexBuff_desc.Usage = D3D11_USAGE_DEFAULT;
	IndexBuff_desc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexbuff_subres{};
	indexbuff_subres.pSysMem = this->indices.data();
	core.m_device->CreateBuffer(&IndexBuff_desc, &indexbuff_subres, IndexBuff.GetAddressOf());

	this->indexCount = static_cast<UINT>(indices.size());

	if (ClearMem) {
		this->vertices.clear(); this->indices.clear();
		this->vertices.shrink_to_fit(); this->indices.shrink_to_fit();
	}
}

void Mesh::Draw(Core& core) {
	UINT stride = sizeof(vertex); UINT offset = 0;
	core.m_deviceCtx->PSSetShaderResources(0, 1, core.defaultAlbedo.GetAddressOf());
	core.m_deviceCtx->PSSetShaderResources(1, 1, core.defaultNormal.GetAddressOf());
	core.m_deviceCtx->PSSetShaderResources(2, 1, core.defaultMetallic.GetAddressOf());
	core.m_deviceCtx->PSSetShaderResources(3, 1, core.defaultRoughness.GetAddressOf());
	core.m_deviceCtx->PSSetShaderResources(4, 1, core.defaultAO.GetAddressOf());

	core.m_deviceCtx->IASetVertexBuffers(0, 1, VertexBuff.GetAddressOf(), &stride, &offset);
	core.m_deviceCtx->IASetIndexBuffer(IndexBuff.Get(), DXGI_FORMAT_R32_UINT, 0);

	for (const auto& tex : textures) {
		if (tex.type == "albedo") core.m_deviceCtx->PSSetShaderResources(0, 1, tex.srv.GetAddressOf());
		if (tex.type == "normal") core.m_deviceCtx->PSSetShaderResources(1, 1, tex.srv.GetAddressOf());
		if (tex.type == "metallic") core.m_deviceCtx->PSSetShaderResources(2, 1, tex.srv.GetAddressOf());
		if (tex.type == "roughness") core.m_deviceCtx->PSSetShaderResources(3, 1, tex.srv.GetAddressOf());
		if (tex.type == "ao") core.m_deviceCtx->PSSetShaderResources(4, 1, tex.srv.GetAddressOf());
	}

	core.m_deviceCtx->DrawIndexed(this->indexCount, 0, 0);
}

const std::vector<vertex>& Mesh::GetVertexData() const {
	static const std::vector<vertex> empty;
	if (!vertices.empty()) return vertices;
	MessageBoxA(NULL, "vertices is a null value", "GetVertexData() Failed...", MB_OK | MB_ICONWARNING);
	return empty;
}

std::wstring StrToWs(const std::string& str) {
	return std::wstring(str.begin(), str.end());
}

// yeah dont ask me why this isnt in TextureHandle okay?
ComPtr<ID3D11ShaderResourceView> LoadTextureResource(Core& core, const aiScene* scene, aiString* par_filename, std::string dir) {
	HRESULT hr;
	ScratchImage scratchimg = ScratchImage{};

	// check if the texture is embedded like .gbl or other formats
	if (par_filename->C_Str()[0] == '*') {
		const aiTexture* embeddedTex = scene->GetEmbeddedTexture(par_filename->C_Str());
		if (embeddedTex) {
			if (embeddedTex->mHeight == 0) {  // if its compressed: jpg, png etc.
				hr = LoadFromWICMemory(
					reinterpret_cast<const uint8_t*>(embeddedTex->pcData),
					static_cast<size_t>(embeddedTex->mWidth), // just to be safe
					WIC_FLAGS_NONE, nullptr, scratchimg
				);
			}
			else { // rare case where its raw RGB data
				hr = scratchimg.Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM, embeddedTex->mWidth, embeddedTex->mHeight, 1, 1);
				if (SUCCEEDED(hr)) memcpy(scratchimg.GetPixels(), embeddedTex->pcData, scratchimg.GetPixelsSize());
			}
		}
	}
	else { // regular fbx or obj that has seperate png's and jpg's (note: fbx doesnt work in the engine but thats fine cuz it sux)  
		std::string filename = dir + '/' + std::string(par_filename->C_Str());
		std::wstring widefilename = StrToWs(filename);
		hr = LoadFromWICFile(widefilename.c_str(), WIC_FLAGS_NONE, nullptr, scratchimg);
	}

	if (FAILED(hr)) {
		MessageBoxA(NULL, "LoadFromWICFile() Failed...", "Error loading texture!", MB_OK | MB_ICONERROR);
		return nullptr;
	}

	ScratchImage mipChain;
	hr = GenerateMipMaps(scratchimg.GetImages(), scratchimg.GetImageCount(), scratchimg.GetMetadata(), TEX_FILTER_DEFAULT, 0, mipChain);
	if (FAILED(hr)) mipChain = std::move(scratchimg); // fall back but will make it fuzzy :\

	ComPtr<ID3D11ShaderResourceView> srv = nullptr;
	hr = CreateShaderResourceView(core.GetDevice(), mipChain.GetImages(), mipChain.GetImageCount(), mipChain.GetMetadata(), srv.GetAddressOf());
	return srv;
}

// some helpers
// those are like the mexicans of my code. you barley pay them yet they do the work 
XMMATRIX aiMatrixToXM(const aiMatrix4x4& from) {
	// assimp matrix is row-major, convert to XMMatrix because fuck you fuck you fuyck you
	return XMMATRIX( // 2 HOURS 2 FUCKING HOURS JUST DEALING WITH THIS SHIT 
		from.a1, from.b1, from.c1, from.d1,
		from.a2, from.b2, from.c2, from.d2,
		from.a3, from.b3, from.c3, from.d3,
		from.a4, from.b4, from.c4, from.d4
	);
}

static bool IsMatrixValid(const XMMATRIX& m) {
	XMFLOAT4X4 f; XMStoreFloat4x4(&f, m);
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			float v = f.m[i][j];
			if (!std::isfinite(v)) return false;
			if (std::fabs(v) > 1e6f) return false;
		}
	}
	return true;
}

// compute max abs element
static float MatrixMaxAbs(const XMMATRIX& m) {
	XMFLOAT4X4 f; XMStoreFloat4x4(&f, m);
	float maxv = 0.0f;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			float v = std::fabs(f.m[i][j]);
			if (v > maxv) maxv = v;
		}
	}
	return maxv;
}

void SetVertexBoneData(vertex& v, uint32_t ID, float weight) {
	if (weight <= 0.0001f) return; // skip zero/negative weights

	if (v.Weights.x == 0.0f) {
		v.BoneIds[0] = ID;
		v.Weights.x = weight;
	}
	else if (v.Weights.y == 0.0f) {
		v.BoneIds[1] = ID;
		v.Weights.y = weight;
	}
	else if (v.Weights.z == 0.0f) {
		v.BoneIds[2] = ID;
		v.Weights.z = weight;
	}
	else if (v.Weights.w == 0.0f) {
		v.BoneIds[3] = ID;
		v.Weights.w = weight;
	}
}

#include <cstdio>
#include <ctime>
inline void LogToFile(const char* msg) {
	FILE* f = nullptr;
	fopen_s(&f, "bone_debug.log", "a");
	if (f) {
		time_t now = time(0);
		tm timeinfo;
		localtime_s(&timeinfo, &now);
		char timebuf[32];
		strftime(timebuf, sizeof(timebuf), "%H:%M:%S", &timeinfo);
		fprintf(f, "[%s] %s", timebuf, msg);
		fclose(f);
	}
	printf("%s", msg);
}

// animation sampling
XMVECTOR Model::InterpPosition(const std::vector<KeyPosition>& keys, double time) const {
	if (keys.empty()) return XMVectorZero();
	if (keys.size() == 1) return keys[0].Value;

	if (time <= keys.front().Time) return keys.front().Value;
	if (time >= keys.back().Time) return keys.back().Value;

	size_t index = 0;
	for (size_t i = 0; i + 1 < keys.size(); ++i) {
		if (time < keys[i + 1].Time) { index = i; break; }
	}
	double t1 = keys[index].Time;
	double t2 = keys[std::min(index + 1, keys.size() - 1)].Time;
	double factor = (t2 - t1) > 0.0 ? (time - t1) / (t2 - t1) : 0.0;
	return XMVectorLerp(keys[index].Value, keys[std::min(index + 1, keys.size() - 1)].Value, (float)factor);
}

XMVECTOR Model::InterpRotation(const std::vector<KeyRotation>& keys, double time) const {
	if (keys.empty()) return XMQuaternionIdentity();
	if (keys.size() == 1) return keys[0].Value;

	if (time <= keys.front().Time) return keys.front().Value;
	if (time >= keys.back().Time) return keys.back().Value;

	size_t index = 0;
	for (size_t i = 0; i + 1 < keys.size(); ++i) {
		if (time < keys[i + 1].Time) { index = i; break; }
	}
	double t1 = keys[index].Time;
	double t2 = keys[std::min(index + 1, keys.size() - 1)].Time;
	double factor = (t2 - t1) > 0.0 ? (time - t1) / (t2 - t1) : 0.0;
	XMVECTOR a = keys[index].Value;
	XMVECTOR b = keys[std::min(index + 1, keys.size() - 1)].Value;
	XMVECTOR out = XMQuaternionSlerp(a, b, (float)factor);
	return XMQuaternionNormalize(out);
}

XMVECTOR Model::InterpScaling(const std::vector<KeyScaling>& keys, double time) const {
	if (keys.empty()) return XMVectorSet(1, 1, 1, 0);
	if (keys.size() == 1) return keys[0].Value;

	if (time <= keys.front().Time) return keys.front().Value;
	if (time >= keys.back().Time) return keys.back().Value;

	size_t index = 0;
	for (size_t i = 0; i + 1 < keys.size(); ++i) {
		if (time < keys[i + 1].Time) { index = i; break; }
	}
	double t1 = keys[index].Time;
	double t2 = keys[std::min(index + 1, keys.size() - 1)].Time;
	double factor = (t2 - t1) > 0.0 ? (time - t1) / (t2 - t1) : 0.0;
	return XMVectorLerp(keys[index].Value, keys[std::min(index + 1, keys.size() - 1)].Value, (float)factor);
}

XMMATRIX Model::SampleNodeTransform(const NodeAnimChannel& channel, double timeSeconds) const {
	XMVECTOR pos = InterpPosition(channel.Positions, timeSeconds);
	XMVECTOR rot = InterpRotation(channel.Rotations, timeSeconds);
	XMVECTOR scl = InterpScaling(channel.Scalings, timeSeconds);
	return XMMatrixAffineTransformation(scl, XMVectorZero(), rot, pos);
}


XMFLOAT3 SubtractFloat3(XMFLOAT3 A, XMFLOAT3 B) {
	XMVECTOR VecA = XMLoadFloat3(&A);
	XMVECTOR VecB = XMLoadFloat3(&B);

	XMVECTOR VecRes = VecA - VecB;
	XMFLOAT3 Res;
	XMStoreFloat3(&Res, VecRes);
	return Res;
}

float GetCentroid(Mesh& mesh, int index, int axis) {
	// get indices 
	int idx_x = mesh.indices[index * 3 + 0];
	int idx_y = mesh.indices[index * 3 + 1];
	int idx_z = mesh.indices[index * 3 + 2];

	// lookup the specific axis of every vertex
	float v0 = mesh.vertices[idx_x].xyz[axis];
	float v1 = mesh.vertices[idx_y].xyz[axis];
	float v2 = mesh.vertices[idx_z].xyz[axis];

	// average 'em BOII
	return (v0 + v1 + v2) / 3.0f;
}

void SwapTriangles(Mesh& mesh, int A, int B) { // NOTE: can use std::swap but this loks much cooler
	if (A == B) return;

	int startA = A * 3;
	int startB = B * 3;

	for (int i = 0; i < 3; i++) {
		int tempA = mesh.indices[startA + i];
		mesh.indices[startA + i] = mesh.indices[startB + i];
		mesh.indices[startB + i] = tempA;
	}
}

void Model::Subdivide(Mesh& mesh, int nodeIndex, int depth = 0) {
	BVHNode& node = mesh.BVHNodes[nodeIndex];
	if (node.count <= 4 || depth > 25) { // leaf 
		node.isLeaf = true;
		return;
	}

	auto currentIndex = mesh.BVHNodes[nodeIndex].index;
	auto currentCount = mesh.BVHNodes[nodeIndex].count;

	// determine the split axis and pos
	XMFLOAT3 FLT_extent = SubtractFloat3(node.Max, node.Min);
	float extent[3] = { FLT_extent.x, FLT_extent.y, FLT_extent.z };
	int axis = 0;
	if (FLT_extent.y > FLT_extent.x) axis = 1;
	if (FLT_extent.z > extent[axis]) axis = 2;
	float splitPos = ((float*)&node.Min)[axis] + extent[axis] * 0.5f; // im scared of this it might fail


	// now we partition the triangles
	int i = currentIndex;
	int j = currentIndex + currentCount - 1;

	while (i <= j) {
		auto centroid = GetCentroid(mesh, i, axis);
		if (splitPos > centroid) {
			i++;
		}
		else {
			SwapTriangles(mesh, i, j);
			j--;
		}
	}

	auto leftCount = i - currentIndex;
	if (leftCount == 0 || leftCount == currentCount) return;

	XMFLOAT3 leftMin = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
	XMFLOAT3 leftMax = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	for (int tri = currentIndex; tri < currentIndex + leftCount; tri++) {
		for (int v = 0; v < 3; v++) {
			int Vidx = mesh.indices[tri * 3 + v];
			XMFLOAT3 V = XMFLOAT3(mesh.vertices[Vidx].xyz[0], mesh.vertices[Vidx].xyz[1], mesh.vertices[Vidx].xyz[2]);

			if (V.x < leftMin.x) leftMin.x = V.x;
			if (V.x > leftMax.x) leftMax.x = V.x;

			if (V.y < leftMin.y) leftMin.y = V.y;
			if (V.y > leftMax.y) leftMax.y = V.y;

			if (V.z < leftMin.z) leftMin.z = V.z;
			if (V.z > leftMax.z) leftMax.z = V.z;
		}
	}

	XMFLOAT3 rightMin = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
	XMFLOAT3 rightMax = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (int tri = i; tri < currentIndex + currentCount; tri++) {
		for (int v = 0; v < 3; v++) {
			int Vidx = mesh.indices[tri * 3 + v];
			XMFLOAT3 V = XMFLOAT3(mesh.vertices[Vidx].xyz[0], mesh.vertices[Vidx].xyz[1], mesh.vertices[Vidx].xyz[2]);

			if (V.x < rightMin.x) rightMin.x = V.x;
			if (V.x > rightMax.x) rightMax.x = V.x;

			if (V.y < rightMin.y) rightMin.y = V.y;
			if (V.y > rightMax.y) rightMax.y = V.y;

			if (V.z < rightMin.z) rightMin.z = V.z;
			if (V.z > rightMax.z) rightMax.z = V.z;
		}
	}

	// now we make children..
	BVHNode leftNode;
	leftNode.count = i - currentIndex;
	leftNode.index = currentIndex;
	leftNode.Min = leftMin;
	leftNode.Max = leftMax;
	leftNode.left = -1;
	leftNode.right = -1;


	BVHNode rightNode;
	rightNode.count = currentCount - leftCount;
	rightNode.index = i;
	rightNode.Min = rightMin;
	rightNode.Max = rightMax;
	rightNode.left = -1;
	rightNode.right = -1;

	int leftChildIdx = (int)mesh.BVHNodes.size();
	mesh.BVHNodes.push_back(leftNode);

	int rightChildIdx = (int)mesh.BVHNodes.size();
	mesh.BVHNodes.push_back(rightNode);

	mesh.BVHNodes[nodeIndex].left = leftChildIdx;
	mesh.BVHNodes[nodeIndex].right = rightChildIdx;
	mesh.BVHNodes[nodeIndex].count = 0; // no longer a leaf

	Subdivide(mesh, leftChildIdx, depth + 1);
	Subdivide(mesh, rightChildIdx, depth + 1);
}

Model::Model(Core& coreRef, const std::string path, bool clearMem, bool BakeModelTransformations) : core(coreRef), ClearData(clearMem) {
	if (BakeModelTransformations) this->BakeTransformations = true;
	this->LoadModel(path);

	// Build a BVH, i seriously should move this into the entity constructor 
	// start with 1 AABB box around the mesh
	size_t totalNodes = 0;
	switch (clearMem) {
	case false:
		for (auto& Mesh : this->mesh) {
			XMFLOAT3 min = XMFLOAT3(FLT_MAX, FLT_MAX, FLT_MAX);
			XMFLOAT3 max = XMFLOAT3(-FLT_MAX, -FLT_MAX, -FLT_MAX);
			for (auto& Vertex : Mesh.vertices) {
				if (Vertex.xyz[0] < min.x) min.x = Vertex.xyz[0];
				if (Vertex.xyz[0] > max.x) max.x = Vertex.xyz[0];

				if (Vertex.xyz[1] < min.y) min.y = Vertex.xyz[1];
				if (Vertex.xyz[1] > max.y) max.y = Vertex.xyz[1];

				if (Vertex.xyz[2] < min.z) min.z = Vertex.xyz[2];
				if (Vertex.xyz[2] > max.z) max.z = Vertex.xyz[2];
			}



			BVHNode RootNode;
			RootNode.Min = min;
			RootNode.Max = max;
			RootNode.left = -1;
			RootNode.right = -1;
			RootNode.index = 0;
			RootNode.count = Mesh.indices.size() / 3;
			Mesh.BVHNodes.push_back(RootNode);

			// TODO: might add discarding of certain materials such as transparent.

			auto root = Mesh.BVHNodes.begin();
			Subdivide(Mesh, 0, 0);
			XMFLOAT3 overallMin = { FLT_MAX, FLT_MAX, FLT_MAX };
			XMFLOAT3 overallMax = { -FLT_MAX, -FLT_MAX, -FLT_MAX };

			for (auto& Mesh : this->mesh) {
				for (auto& Node : Mesh.BVHNodes) {
					if (Node.count > 0) {  // leaf
						overallMin.x = std::min(overallMin.x, Node.Min.x);
						overallMin.y = std::min(overallMin.y, Node.Min.y);
						overallMin.z = std::min(overallMin.z, Node.Min.z);
						overallMax.x = (std::max)(overallMax.x, Node.Max.x);
						overallMax.y = (std::max)(overallMax.y, Node.Max.y);
						overallMax.z = (std::max)(overallMax.z, Node.Max.z);
						
					}
				}
			}

			/*
			float width = overallMax.x - overallMin.x;
			float height = overallMax.y - overallMin.y;
			float depth = overallMax.z - overallMin.z;
			std::cout << "Model " << path << " local AABB size: "
				<< width << " x " << height << " x " << depth
				<< " (center: " << (overallMin.x + overallMax.x) / 2 << ", ...)\n";
			
			*/
			totalNodes += Mesh.BVHNodes.size();
		}
		std::cout << "BVH Built for model: " << path << " Total Nodes: " << totalNodes << '\n';

		break;

	case true:
		break;
	}

	for (int i = 0; i < MAX_BONE_COUNT; i++) this->BonePallete[i] = XMMatrixIdentity();
}

Model::~Model() {}

void Model::LoadModel(const std::string& path) {
	this->pathcpy = path; // for now ill just record the path, maybe messy but it works
	const aiScene* scene;
	if (this->BakeTransformations) {
		scene = this->importer.ReadFile(
			path,
			aiProcess_CalcTangentSpace |
			aiProcess_GenNormals |
			aiProcess_JoinIdenticalVertices |
			aiProcess_Triangulate |
			aiProcess_GenUVCoords |
			aiProcess_SortByPType |
			aiProcess_LimitBoneWeights | // limits to 4 weights per vertex, commenting this breaks some models 
			aiProcess_ValidateDataStructure |
			aiProcess_ConvertToLeftHanded |
			aiProcess_PreTransformVertices
		);
	}
	else {
		scene = this->importer.ReadFile(
			path,
			aiProcess_CalcTangentSpace |
			aiProcess_GenNormals |
			aiProcess_JoinIdenticalVertices |
			aiProcess_Triangulate |
			aiProcess_GenUVCoords |
			aiProcess_SortByPType |
			aiProcess_LimitBoneWeights |  
			aiProcess_ValidateDataStructure |
			aiProcess_ConvertToLeftHanded 
		//	aiProcess_PreTransformVertices
		);
	}

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		MessageBoxA(NULL, importer.GetErrorString(), "Assimp Error", MB_OK | MB_ICONERROR);
		this->Valid = false;
		return;
	}

	this->Valid = true;
	this->RootNode = scene->mRootNode;
	this->dir = path.substr(0, path.find_last_of('/')); // ahh luv it 

	// select mesh with most bones instead of processing all
	ProcessNodeSelectBestMesh(scene->mRootNode, scene);

	// parse animations
	m_Animations.clear();
	if (scene->mNumAnimations > 0) {
		m_Animations.reserve(scene->mNumAnimations);
		for (UINT a = 0; a < scene->mNumAnimations; ++a) {
			aiAnimation* anim = scene->mAnimations[a];
			AnimationClip clip;
			clip.Name = anim->mName.C_Str();
			clip.DurationTicks = anim->mDuration;
			clip.TicksPerSecond = (anim->mTicksPerSecond != 0.0) ? anim->mTicksPerSecond : 1.0;
			clip.Channels.reserve(anim->mNumChannels);
			for (UINT c = 0; c < anim->mNumChannels; ++c) {
				aiNodeAnim* ch = anim->mChannels[c];
				NodeAnimChannel channel;
				channel.NodeName = ch->mNodeName.C_Str();
				channel.Positions.reserve(ch->mNumPositionKeys);
				for (UINT k = 0; k < ch->mNumPositionKeys; ++k) {
					auto& pk = ch->mPositionKeys[k];
					KeyPosition kp;
					kp.Time = pk.mTime;
					kp.Value = XMVectorSet(pk.mValue.x, pk.mValue.y, pk.mValue.z, 0.0f);
					channel.Positions.push_back(kp);
				}
				channel.Rotations.reserve(ch->mNumRotationKeys);
				for (UINT k = 0; k < ch->mNumRotationKeys; ++k) {
					auto& rk = ch->mRotationKeys[k];
					KeyRotation kr;
					kr.Time = rk.mTime;
					kr.Value = XMVectorSet(rk.mValue.x, rk.mValue.y, rk.mValue.z, rk.mValue.w);
					channel.Rotations.push_back(kr);
				}
				channel.Scalings.reserve(ch->mNumScalingKeys);
				for (UINT k = 0; k < ch->mNumScalingKeys; ++k) {
					auto& sk = ch->mScalingKeys[k];
					KeyScaling ks;
					ks.Time = sk.mTime;
					ks.Value = XMVectorSet(sk.mValue.x, sk.mValue.y, sk.mValue.z, 0.0f);
					channel.Scalings.push_back(ks);
				}
				clip.Channels.push_back(std::move(channel));
			}
			m_Animations.push_back(std::move(clip));
		}
	}

	this->HasBones = !this->BoneMap.empty();

	// then apply unit scale from metadata if present
	float unitScale = 1.0f;
	if (scene->mMetaData) {
		double scale = 1.0;
		if (AI_SUCCESS == scene->mMetaData->Get("UnitScaleFactor", scale)) {
			if (scale > 0.0 && scale != 1.0) {
				unitScale = (float)scale;
			}
		}
	}

	XMMATRIX rootTransform = aiMatrixToXM(scene->mRootNode->mTransformation);
	if (unitScale != 1.0f) {
		XMMATRIX scaleMat = XMMatrixScaling(unitScale, unitScale, unitScale);
		rootTransform = XMMatrixMultiply(rootTransform, scaleMat);
	}

	this->GlobalInverseTransf = XMMatrixInverse(nullptr, rootTransform);
	if (!IsMatrixValid(this->GlobalInverseTransf)) {
		this->GlobalInverseTransf = XMMatrixIdentity();
	}

	for (size_t i = 0; i < MAX_BONE_COUNT; ++i) BonePallete[i] = XMMatrixIdentity();
	UpdateHierarchy(scene->mRootNode, rootTransform);
	this->RootNodeTransf = rootTransform;
}

std::vector<Texture> Model::LoadTexture(aiMaterial* mat, aiTextureType textyp, std::string typeName, const aiScene* scene) {
	std::vector<Texture> textures;

	for (unsigned int i = 0; i < mat->GetTextureCount(textyp); ++i) {
		aiString str;
		mat->GetTexture(textyp, i, &str);
		bool skip = false;

		for (unsigned int j = 0; j < this->loadedTextures.size(); ++j) {
			if (std::strcmp(this->loadedTextures[j].path.data(), str.C_Str()) == 0) {
				textures.push_back(loadedTextures[j]);
				skip = true;
				break;
			}
		}

		if (!skip) {
			Texture texture;
			texture.srv = LoadTextureResource(core, scene, &str, this->dir);
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
			loadedTextures.push_back(texture);
		}
	}
	return textures;
}

void Model::Draw() {
	for (auto& M : mesh) {
		M.Draw(core);
	}
}


// selects the mesh with most bones
void Model::ProcessNodeSelectBestMesh(aiNode* node, const aiScene* scene) {
	this->mesh.clear();

	std::vector<std::pair<aiMesh*, aiNode*>> allMeshes;

	// collect EVERY mesh in the hierarchy and its owner node. even they have slavery ffs
	std::function<void(aiNode*)> collectAll = [&](aiNode* n) {
		for (UINT i = 0; i < n->mNumMeshes; ++i) allMeshes.push_back({ scene->mMeshes[n->mMeshes[i]], n });
		for (UINT i = 0; i < n->mNumChildren; ++i) collectAll(n->mChildren[i]);
		};

	collectAll(scene->mRootNode);

	// process em
	for (auto& [m, n] : allMeshes) {
		if (m->mNumBones > 0) {
			// character / skinned parts
			this->mesh.emplace_back(ProcessMesh(m, scene));
		}
		else {
			// static parts 
			// just treat the Node itself as the bone to keep its transform/scale
			this->mesh.emplace_back(ProcessStaticMeshAsSkinned(m, scene, n->mName.C_Str()));
		}
	}
}

Mesh Model::ProcessStaticMeshAsSkinned(aiMesh* mesh, const aiScene* scene, const std::string& parentBoneName) {
	std::vector<vertex> vertices;
	std::vector<UINT> indices;
	std::vector<Texture> textures;

	for (UINT i = 0; i < mesh->mNumVertices; ++i) {
		vertex V;
		V.xyz[0] = mesh->mVertices[i].x; V.xyz[1] = mesh->mVertices[i].y; V.xyz[2] = mesh->mVertices[i].z;
		V.rgb[0] = 1.0f; V.rgb[1] = 1.0f; V.rgb[2] = 1.0f;
		if (mesh->HasNormals()) {
			V.Normal[0] = mesh->mNormals[i].x; V.Normal[1] = mesh->mNormals[i].y; V.Normal[2] = mesh->mNormals[i].z;
		}
		if (mesh->HasTangentsAndBitangents()) {
			V.tangent[0] = mesh->mTangents[i].x; V.tangent[1] = mesh->mTangents[i].y; V.tangent[2] = mesh->mTangents[i].z;
		}
		if (mesh->mTextureCoords[0]) {
			V.uv[0] = mesh->mTextureCoords[0][i].x; V.uv[1] = mesh->mTextureCoords[0][i].y;
		}
		V.Weights = { 1.0f, 0.0f, 0.0f, 0.0f };
		vertices.push_back(V);
	}

	uint32_t boneID = 0;
	auto it = BoneMap.find(parentBoneName);
	if (it != BoneMap.end()) {
		boneID = it->second;
	}
	else {
		boneID = (uint32_t)BoneData.size();
		BoneInfo newbone;
		newbone.ID = boneID;

		// find the hand node in the scene to get the correct bind pose scale/rotation
		aiNode* handNode = scene->mRootNode->FindNode(parentBoneName.c_str());
		if (handNode) {
			newbone.MatrixOffset = XMMatrixInverse(nullptr, aiMatrixToXM(handNode->mTransformation));
		}
		else {
			newbone.MatrixOffset = XMMatrixIdentity();
		}

		BoneData.push_back(newbone);
		BoneMap[parentBoneName] = boneID;
	}

	for (auto& v : vertices) v.BoneIds[0] = boneID;

	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];

		// le colors
		auto albedo = LoadTexture(mat, aiTextureType_DIFFUSE, "albedo", scene);
		if (albedo.empty()) albedo = LoadTexture(mat, aiTextureType_BASE_COLOR, "albedo", scene); // GLTF specific
		textures.insert(textures.end(), albedo.begin(), albedo.end());

		// normals
		auto norm = LoadTexture(mat, aiTextureType_NORMALS, "normal", scene);
		if (norm.empty()) norm = LoadTexture(mat, aiTextureType_HEIGHT, "normal", scene);
		textures.insert(textures.end(), norm.begin(), norm.end());

		// metal
		auto metal = LoadTexture(mat, aiTextureType_METALNESS, "metallic", scene);
		textures.insert(textures.end(), metal.begin(), metal.end());

		// roughness
		auto rough = LoadTexture(mat, aiTextureType_DIFFUSE_ROUGHNESS, "roughness", scene);
		if (rough.empty()) rough = LoadTexture(mat, aiTextureType_SHININESS, "roughness", scene);
		// GLTF fallback for roughness (note GLTF models are broken when imported in the engine, but glb on top anyways so deal with it)
		if (rough.empty()) rough = LoadTexture(mat, aiTextureType_DIFFUSE_ROUGHNESS, "roughness", scene);
		textures.insert(textures.end(), rough.begin(), rough.end());

		// ambient occlusion
		auto ao = LoadTexture(mat, aiTextureType_AMBIENT_OCCLUSION, "ao", scene);
		if (ao.empty()) ao = LoadTexture(mat, aiTextureType_LIGHTMAP, "ao", scene);
		textures.insert(textures.end(), ao.begin(), ao.end());

		auto emissive = LoadTexture(mat, aiTextureType_EMISSIVE, "emissive", scene);
		textures.insert(textures.end(), emissive.begin(), emissive.end());
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; j++)
			indices.push_back(mesh->mFaces[i].mIndices[j]);
	}

	return Mesh(core, vertices, indices, textures, this->ClearData);
}



Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene) {
	std::vector<vertex> vertices;
	std::vector<UINT> indices;
	std::vector<Texture> textures;

	for (UINT i = 0; i < mesh->mNumVertices; ++i) {
		vertex V;
		V.xyz[0] = mesh->mVertices[i].x;
		V.xyz[1] = mesh->mVertices[i].y;
		V.xyz[2] = mesh->mVertices[i].z;

		V.rgb[0] = 1.0f;
		V.rgb[1] = 1.0f;
		V.rgb[2] = 1.0f;

		if (mesh->HasNormals()) {
			V.Normal[0] = mesh->mNormals[i].x;
			V.Normal[1] = mesh->mNormals[i].y;
			V.Normal[2] = mesh->mNormals[i].z;
		}
		else {
			V.Normal[0] = 1.0f;
			V.Normal[1] = 1.0f;
			V.Normal[2] = 0.0f;
		}

		if (mesh->mTextureCoords[0]) {
			V.uv[0] = mesh->mTextureCoords[0][i].x;
			V.uv[1] = mesh->mTextureCoords[0][i].y;
		}

		if (mesh->HasTangentsAndBitangents()) {
			V.tangent[0] = mesh->mTangents[i].x;
			V.tangent[1] = mesh->mTangents[i].y;
			V.tangent[2] = mesh->mTangents[i].z;
		}
		else {
			V.tangent[0] = 0.0f;
			V.tangent[1] = 0.0f;
			V.tangent[2] = 0.0f;
		}

		V.Weights = { 0.0f,0.0f,0.0f,0.0f };
		V.BoneIds[0] = 0;
		V.BoneIds[1] = 0;
		V.BoneIds[2] = 0;
		V.BoneIds[3] = 0;

		vertices.push_back(V);
	}

	if (mesh->HasBones()) {
		for (int i = 0; i < mesh->mNumBones; i++) {
			aiBone* bone = mesh->mBones[i];
			std::string boneName = bone->mName.C_Str();
			uint32_t boneID = 0;

			if (BoneMap.find(boneName) == BoneMap.end()) {
				if (BoneData.size() < MAX_BONE_COUNT) {
					boneID = BoneData.size();
					BoneInfo newbone;
					newbone.ID = boneID;
					newbone.MatrixOffset = aiMatrixToXM(bone->mOffsetMatrix);
					BoneData.push_back(newbone);
					BoneMap[boneName] = boneID;
				}
				else {
					continue;
				}
			}
			else {
				boneID = BoneMap[boneName];
			}

			for (int j = 0; j < bone->mNumWeights; j++) {
				uint32_t vertexID = bone->mWeights[j].mVertexId;
				float weight = bone->mWeights[j].mWeight;
				SetVertexBoneData(vertices[vertexID], boneID, weight);
			}
		}
	}

	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];

		// color 
		std::vector<Texture> albedoMaps = this->LoadTexture(mat, aiTextureType_DIFFUSE, "albedo", scene);
		textures.insert(textures.end(), albedoMaps.begin(), albedoMaps.end());

		// metalic 
		std::vector<Texture> metallicMaps = this->LoadTexture(mat, aiTextureType_METALNESS, "metallic", scene);
		textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());

		// normals
		std::vector<Texture> normals = LoadTexture(mat, aiTextureType_NORMALS, "normal", scene);
		if (normals.empty()) normals = this->LoadTexture(mat, aiTextureType_HEIGHT, "normal", scene);
		if (normals.empty()) normals = this->LoadTexture(mat, aiTextureType_NORMAL_CAMERA, "normal", scene);
		textures.insert(textures.end(), normals.begin(), normals.end());

		// what can i say i like em rough baby 
		std::vector<Texture> roughnessMaps = LoadTexture(mat, aiTextureType_DIFFUSE_ROUGHNESS, "roughness", scene);
		if (roughnessMaps.empty()) roughnessMaps = this->LoadTexture(mat, aiTextureType_SHININESS, "roughness", scene);
		textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());

		// ambient occlusion ( i dont even use this shit)
		std::vector<Texture> aoMaps = LoadTexture(mat, aiTextureType_AMBIENT_OCCLUSION, "ao", scene);
		if (aoMaps.empty()) aoMaps = this->LoadTexture(mat, aiTextureType_LIGHTMAP, "ao", scene);
		textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());
		
		auto emissive = LoadTexture(mat, aiTextureType_EMISSIVE, "emissive", scene);
		textures.insert(textures.end(), emissive.begin(), emissive.end());
	}

	for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	return Mesh(core, vertices, indices, textures, this->ClearData);
}

void Model::Update(float dt) {
	const aiScene* scene = importer.GetScene();
	if (!scene || !scene->mRootNode) return;

	// advance animation time
	if (m_CurrentAnim != SIZE_MAX && m_CurrentAnim < m_Animations.size()) {
		const AnimationClip& clip = m_Animations[m_CurrentAnim];
		m_AnimTimeSec += dt * m_AnimSpeed;

		double ticksPerSec = clip.TicksPerSecond > 0.0 ? clip.TicksPerSecond : 1.0;
		double timeInTicks = m_AnimTimeSec * ticksPerSec;
		double duration = clip.DurationTicks > 0.0 ? clip.DurationTicks : 1.0;
		if (this->TimeStamped) {
			if (m_AnimLoop) {
				auto range = this->Anim_EndTick - this->Anim_StartTick;
				timeInTicks = this->Anim_StartTick + fmod(timeInTicks - this->Anim_StartTick, range);
			}
			else { // stop at the end tick
				if (timeInTicks >= this->Anim_EndTick) {
					timeInTicks = duration;
					this->StopAnimation();
				}
			}

		}
		else {
			if (m_AnimLoop) {
				timeInTicks = fmod(timeInTicks, duration);
			}
			else {
				if (timeInTicks > duration) timeInTicks = duration;
			}
		}

		m_AnimatedLocalTransforms.clear();
		for (const auto& ch : clip.Channels) {
			XMMATRIX nodeLocal = SampleNodeTransform(ch, timeInTicks);
			m_AnimatedLocalTransforms[ch.NodeName] = nodeLocal;
		}
	}

	UpdateHierarchy(scene->mRootNode, XMMatrixIdentity());
}

void Model::UpdateHierarchy(aiNode* node, XMMATRIX parentTransf) { // FUCK YOU FUCK YOU FUCK YOU 2 DAYS WASTED FUCKING WITH YOU
	if (!node) return;

	// get local transform (animated or default)
	XMMATRIX local;
	auto it = m_AnimatedLocalTransforms.find(node->mName.C_Str());
	if (it != m_AnimatedLocalTransforms.end()) {
		local = it->second;
	}
	else {
		local = aiMatrixToXM(node->mTransformation);
	}

	// reunion with the parent unlike my cousins. yeah my uncle's wife is a bitch 
	XMMATRIX global = XMMatrixMultiply(local, parentTransf);

	auto boneIt = BoneMap.find(node->mName.C_Str());
	if (boneIt != BoneMap.end()) {
		uint32_t boneID = boneIt->second;
		if (boneID < BoneData.size()) {

			XMMATRIX finalMat = BoneData[boneID].MatrixOffset * global * GlobalInverseTransf;

			BonePallete[boneID] = XMMatrixTranspose(finalMat);
		}
	}

	// process em kids
	for (UINT i = 0; i < node->mNumChildren; ++i) {
		UpdateHierarchy(node->mChildren[i], global);
	}
}
void Model::PlayAnimation(size_t index, bool loop) {
	if (index >= m_Animations.size()) return;
	if (m_CurrentAnim == index) {
		m_AnimLoop = loop;
		if (m_AnimLoop) IsAnimating = true;
	}
	m_CurrentAnim = index;
	m_AnimLoop = loop;
	m_AnimTimeSec = 0.0;
}


void Model::PlayAnimationTimeStamp(size_t index, float startTime, float endTime, bool loop) { // in seconds
	if (index >= m_Animations.size()) return;
	//if (m_endTime < startTime) std::cout << "Hey dummy PlayAnimationTimeStamp() has the end-time higher than the start-time.\n"; 
	m_CurrentAnim = index;
	m_AnimLoop = loop;
	this->TimeStamped = true;
	AnimationClip& clip = m_Animations[index];
	double TicksPerSec = clip.TicksPerSecond > 0.0 ? clip.TicksPerSecond : 24.0;

	// convert the seconds to ticks
	this->Anim_StartTick = startTime * (float)TicksPerSec;
	this->Anim_EndTick = endTime * (float)TicksPerSec;

	m_AnimTimeSec = startTime;
	IsAnimating = true;
}


void Model::StopAnimation() {
	m_CurrentAnim = SIZE_MAX;
	m_AnimTimeSec = 0.0;
	m_AnimatedLocalTransforms.clear();
	IsAnimating = false;
}

void Model::SetAnimationSpeed(float speed) {
	m_AnimSpeed = speed;
}


void Model::ClearVertexData() {
	if (this->mesh.empty()) return;
	for (auto& m : mesh) {
		if (!m.vertices.empty()) {
			m.vertices.clear();
			m.vertices.shrink_to_fit();
			m.indices.clear();
			m.indices.shrink_to_fit();
		}
	}
	this->ClearData = true;
}

const std::vector<vertex>& Model::GetVertexData() {
	if (!this->mesh.empty()) {
		return mesh[0].GetVertexData();
	}
	static const std::vector<vertex> emptyVD;
	return emptyVD;
}

XMMATRIX* Model::GetBonePallete() {
	return this->BonePallete;
}
