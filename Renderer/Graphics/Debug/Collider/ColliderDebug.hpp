#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include <memory>
#include <wrl/client.h>
#include "Renderer/Graphics/shaders.hpp"
#include "../../Entities/Entity/entity.hpp" 
#include "../../Entities/Player/player.hpp" 

using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct DebugVertex {
    XMFLOAT3 pos;
    XMFLOAT4 color;
};


struct DebugCB {
    XMMATRIX worldViewProj;
};

class ColliderDebug {
public:
    void Init(Core& core);
    void Draw(ID3D11DeviceContext* ctx, const XMMATRIX& viewProj,
              const std::vector<std::unique_ptr<Entity>>& entities,
			  Player* playerPtr, Shader& prevShader,
			  ID3D11InputLayout* prevIL, ID3D11Buffer* globalCB);

private:
    std::vector<DebugVertex> m_DebugVerts;


    // maximum number of mesh triangles visualized.
    // small meshes draw completely.
    // large meshes are sampled uniformly.
    static constexpr size_t kMaxDebugVerts = 500000;
    static constexpr size_t kTriangleBudget = 10000;
    static constexpr size_t kBVHNodeBudget = 2000;

    Shader shader; // Private member as requested
    ComPtr<ID3D11InputLayout> m_IL;
    ComPtr<ID3D11Buffer> m_VB;
    ComPtr<ID3D11Buffer> m_CB;
    void FlushDebugVerts(ID3D11DeviceContext* ctx);
    void AddBVHLines(const Mesh& mesh, std::vector<DebugVertex>& out, XMFLOAT4 color);
    void AddAABBLines(const Collider& box, std::vector<DebugVertex>& out, XMFLOAT4 color);
    void AddCapsuleLines(const Capsule& cap, std::vector<DebugVertex>& out, XMFLOAT4 color);
    void AddMeshWireframe(const Mesh& mesh, std::vector<DebugVertex>& out, XMFLOAT4 color);
};