#pragma once
#include "Component.h"
#include "Buffer.h"
#include "Camera.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "MonoBehaviour.h"
#include "Light.h"
class GameObject : public enable_shared_from_this<GameObject>
{
public:
	GameObject();
	~GameObject();

	void Start();
	void Update();
	void LateUpdate();

	void AddComponent(shared_ptr<Component> component);
	void SetParent(shared_ptr<GameObject> parent);
	void AddChild(shared_ptr<GameObject> child);
	template <typename T>
	shared_ptr<T> GetComponent()
	{
		ComponentType type = ComponentType::Script;
		if (std::is_same_v<T, Transform>)
			type = ComponentType::Transform;
		if (std::is_same_v<T, MeshRenderer>)
			type = ComponentType::MeshRenderer;
		if (std::is_same_v<T, Camera>)
			type = ComponentType::Camera;
		if (std::is_same_v<T, Light>)
			type = ComponentType::Light;
		if (std::is_same_v<T, MonoBehaviour>)
			type = ComponentType::Script;

		uint8 index = static_cast<uint8>(type);
		shared_ptr<Component> component = _components[index];
		shared_ptr<T> castedComponent = static_pointer_cast<T>(component);
		return castedComponent;

	}
	shared_ptr<Transform> transform();

	shared_ptr<Buffer> GetCameraBuffer() {
		uint8 index = static_cast<uint8>(ComponentType::Camera);
		shared_ptr<Component> cameraComponent = _components[index];
		shared_ptr<Camera> castedComponent = static_pointer_cast<Camera>(cameraComponent);
		return castedComponent->GetCameraBuffer();
	}
	shared_ptr<Buffer> GetLightBuffer() {
		uint8 index = static_cast<uint8>(ComponentType::Light);
		shared_ptr<Component> lightComponent = _components[index];
		shared_ptr<Light> castedComponent = static_pointer_cast<Light>(lightComponent);
		return castedComponent->GetLightBuffer();
	}
	shared_ptr<Buffer> GetTransformBuffer() {
		uint8 index = static_cast<uint8>(ComponentType::Transform);
		shared_ptr<Component> transformComponent = _components[index];
		shared_ptr<Transform> castedComponent = static_pointer_cast<Transform>(transformComponent);
		return castedComponent->GetTransformBuffer();
	}
	void SetName(const wstring& name) { _name = name; }
	wstring& GetName() { return _name; }

private:
	vector<shared_ptr<Component>> _components;
	vector<shared_ptr<GameObject>> _children;
	wstring _name;
	shared_ptr<GameObject> _parent;
};

