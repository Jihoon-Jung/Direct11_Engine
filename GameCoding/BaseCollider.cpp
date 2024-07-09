#include "pch.h"
#include "BaseCollider.h"

BaseCollider::BaseCollider(ColliderType colliderType)
	: Component(ComponentType::Collider), _colliderType(colliderType)
{

}

BaseCollider::~BaseCollider()
{

}