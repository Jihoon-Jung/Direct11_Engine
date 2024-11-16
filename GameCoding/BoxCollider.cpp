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

	// �߽��� ������Ʈ
	_boundingBox.Center = transform->GetWorldPosition();

	// ũ�� ������Ʈ (���� ������ * �ݶ��̴� ������)
	Vec3 scale = transform->GetWorldScale() * _scale;
	_boundingBox.Extents = Vec3(scale.x * 0.5f, scale.y * 0.5f, scale.z * 0.5f);  // �� �ະ�� ����

	// ȸ�� ������Ʈ (���ʹϾ� ���� ���)
	_boundingBox.Orientation = transform->GetQTRotation();  // GetQTRotation���� ����
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
