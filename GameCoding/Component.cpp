#include "pch.h"
#include "GameObject.h"

Component::Component(ComponentType type)
	:_type(type)
{
}

Component::~Component()
{
}

shared_ptr<Transform> Component::GetTransform()
{
	if (_gameObject != nullptr)
	{
		shared_ptr<Transform> transform = _gameObject->GetComponent<Transform>();
		return transform;
	}
	return nullptr;
}
