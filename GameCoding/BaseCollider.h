#pragma once
#include "Component.h"

enum class ColliderType
{
	Sphere,
	Box,
};

class BaseCollider : public Component
{
public:
	BaseCollider(ColliderType colliderType);
	virtual ~BaseCollider();

	//											레이시작부터의 거리?
	virtual bool Intersects(Ray& ray, OUT float& distance) = 0; // ray와 충돌했는지 판별
	virtual bool Intersects(shared_ptr<BaseCollider>& other) = 0; // 다른 collider와 충돌했는지 판별

	ColliderType GetColliderType() { return _colliderType; }

protected:
	ColliderType _colliderType;
};

