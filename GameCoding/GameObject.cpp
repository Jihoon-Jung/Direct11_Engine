#include "pch.h"
#include "GameObject.h"
#include "tinyxml2.h"

GameObject::GameObject()
{
	_components.resize(FIXED_COMPONENT_COUNT);
}

GameObject::~GameObject()
{
}

void GameObject::Start()
{
	for (shared_ptr<Component>& component : _components)
	{
		if (component)
			component->Start();
	}
}

void GameObject::Update()
{
	for (shared_ptr<Component>& component : _components)
	{
		if (component)
			component->Update();
	}
}

void GameObject::LateUpdate()
{
	for (shared_ptr<Component>& component : _components)
	{
		if (component)
			component->LateUpdate();
	}
}

void GameObject::AddComponent(shared_ptr<Component> component)
{
	component->SetGameObject(shared_from_this());
	int index = static_cast<uint8>(component->GetType());
	if (index < FIXED_COMPONENT_COUNT)
	{
		_components[index] = component;
	}
	else
	{
		_components.push_back(component);
	}
}

void GameObject::RemoveComponent(shared_ptr<Component> component)
{
	if (!component)
		return;

	ComponentType type = component->GetType();
	uint8 index = static_cast<uint8>(type);

	// Transform ������Ʈ�� ������ �� ����
	if (type == ComponentType::Transform)
		return;

	// FIXED_COMPONENT_COUNT �̳��� ������Ʈ�� ���
	if (index < FIXED_COMPONENT_COUNT)
	{
		if (_components[index] == component)
		{
			_components[index] = nullptr;
		}
	}
	else
	{
		// FIXED_COMPONENT_COUNT ���Ŀ� �߰��� ������Ʈ�� ���
		auto it = std::find(_components.begin() + FIXED_COMPONENT_COUNT, _components.end(), component);
		if (it != _components.end())
		{
			_components.erase(it);
		}
	}

	// ������Ʈ�� GameObject ���� ����
	component->SetGameObject(nullptr);
}

void GameObject::SetParent(shared_ptr<GameObject> parent)
{
	_parent = parent;
	shared_from_this()->transform()->SetParent(_parent->transform());
	_parent->AddChild(shared_from_this());
	_parent->transform()->AddChild(shared_from_this()->transform());
}

void GameObject::AddChild(shared_ptr<GameObject> child)
{
	_children.push_back(child);

}


shared_ptr<Transform> GameObject::transform()
{
	int index = static_cast<uint8>(ComponentType::Transform);
	if (_components[index] != nullptr)
	{
		return static_pointer_cast<Transform>(_components[index]);
	}
	else
	{
		shared_ptr<Transform> transform = make_shared<Transform>();
		AddComponent(transform);
	}

	return transform();
}



