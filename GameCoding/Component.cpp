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
    if (auto gameObject = _gameObject.lock())
    {
        return gameObject->GetComponent<Transform>();
    }
    return nullptr;
}
