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
	/*_boundingSphere.Center = GetTransform()->GetWorldPosition();

	Vec3 scale = GetTransform()->GetWorldScale();
	_boundingSphere.Radius = _radius * max(max(scale.x, scale.y),scale.z);*/
	shared_ptr<Transform> transform = GetTransform();

	// 중심점 업데이트
	_boundingSphere.Center = transform->GetWorldPosition() + _center;

	// 크기 업데이트 (로컬 스케일 * 콜라이더 스케일)
	Vec3 scale = transform->GetWorldScale() * _scale;
	// 구체는 가장 큰 스케일 값을 반지름으로 사용
	_boundingSphere.Radius = max(max(scale.x, scale.y), scale.z) * 0.5f;
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
