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
	/*_boundingBox.Center = GetTransform()->GetWorldPosition();
	Vec3 scale = GetTransform()->GetWorldScale() * _scale;
	Vec3 rotation = GetTransform()->GetWorldRotation();
	_boundingBox.Extents = Vec3(0.5f) * max(max(scale.x, scale.y), scale.z);
	_boundingBox.Orientation = Quaternion::CreateFromYawPitchRoll(rotation.y, rotation.x, rotation.z);*/

	shared_ptr<Transform> transform = GetTransform();

	// 중심점 업데이트
	_boundingBox.Center = transform->GetWorldPosition();

	// 크기 업데이트 (로컬 스케일 * 콜라이더 스케일)
	Vec3 scale = transform->GetWorldScale() * _scale;
	_boundingBox.Extents = Vec3(scale.x * 0.5f, scale.y * 0.5f, scale.z * 0.5f);  // 각 축별로 적용

	// 회전 업데이트 (쿼터니언 직접 사용)
	_boundingBox.Orientation = transform->GetQTRotation();  // GetQTRotation으로 변경
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
