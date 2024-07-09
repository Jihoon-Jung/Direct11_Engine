#include "pch.h"
#include "SphereCollider.h"

SphereCollider::SphereCollider()
	:BaseCollider(ColliderType::Sphere)
{
}

SphereCollider::~SphereCollider()
{
}

void SphereCollider::Update()
{
	_boundingSphere.Center = GetTransform()->GetWorldPosition();

	Vec3 scale = GetTransform()->GetWorldScale();
	_boundingSphere.Radius = _radius * max(max(scale.x, scale.y),scale.z);
}

bool SphereCollider::Intersects(Ray& ray, OUT float& distance)
{
	return _boundingSphere.Intersects(ray.position, ray.direction, OUT distance);
}

bool SphereCollider::Intersects(shared_ptr<BaseCollider>& other)
{
	ColliderType type = other->GetColliderType();

	switch (type)
	{
	case ColliderType::Sphere:
		return _boundingSphere.Intersects(dynamic_pointer_cast<SphereCollider>(other)->GetBoundingSphere());
	case ColliderType::Box:
		return _boundingSphere.Intersects(dynamic_pointer_cast<BoxCollider>(other)->GetBoundingBox());
	}

	return false;
}
