#include "ColliderDebug.hpp"
#include <cstring>
#include <algorithm>

// TODO: AI is FUCKING STUPID, I WROTE THE WHOLE FUCKING MATH FOR THE GODDAMN COLLISION
// AND NOW AI CANT EVEN SET UP A BASIC BUFFER WITH A MINIMAL SHADER FOR GOD'S SAKE 
void ColliderDebug::Init(Core& core) {
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };


    shader.VSCompileFromFile(core, core.m_device.Get(), "Renderer/Graphics/shaders/Colliderview.hlsl", "ColliderVS");

    core.m_device->CreateInputLayout(layout, _countof(layout),
        shader.GetBlob()->GetBufferPointer(),
        shader.GetBlob()->GetBufferSize(),
        &m_IL);

    shader.PSCompileFromFile(core, core.m_device.Get(), "Renderer/Graphics/shaders/Colliderview.hlsl", "ColliderPS");

    D3D11_BUFFER_DESC vbd = {};
    vbd.Usage = D3D11_USAGE_DYNAMIC;
    vbd.ByteWidth = sizeof(DebugVertex) * this->kMaxDebugVerts;
    vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    core.m_device->CreateBuffer(&vbd, nullptr, &m_VB);
    m_DebugVerts.reserve(kMaxDebugVerts);

    D3D11_BUFFER_DESC cbd = {};
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.ByteWidth = sizeof(DebugCB);
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    core.m_device->CreateBuffer(&cbd, nullptr, &m_CB);
}

void ColliderDebug::FlushDebugVerts(ID3D11DeviceContext* ctx)
{
    if (m_DebugVerts.empty() || !m_VB)
        return;

    D3D11_MAPPED_SUBRESOURCE mapped;

    if (SUCCEEDED(ctx->Map(
        m_VB.Get(),
        0,
        D3D11_MAP_WRITE_DISCARD,
        0,
        &mapped)))
    {
        memcpy(
            mapped.pData,
            m_DebugVerts.data(),
            sizeof(DebugVertex) * m_DebugVerts.size());

        ctx->Unmap(m_VB.Get(), 0);

        ctx->Draw((UINT)m_DebugVerts.size(), 0);
    }

    m_DebugVerts.clear();
}

void ColliderDebug::AddBVHLines(const Mesh& mesh, std::vector<DebugVertex>& out, XMFLOAT4 color) {
    if (mesh.BVHNodes.empty())
        return;

    size_t nodeCount = mesh.BVHNodes.size();

    size_t step = 1;

    if (nodeCount > kBVHNodeBudget)
        step = nodeCount / kBVHNodeBudget;

    for (size_t n = 0; n < nodeCount; n += step)
    {
        const auto& node = mesh.BVHNodes[n];

        XMFLOAT3 min = node.Min;
        XMFLOAT3 max = node.Max;

        XMFLOAT3 c[8] =
        {
            min,
            { max.x,min.y,min.z },
            { max.x,max.y,min.z },
            { min.x,max.y,min.z },

            { min.x,min.y,max.z },
            { max.x,min.y,max.z },
            max,
            { min.x,max.y,max.z }
        };

        int indices[] =
        {
            0,1, 1,2, 2,3, 3,0,
            4,5, 5,6, 6,7, 7,4,
            0,4, 1,5, 2,6, 3,7
        };

        for (int i : indices)
        {
            if (out.size() >= kMaxDebugVerts)
                break;

            out.push_back({ c[i], color });
        }

        if (out.size() >= kMaxDebugVerts)
            break;
    }
}

void ColliderDebug::Draw(ID3D11DeviceContext* ctx, const XMMATRIX& viewProj,
    const std::vector<std::unique_ptr<Entity>>& entities,
    Player* playerPtr, Shader& prevShader,
    ID3D11InputLayout* prevIL, ID3D11Buffer* globalCB)
{
    if (!m_IL) return;

    // set Shaders
    ctx->VSSetShader(shader.GetVS().Get(), nullptr, 0);
    ctx->PSSetShader(shader.GetPS().Get(), nullptr, 0);

    ctx->IASetInputLayout(m_IL.Get());
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

    UINT stride = sizeof(DebugVertex), offset = 0;
    if (m_VB) ctx->IASetVertexBuffers(0, 1, m_VB.GetAddressOf(), &stride, &offset);


    for (const auto& up : entities) {
        if (!up || up->colliders.empty()) continue;

        DebugCB cbData;
        cbData.worldViewProj = XMMatrixTranspose(up->GetWorldMat() * viewProj);
        if (m_CB) ctx->UpdateSubresource(m_CB.Get(), 0, nullptr, &cbData, 0, 0);
        if (m_CB) ctx->VSSetConstantBuffers(0, 1, m_CB.GetAddressOf());

        m_DebugVerts.clear();
        bool wantsMeshWire = false;
        for (const auto& box : up->colliders) {
            if (box.Type == CollisionType::Box) AddAABBLines(box, m_DebugVerts, { 0.0f, 0.0f, 1.0f, 1.0f });
            if (box.Type == CollisionType::TriangleMesh) wantsMeshWire = true;
        }

        // If a TriangleMesh collider is present, add mesh wireframes and BVH boxes
        if (wantsMeshWire && up->GetModel()) {
            Model* model = up->GetModel();
            for (const auto& mesh : model->mesh) {
                AddMeshWireframe(mesh, m_DebugVerts, { 1.0f,0.0f,0.0f,1.0f }); // red triangles
                AddBVHLines(mesh, m_DebugVerts, { 0.0f,1.0f,0.0f,0.6f }); // green BVH nodes (semi-transparent)
            }
        }

        FlushDebugVerts(ctx);
    }

    if (playerPtr) {
        DebugCB cbData;

        cbData.worldViewProj = XMMatrixTranspose(XMMatrixIdentity() * viewProj);
        if (m_CB) ctx->UpdateSubresource(m_CB.Get(), 0, nullptr, &cbData, 0, 0);
        if (m_CB) ctx->VSSetConstantBuffers(0, 1, m_CB.GetAddressOf());

        m_DebugVerts.clear();

        AddCapsuleLines(playerPtr->GetCollider(), m_DebugVerts, { 1.0f,1.0f,0.0f,1.0f });

        FlushDebugVerts(ctx);
    }
    // set the previously used shaders back
    // yes this might be annoying if you're swtiching alot of shaders but hey 
    // this works so stfu
    ctx->VSSetShader(prevShader.GetVS().Get(), nullptr, 0);
    ctx->PSSetShader(prevShader.GetPS().Get(), nullptr, 0);
    ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ctx->IASetInputLayout(prevIL);
    ctx->VSSetConstantBuffers(0, 1, &globalCB);
    ctx->PSSetConstantBuffers(0, 1, &globalCB);
}


void ColliderDebug::AddAABBLines(const Collider& box, std::vector<DebugVertex>& out, XMFLOAT4 color) {
    XMFLOAT3 min = box.Min;
    XMFLOAT3 max = box.Max;
    XMFLOAT3 c[8] = {
        min, {max.x, min.y, min.z}, {max.x, max.y, min.z}, {min.x, max.y, min.z},
        {min.x, min.y, max.z}, {max.x, min.y, max.z}, max, {min.x, max.y, max.z}
    };
    int indices[] = { 0,1, 1,2, 2,3, 3,0, 4,5, 5,6, 6,7, 7,4, 0,4, 1,5, 2,6, 3,7 };
    for (int i : indices) out.push_back({ c[i], color });
}

void ColliderDebug::AddCapsuleLines(const Capsule& cap, std::vector<DebugVertex>& out, XMFLOAT4 color) {
    float r = cap.radius;
    int segments = 16;

    for (int i = 0; i < 4; i++) {
        float angle = i * (XM_PIDIV2);
        float x = cosf(angle) * r;
        float z = sinf(angle) * r;
        out.push_back({ {cap.base.x + x, cap.base.y, cap.base.z + z}, color });
        out.push_back({ {cap.tip.x + x, cap.tip.y, cap.tip.z + z}, color });
    }


    for (int i = 0; i < segments; i++) {
        float a1 = (float)i / segments * XM_2PI;
        float a2 = (float)(i + 1) / segments * XM_2PI;

        out.push_back({ {cap.base.x + cosf(a1) * r, cap.base.y, cap.base.z + sinf(a1) * r}, color });
        out.push_back({ {cap.base.x + cosf(a2) * r, cap.base.y, cap.base.z + sinf(a2) * r}, color });

        out.push_back({ {cap.tip.x + cosf(a1) * r, cap.tip.y, cap.tip.z + sinf(a1) * r}, color });
        out.push_back({ {cap.tip.x + cosf(a2) * r, cap.tip.y, cap.tip.z + sinf(a2) * r}, color });
    }

/*
    for (int i = 0; i < segments / 2; i++) {
        float a1 = (float)i / (segments / 2) * XM_PIDIV2;
        float a2 = (float)(i + 1) / (segments / 2) * XM_PIDIV2;

        out.push_back({ {cap.tip.x + cosf(a1) * r, cap.tip.y + sinf(a1) * r, cap.tip.z}, color });
        out.push_back({ {cap.tip.x + cosf(a2) * r, cap.tip.y + sinf(a2) * r, cap.tip.z}, color });

        out.push_back({ {cap.base.x + cosf(a1) * r, cap.base.y - sinf(a1) * r, cap.base.z}, color });
        out.push_back({ {cap.base.x + cosf(a2) * r, cap.base.y - sinf(a2) * r, cap.base.z}, color });

        out.push_back({ {cap.tip.x, cap.tip.y + sinf(a1) * r, cap.tip.z + cosf(a1) * r}, color });
        out.push_back({ {cap.tip.x, cap.tip.y + sinf(a2) * r, cap.tip.z + cosf(a2) * r}, color });

        out.push_back({ {cap.base.x, cap.base.y - sinf(a1) * r, cap.base.z + cosf(a1) * r}, color });
        out.push_back({ {cap.base.x, cap.base.y - sinf(a2) * r, cap.base.z + cosf(a2) * r}, color });
    }
*/
    // bloh het
    /*
    XMFLOAT4 hatColor = { 0.0f, 0.5f, 1.0f, 1.0f };
    float hHeight = r * 0.5f;

    segments = 16;
    for (int i = 0; i < segments; i++) {
        float a1 = (float)i / segments * XM_2PI;
        float a2 = (float)(i + 1) / segments * XM_2PI;

        out.push_back({ {cap.tip.x + cosf(a1) * r, cap.tip.y, cap.tip.z + sinf(a1) * r}, hatColor });
        out.push_back({ {cap.tip.x + cosf(a2) * r, cap.tip.y, cap.tip.z + sinf(a2) * r}, hatColor });
    }

    XMFLOAT3 peak = { cap.tip.x, cap.tip.y + hHeight, cap.tip.z };
    for (int i = 0; i < 4; i++) {
        float angle = i * XM_PIDIV2;
        float x = cosf(angle) * r;
        float z = sinf(angle) * r;

        out.push_back({ {cap.tip.x + x, cap.tip.y, cap.tip.z + z}, hatColor });
        out.push_back({ peak, hatColor });
    }
*/
}

void ColliderDebug::AddMeshWireframe(const Mesh& mesh, std::vector<DebugVertex>& out, XMFLOAT4 color) {

    if (mesh.vertices.empty() || mesh.indices.empty())
        return;

    size_t triCount = mesh.indices.size() / 3;

    size_t step = 1;

    if (triCount > kTriangleBudget)
        step = triCount / kTriangleBudget;

    for (size_t tri = 0; tri < triCount; tri += step)
    {
        if (out.size() + 6 >= kMaxDebugVerts)
            break;

        size_t i = tri * 3;

        uint32_t i0 = mesh.indices[i + 0];
        uint32_t i1 = mesh.indices[i + 1];
        uint32_t i2 = mesh.indices[i + 2];

        XMFLOAT3 v0 =
        {
            mesh.vertices[i0].xyz[0],
            mesh.vertices[i0].xyz[1],
            mesh.vertices[i0].xyz[2]
        };

        XMFLOAT3 v1 =
        {
            mesh.vertices[i1].xyz[0],
            mesh.vertices[i1].xyz[1],
            mesh.vertices[i1].xyz[2]
        };

        XMFLOAT3 v2 =
        {
            mesh.vertices[i2].xyz[0],
            mesh.vertices[i2].xyz[1],
            mesh.vertices[i2].xyz[2]
        };

        out.push_back({ v0,color });
        out.push_back({ v1,color });

        out.push_back({ v1,color });
        out.push_back({ v2,color });

        out.push_back({ v2,color });
        out.push_back({ v0,color });
    }
}