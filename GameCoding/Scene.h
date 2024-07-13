#pragma once
#include "GameObject.h"

class Scene
{
public:
	void Start();
	void Update();
	void LateUpdate();

public:
	void AddGameObject(shared_ptr<GameObject> gameObject);
	void RemoveGameObject(shared_ptr<GameObject> gameObject);
	void Picking();
	void UIPicking();
	void CheckCollision();
	const vector<shared_ptr<GameObject>>& GetGameObjects() { return _gameObjects; }
	shared_ptr<GameObject> Find(const wstring& name);

	shared_ptr<GameObject> FindWithComponent(ComponentType type);

	Vec3 GetCameraPos();
private:
	vector<shared_ptr<GameObject>> _gameObjects;

	Vec3 hitPoint;

	shared_ptr<GameObject> picked = nullptr;
	int32 firstClickedMouseX = 0;
	int32 firstClickedMouseY = 0;
};

