#pragma once

class GameObject;
class Transform;

enum class ComponentType : uint8
{
	Transform,
	RectTransform,
	MeshRenderer,
	Camera,
	Animator,
	Light,
	Collider,
	Terrain,
	UIImage,
	Button,
	Billboard,
	Particle,
	BoxCollider,
	SphereCollider,
	ParticleSystem,
	Script,
	None
};
enum
{
	FIXED_COMPONENT_COUNT = 14
};

class Component
{
public:
	Component(ComponentType type);
	~Component();

	virtual void Start() {};
	virtual void Update() {};
	virtual void LateUpdate() {};

	shared_ptr<GameObject> GetGameObject() {
		if (auto gameObject = _gameObject.lock())
			return gameObject;
		return nullptr;
	}
	shared_ptr<Transform> GetTransform();
	ComponentType GetType() { return _type; }


protected:
	ComponentType _type;
	weak_ptr<GameObject> _gameObject;
	
private:
	friend class GameObject;
	void SetGameObject(shared_ptr<GameObject> gameObject) { _gameObject = gameObject; }
};

