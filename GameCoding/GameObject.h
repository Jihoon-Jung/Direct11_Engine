#pragma once
#include "Component.h"
#include "Buffer.h"
#include "Camera.h"
#include "Transform.h"
#include "MeshRenderer.h"
#include "MonoBehaviour.h"
#include "Light.h"
#include "BaseCollider.h"
#include "Terrain.h"
#include "UIImage.h"
#include "Button.h"
#include "Billboard.h"
#include "ParticleSystem.h"
#include "BoxCollider.h"
#include "SphereCollider.h"
#include "Animator.h"
#include "EditorCamera.h"

enum class GameObjectType
{
	NormalObject,
	UIObject
};
class GameObject : public enable_shared_from_this<GameObject>
{
public:
	GameObject();
	~GameObject();

	void Start();
	void Update();
	void LateUpdate();

	void AddComponent(shared_ptr<Component> component);
	void RemoveComponent(shared_ptr<Component> component);
	void SetParent(shared_ptr<GameObject> parent);
	void AddChild(shared_ptr<GameObject> child);
	void SetObjectType(GameObjectType type) { _type = type; }
	shared_ptr<GameObject> GetParent() { return _parent; }
	const vector<shared_ptr<GameObject>>& GetChildren() { return _children; }
	vector<shared_ptr<Component>>& GetComponents() { return _components; }
	GameObjectType GetObjectType() { return _type; }
	void SetTreeNodeOpen(bool open) { _isTreeNodeOpen = open; }
	bool IsTreeNodeOpen() const { return _isTreeNodeOpen; }
	void DetachFromParent()
	{
		if (_parent)
		{
			// 부모의 자식 목록에서 제거
			auto it = std::find(_parent->_children.begin(), _parent->_children.end(), shared_from_this());
			if (it != _parent->_children.end())
				_parent->_children.erase(it);

			// Transform 관계도 해제
			transform()->DetachFromParent();

			_parent = nullptr;
		}
	}

	template <typename T>
	shared_ptr<T> GetComponent()
	{
		ComponentType type = ComponentType::None;
		// MonoBehaviour를 상속받는 클래스인지 먼저 확인
		if (std::is_base_of_v<MonoBehaviour, T>)
			type = ComponentType::Script;

		if (std::is_same_v<T, Transform>)
			type = ComponentType::Transform;
		if (std::is_same_v<T, MeshRenderer>)
			type = ComponentType::MeshRenderer;
		if (std::is_same_v<T, Camera>)
			type = ComponentType::Camera;
		if (std::is_same_v<T, Light>)
			type = ComponentType::Light;
		if (std::is_same_v<T, BaseCollider>)
			type = ComponentType::Collider;
		if (std::is_same_v<T, Terrain>)
			type = ComponentType::Terrain;
		if (std::is_same_v<T, UIImage>)
			type = ComponentType::UIImage;
		if (std::is_same_v<T, Button>)
			type = ComponentType::Button;
		if (std::is_same_v<T, Billboard>)
			type = ComponentType::Billboard;
		if (std::is_same_v<T, ParticleSystem>)
			type = ComponentType::Particle;
		if (std::is_same_v<T, MonoBehaviour>)
			type = ComponentType::Script;
		if (std::is_same_v<T, BoxCollider>)
			type = ComponentType::BoxCollider;
		if (std::is_same_v<T, SphereCollider>)
			type = ComponentType::SphereCollider;
		if (std::is_same_v<T, Animator>)
			type = ComponentType::Animator;

		uint8 index = static_cast<uint8>(type);
		if (std::is_same_v<T, EditorCamera>)
			index = 13;

		shared_ptr<Component> component = _components[index];
		if (component == nullptr)
			return nullptr;
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

	shared_ptr<Buffer> GetEnvCameraBuffer() {
		uint8 index = static_cast<uint8>(ComponentType::Camera);
		shared_ptr<Component> cameraComponent = _components[index];
		shared_ptr<Camera> castedComponent = static_pointer_cast<Camera>(cameraComponent);
		return castedComponent->GetEnvironmentCameraBuffer();
	}
	shared_ptr<Buffer> GetShadowCameraBuffer() {
		uint8 index = static_cast<uint8>(ComponentType::Camera);
		shared_ptr<Component> cameraComponent = _components[index];
		shared_ptr<Camera> castedComponent = static_pointer_cast<Camera>(cameraComponent);
		return castedComponent->GetShadowCameraBuffer();
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

	void SetRenderPassDefault()
	{
		shared_ptr<MeshRenderer> meshRenderer = GetComponent<MeshRenderer>();
		if (meshRenderer)
		{
			meshRenderer->GetRenderPasses()[0]->SetEnvTexture(nullptr);
			meshRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
			meshRenderer->SetUseEnvironmentMap(false);
			meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
		}
		
	}
	void SetRenderPassEnvironmentMap()
	{
		shared_ptr<MeshRenderer> meshRenderer = GetComponent<MeshRenderer>();
		if (meshRenderer)
		{
			meshRenderer->GetRenderPasses()[0]->SetPass(Pass::ENVIRONMENTMAP_RENDER);
			meshRenderer->SetUseEnvironmentMap(true);
			meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
		}
		
	}
	void SetRenderPassOutLine()
	{
		shared_ptr<MeshRenderer> meshRenderer = GetComponent<MeshRenderer>();
		if (meshRenderer)
		{
			meshRenderer->GetRenderPasses()[0]->SetEnvTexture(nullptr);
			meshRenderer->GetRenderPasses()[0]->SetPass(Pass::OUTLINE_RENDER);
			meshRenderer->SetUseEnvironmentMap(false);
			meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::CUSTOM1);
		}
	}
	void SetRenderPassUI()
	{
		shared_ptr<MeshRenderer> meshRenderer = GetComponent<MeshRenderer>();
		if (meshRenderer)
		{
			meshRenderer->GetRenderPasses()[0]->SetEnvTexture(nullptr);
			meshRenderer->GetRenderPasses()[0]->SetPass(Pass::UI_RENDER);
			meshRenderer->SetUseEnvironmentMap(false);
			meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::UI);
		}
	}

	void SetBoneObjectFlag(bool flag) { _isBoneObject = flag; }
	bool GetBoneObjectFlag() { return _isBoneObject; }
	void SetBoneParentObject(shared_ptr<GameObject> parent) { _boneParentObject = parent; }
	weak_ptr<GameObject> GetBoneParentObject() { return _boneParentObject; }
	void SetHasNoneBoneChildrenFlag(bool flag) { _hasNonBoneChildren = flag; }
	bool GetHasNoneBoneChildrenFlag() { return _hasNonBoneChildren; }
	void SetBoneIndex(int index) { _boneIndex = index; }
	int GetBoneIndex() { return _boneIndex; }
	vector<int> GetActiveBoneIndices() { return _activeBoneIndices; }

	void AddActiveBoneIndex(int index) { 
		_activeBoneIndices.push_back(index); 

		std::sort(_activeBoneIndices.begin(), _activeBoneIndices.end());
		auto last = std::unique(_activeBoneIndices.begin(), _activeBoneIndices.end());
		_activeBoneIndices.erase(last, _activeBoneIndices.end());
	}

	void RemoveActiveBoneIndex(int index) { _activeBoneIndices.erase(std::remove(_activeBoneIndices.begin(), _activeBoneIndices.end(), index), _activeBoneIndices.end()); }
	void SetNonBoneChildrenParent(shared_ptr<GameObject> parent) { _nonBoneChildrenParent = parent; }
	shared_ptr<GameObject> GetNoneBoneChildrenParent() { return _nonBoneChildrenParent.lock(); }
private:
	vector<shared_ptr<Component>> _components;
	vector<shared_ptr<GameObject>> _children;
	wstring _name;
	GameObjectType _type;
	shared_ptr<GameObject> _parent;
	bool _isTreeNodeOpen = false;  // 트리 노드의 펼침 상태를 저장
	int _boneIndex;

private:
	bool _isBoneObject = false;
	bool _hasNonBoneChildren = false;
	vector<int> _activeBoneIndices;
	weak_ptr<GameObject> _nonBoneChildrenParent;
	weak_ptr<GameObject> _boneParentObject;
};

