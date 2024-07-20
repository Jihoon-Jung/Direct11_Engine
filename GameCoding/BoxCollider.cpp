#include "pch.h"
#include "BoxCollider.h"

BoxCollider::BoxCollider() 
	: BaseCollider(ColliderType::Box)
{
}

BoxCollider::~BoxCollider()
{
}

void BoxCollider::Update()
{
	_boundingBox.Center = GetTransform()->GetWorldPosition();
	Vec3 scale = GetTransform()->GetWorldScale() * _scale;
	Vec3 rotation = GetTransform()->GetWorldRotation();
	_boundingBox.Extents = Vec3(0.5f) * max(max(scale.x, scale.y), scale.z);
	_boundingBox.Orientation = Quaternion::CreateFromYawPitchRoll(rotation.y, rotation.x, rotation.z);
}

bool BoxCollider::Intersects(Ray& ray, OUT float& distance)
{
	return _boundingBox.Intersects(ray.position, ray.direction, OUT distance);
}

bool BoxCollider::Intersects(shared_ptr<BaseCollider>& other)
{
	ColliderType type = other->GetColliderType();

	switch (type)
	{
	case ColliderType::Sphere:
		return _boundingBox.Intersects(dynamic_pointer_cast<SphereCollider>(other)->GetBoundingSphere());
	case ColliderType::Box:
		return _boundingBox.Intersects(dynamic_pointer_cast<BoxCollider>(other)->GetBoundingBox());
	}

	return false;
}
