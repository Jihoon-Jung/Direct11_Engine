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
	//void SetScale(float scale) { _scale = scale; }
	void SetScale(const Vec3& scale) { _scale = scale; }
	const Vec3& GetScale() const { return _scale; }

	void SetCenter(const Vec3& center) { _center = center; }
	Vec3 GetCenter() const { return _center; }

protected:
	ColliderType _colliderType;
	//float _scale = 1.0f;
	Vec3 _scale = Vec3(1.0f, 1.0f, 1.0f);
	Vec3 _center = Vec3::Zero;

};

