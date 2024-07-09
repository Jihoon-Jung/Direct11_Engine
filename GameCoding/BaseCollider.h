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

	//											���̽��ۺ����� �Ÿ�?
	virtual bool Intersects(Ray& ray, OUT float& distance) = 0; // ray�� �浹�ߴ��� �Ǻ�
	virtual bool Intersects(shared_ptr<BaseCollider>& other) = 0; // �ٸ� collider�� �浹�ߴ��� �Ǻ�

	ColliderType GetColliderType() { return _colliderType; }

protected:
	ColliderType _colliderType;
};

