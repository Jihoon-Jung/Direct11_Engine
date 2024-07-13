#pragma once

class GameObject;
class Transform;

enum class ComponentType : uint8
{
	Transform,
	MeshRenderer,
	Camera,
	Animator,
	Light,
	Collider,
	Terrain,
	UIImage,
	Button,
	Billboard,
	Script
};
enum
{
	FIXED_COMPONENT_COUNT = 10
};

class Component
{
public:
	Component(ComponentType type);
	~Component();

	virtual void Start() {};
	virtual void Update() {};
	virtual void LateUpdate() {};

	shared_ptr<GameObject> GetGameObject() { return _gameObject; }
	shared_ptr<Transform> GetTransform();
	ComponentType GetType() { return _type; }


protected:
	ComponentType _type;
	shared_ptr<GameObject> _gameObject;
	
private:
	friend class GameObject;
	void SetGameObject(shared_ptr<GameObject> gameObject) { _gameObject = gameObject; }
};

