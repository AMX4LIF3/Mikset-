// TODO: maybe a step rate tho maybe after i do multiplayer
#pragma once
#include "../Entities/Entity/entity.hpp"
#include "../Entities/Player/player.hpp"
#include "collider.hpp"

void GetAABBLines(const Collider& box, XMFLOAT3* outPoints);

class CollisionHandler
{
	public:
		void ResolvePlayer(Player& player, std::vector<Entity*>& entities, float dt);
		CollisionInfo Info;
};

