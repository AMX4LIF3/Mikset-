#define NOMINMAX
#include "CollisionHandler.hpp"

void GetAABBLines(const Collider& box, XMFLOAT3* outPoints) {
	XMFLOAT3 min = box.Min;
	XMFLOAT3 max = box.Max;

	// the 8 corner vertices
	XMFLOAT3 c[8] = {
		min,                                
		{ max.x, min.y, min.z },            
		{ max.x, max.y, min.z },            
		{ min.x, max.y, min.z },            
		{ min.x, min.y, max.z },            
		{ max.x, min.y, max.z },            
		max,                                
		{ min.x, max.y, max.z }             
	};

	// the 12 lines (24 points)
	XMFLOAT3 lines[24] = {
		c[0], c[1], c[1], c[2], c[2], c[3], c[3], c[0], // bottom square
		c[4], c[5], c[5], c[6], c[6], c[7], c[7], c[4], // top square
		c[0], c[4], c[1], c[5], c[2], c[6], c[3], c[7]  // vertical connectors
	};

	memcpy(outPoints, lines, sizeof(XMFLOAT3) * 24);
}

static XMVECTOR ClosestPointOnSegment(XMVECTOR A, XMVECTOR B, XMVECTOR P) {
	XMVECTOR AB = B - A;
	float t = XMVectorGetX(XMVector3Dot(P - A, AB)) / XMVectorGetX(XMVector3Dot(AB, AB));
	t = std::clamp(t, 0.0f, 1.0f);
	return A + AB * t;
}

static XMVECTOR FindClosestPointOnTriangle(XMVECTOR V0, XMVECTOR V1, XMVECTOR V2, XMVECTOR P) {

	// so the code here i found on an old ass website so.. barycentric coordinates.. enjoy !
	// we're using barycentric coords here.. my guess is as good as yours.
	XMVECTOR N = XMVector3Normalize(XMVector3Cross(V1 - V0, V2 - V0));
	float distToPlane = XMVectorGetX(XMVector3Dot(P - V0, N));
	XMVECTOR P_proj = P - N * distToPlane;

	// check if projection is inside triangle
	XMVECTOR v0 = V1 - V0, v1 = V2 - V0, v2 = P_proj - V0;
	float dot00 = XMVectorGetX(XMVector3Dot(v0, v0));
	float dot01 = XMVectorGetX(XMVector3Dot(v0, v1));
	float dot11 = XMVectorGetX(XMVector3Dot(v1, v1));
	float dot20 = XMVectorGetX(XMVector3Dot(v2, v0));
	float dot21 = XMVectorGetX(XMVector3Dot(v2, v1));
	float denom = dot00 * dot11 - dot01 * dot01;

	if (fabs(denom) < 1e-6f) return V0; // degenerate triangle

	float v = (dot11 * dot20 - dot01 * dot21) / denom;
	float w = (dot00 * dot21 - dot01 * dot20) / denom;
	float u = 1.0f - v - w;

	if (u >= -1e-6f && v >= -1e-6f && w >= -1e-6f && u + v + w <= 1.0f + 1e-6f) { // it IS inside
		return P_proj;
	}

	// outside - find closest point on edges
	XMVECTOR edgePoints[3] = {
		ClosestPointOnSegment(V0, V1, P_proj),
		ClosestPointOnSegment(V1, V2, P_proj),
		ClosestPointOnSegment(V2, V0, P_proj)
	};

	float distSq[3] = {
		XMVectorGetX(XMVector3Dot(edgePoints[0] - P_proj, edgePoints[0] - P_proj)),
		XMVectorGetX(XMVector3Dot(edgePoints[1] - P_proj, edgePoints[1] - P_proj)),
		XMVectorGetX(XMVector3Dot(edgePoints[2] - P_proj, edgePoints[2] - P_proj))
	};

	int best = 0;
	if (distSq[1] < distSq[0]) best = 1;
	if (distSq[2] < distSq[best]) best = 2;

	return edgePoints[best];
}



static CollisionInfo GetCapsuleTriangleDist(
	XMVECTOR vB, XMVECTOR vT, float radius,
	XMVECTOR V0, XMVECTOR V1, XMVECTOR V2) {

	CollisionInfo info;

	// find closest point on triangle to capsule segment
	XMVECTOR closestTriPoint = FindClosestPointOnTriangle(V0, V1, V2, (vB + vT) * 0.5f);

	// find closest point on capsule segment to that point
	XMVECTOR closestCapPoint = ClosestPointOnSegment(vB, vT, closestTriPoint);

	XMVECTOR delta = closestCapPoint - closestTriPoint;
	float distSq = XMVectorGetX(XMVector3Dot(delta, delta));
	float radiusSq = radius * radius;

	if (distSq < radiusSq) {
		info.Hit = true;
		float dist = sqrtf(distSq);
		info.Normal = dist > 0.0001f ? delta / dist : XMVectorSet(0, 1, 0, 0);
		info.Depth = radius - dist;
		info.point = closestTriPoint;
	}

	return info;
}


static void TraverseBVHCollect(Mesh& mesh, unsigned int NodeIndex,
    XMVECTOR vB, XMVECTOR vT, float radius,
    XMMATRIX WorldMat, CollisionResult& result)
{
    BVHNode& Node = mesh.BVHNodes[NodeIndex];

    XMVECTOR boxMin = XMLoadFloat3(&Node.Min);
    XMVECTOR boxMax = XMLoadFloat3(&Node.Max);

    XMVECTOR closestOnCapsule = ClosestPointOnSegment(vB, vT, (boxMin + boxMax) * 0.5f);
    XMVECTOR closestOnBox = XMVectorMax(boxMin, XMVectorMin(closestOnCapsule, boxMax));
    XMVECTOR delta = closestOnCapsule - closestOnBox;
    float distSq = XMVectorGetX(XMVector3Dot(delta, delta));

    if (distSq > radius * radius) return;

    if (Node.count > 0) {
        for (int i = 0; i < Node.count; i++) {
            uint32_t i0 = mesh.indices[(Node.index + i) * 3 + 0];
            uint32_t i1 = mesh.indices[(Node.index + i) * 3 + 1];
            uint32_t i2 = mesh.indices[(Node.index + i) * 3 + 2];

            XMVECTOR V0 = XMLoadFloat3((XMFLOAT3*)&mesh.vertices[i0].xyz);
            XMVECTOR V1 = XMLoadFloat3((XMFLOAT3*)&mesh.vertices[i1].xyz);
            XMVECTOR V2 = XMLoadFloat3((XMFLOAT3*)&mesh.vertices[i2].xyz);

            CollisionInfo triCollision = GetCapsuleTriangleDist(vB, vT, radius, V0, V1, V2);

            if (triCollision.Hit) {
                XMVECTOR worldNormal = XMVector3Normalize(XMVector3TransformNormal(triCollision.Normal, WorldMat));
                float normalY = XMVectorGetY(worldNormal);

                // sphere fix: allow any upward facing surface (normalY > 0) as ground if steep enough
                // but we separate walkable vs slideable
                // update: its still rough even after all of this
                
                if (normalY > 0.7f) {
                    if (triCollision.Depth > result.groundDepth) {
                        result.groundDepth = triCollision.Depth;
                        result.groundNormal = triCollision.Normal; // local space
                        result.groundWorldNormal = worldNormal;    // world space for checks
                        result.hitGround = true;
                    }
                }
                else if (normalY > 0.0f) { // any upward normal is ground, just steep
                    if (triCollision.Depth > result.wallDepth) {
                        result.wallDepth = triCollision.Depth;
                        result.wallNormal = triCollision.Normal;
                        result.wallWorldNormal = worldNormal;
                        result.hitWall = true;
                    }
                }
                else {
                    // ceiling 
                    if (triCollision.Depth > result.wallDepth) {
                        result.wallDepth = triCollision.Depth;
                        result.wallNormal = triCollision.Normal;
                        result.wallWorldNormal = worldNormal;
                        result.hitWall = true;
                    }
                }
            }
        }
        return;
    }

    if (Node.left  != -1)  TraverseBVHCollect(mesh, Node.left,  vB, vT, radius, WorldMat, result);
    if (Node.right != -1)  TraverseBVHCollect(mesh, Node.right, vB, vT, radius, WorldMat, result);
}

#define GRAVITY_STR 30.0f
void CollisionHandler::ResolvePlayer(Player& player, std::vector<Entity*>& entities, float dt) {
    if (entities.empty()) return;

    const int MAX_ITERATIONS = 5;
    const float GROUND_ANGLE = 0.7f;

    XMVECTOR originalPos = XMLoadFloat3(&player.transf.position);
    XMVECTOR totalOffset = XMVectorZero();

    bool wasGrounded = player.grounded;
    player.grounded = false;
    XMVECTOR groundNormal = XMVectorSet(0, 1, 0, 0);
    float maxGroundDepth = 0.0f;

    for (int iter = 0; iter < MAX_ITERATIONS; iter++) {
        CollisionResult result;
        XMVECTOR currentPos = originalPos + totalOffset;

        bool anyHit = false;

        // check ALL entities for collision
        for (auto* entity : entities) {
            if (!entity) continue;
            if (entity->colliders.empty()) continue;

            XMMATRIX WorldMat = entity->GetWorldMat();
            XMMATRIX invWorld = XMMatrixInverse(nullptr, WorldMat);
            XMVECTOR invScaleX = XMVector3Length(invWorld.r[0]);
            XMVECTOR invScaleY = XMVector3Length(invWorld.r[1]);
            XMVECTOR invScaleZ = XMVector3Length(invWorld.r[2]);
            float uniformScale = (XMVectorGetX(invScaleX) + XMVectorGetX(invScaleY) + XMVectorGetX(invScaleZ));

            Capsule cap = player.GetCollider();
            float localRadius = cap.radius / uniformScale;

            XMVECTOR worldBase = XMLoadFloat3(&cap.base) + totalOffset;
            XMVECTOR worldTip = XMLoadFloat3(&cap.tip) + totalOffset;

            XMVECTOR localBase = XMVector3TransformCoord(worldBase, invWorld);
            XMVECTOR localTip = XMVector3TransformCoord(worldTip, invWorld);

            for (const auto& collider : entity->colliders) {
                if (collider.Type == CollisionType::Box) {
                    XMVECTOR boxMin = XMLoadFloat3(&collider.Min);
                    XMVECTOR boxMax = XMLoadFloat3(&collider.Max);

                    XMVECTOR closestOnCap = ClosestPointOnSegment(localBase, localTip, (boxMin + boxMax) * 0.5f);
                    XMVECTOR closestOnBox = XMVectorMax(boxMin, XMVectorMin(closestOnCap, boxMax));
                    XMVECTOR delta = closestOnCap - closestOnBox;
                    float distSq = XMVectorGetX(XMVector3Dot(delta, delta));

                    if (distSq < localRadius * localRadius) {
                        float dist = sqrtf(distSq);
                        XMVECTOR normal = dist > 0.0001f ? delta / dist : XMVectorSet(0, 1, 0, 0);
                        float depth = localRadius - dist;

                        XMVECTOR worldNormal = XMVector3Normalize(XMVector3TransformNormal(normal, WorldMat));
                        float normalY = XMVectorGetY(worldNormal);

                        anyHit = true;

                        if (normalY > GROUND_ANGLE) {
                            if (depth > result.groundDepth) {
                                result.groundDepth = depth;
                                result.groundNormal = normal;
                                result.groundWorldNormal = worldNormal;
                                result.hitGround = true;
                            }
                        }
                        else {
                            if (depth > result.wallDepth) {
                                result.wallDepth = depth;
                                result.wallNormal = normal;
                                result.wallWorldNormal = worldNormal;
                                result.hitWall = true;
                            }
                        }
                    }
                }
                else if (collider.Type == CollisionType::TriangleMesh) {
                    if (!entity->GetModel()) continue;

                    for (auto& Mesh : entity->GetModel()->mesh) {
                        if (Mesh.BVHNodes.empty()) continue;

                        TraverseBVHCollect(Mesh, 0, localBase, localTip, localRadius, WorldMat, result);
                        if (result.hitGround || result.hitWall) anyHit = true;
                    }
                }
            }
        }

        if (!anyHit) break;

        // apply deepest ground push
        if (result.hitGround && result.groundDepth > 0.001f) {
            totalOffset = totalOffset + result.groundWorldNormal * result.groundDepth;
            groundNormal = result.groundWorldNormal;
            player.grounded = true;
            maxGroundDepth = std::max(maxGroundDepth, result.groundDepth);
        }

        // apply deepest wall push
        if (result.hitWall && result.wallDepth > 0.001f) {
            // dont push if wall is actually steep ground and ground was alr handled
            if (XMVectorGetY(result.wallWorldNormal) <= GROUND_ANGLE || !result.hitGround) {
                totalOffset = totalOffset + result.wallWorldNormal * result.wallDepth;
            }
        }
    }

    // final pos
    XMStoreFloat3(&player.transf.position, originalPos + totalOffset);

    // velocity update once per frame
    XMVECTOR velocity = XMLoadFloat3(&player.velocity);

    if (player.grounded) {
        // remove velocity into ground
        float velIntoGround = XMVectorGetX(XMVector3Dot(velocity, groundNormal));
        if (velIntoGround < 0) {
            velocity = velocity - groundNormal * velIntoGround;
        }

        // zero Y for walking on slopes so that it prevents sliding
        // but preserve jump velocity if had just jumped (positive Y)
        if (XMVectorGetY(velocity) < 0) {
            velocity = XMVectorSetY(velocity, 0);
        }

        // friction
        float speed = XMVectorGetX(XMVector3Length(velocity));
        if (speed > 0) {
            float drop = speed * 10.0f * dt; // ground friction
            float newspeed = speed - drop;
            if (newspeed < 0) newspeed = 0;
            velocity = velocity * (newspeed / speed);
        }
    }
    else {
        // air gravity.0 
        velocity = XMVectorSetY(velocity, XMVectorGetY(velocity) - GRAVITY_STR * dt);

        // small air friction cuz why not
        velocity = velocity * 0.98f;
    }

    XMStoreFloat3(&player.velocity, velocity);

    // apply movement. now yes might be annoying to not be able to move if theres no collision but 
    // ill def handle that later...sigh TODO
    XMVECTOR newPos = XMLoadFloat3(&player.transf.position) + velocity * dt;
    XMStoreFloat3(&player.transf.position, newPos);

    player.Update(0);
}
