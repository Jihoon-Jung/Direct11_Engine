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

	const vector<shared_ptr<GameObject>>& GetGameObjects() { return _gameObjects; }
	shared_ptr<GameObject> Find(const wstring& name);

	shared_ptr<GameObject> FindWithComponent(ComponentType type);
private:
	vector<shared_ptr<GameObject>> _gameObjects;
};

