#include "pch.h"
#include "Scene.h"

void Scene::Start()
{
	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		gameObject->Start();
	}
}

void Scene::Update()
{
	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		gameObject->Update();
	}
}

void Scene::LateUpdate()
{
	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		gameObject->LateUpdate();
	}
}

void Scene::AddGameObject(shared_ptr<GameObject> gameObject)
{
	_gameObjects.push_back(gameObject);
}

void Scene::RemoveGameObject(shared_ptr<GameObject> gameObject)
{
	auto it = std::find(_gameObjects.begin(), _gameObjects.end(), gameObject);
	if (it != _gameObjects.end())
	{
		_gameObjects.erase(it);
	}
}

shared_ptr<GameObject> Scene::Find(const wstring& name)
{
	for (shared_ptr<GameObject> gameObject : _gameObjects)
	{
		if (gameObject->GetName() == name)
			return gameObject;
	}
	return nullptr;
}

shared_ptr<GameObject> Scene::FindWithComponent(ComponentType type)
{
	for (shared_ptr<GameObject> gameObject : _gameObjects)
	{
		switch (type)
		{
		case ComponentType::Transform:
			if (gameObject->GetComponent<Transform>() != nullptr)
				return gameObject;
			break;
		case ComponentType::MeshRenderer:
			if (gameObject->GetComponent<MeshRenderer>() != nullptr)
				return gameObject;
			break;
		case ComponentType::Camera:
			if (gameObject->GetComponent<Camera>() != nullptr)
				return gameObject;
			break;
		case ComponentType::Animator:
			if (gameObject->GetComponent<Animator>() != nullptr)
				return gameObject;
			break;
		case ComponentType::Light:
			if (gameObject->GetComponent<Light>() != nullptr)
				return gameObject;
			break;
		default:
			break;
		}
	}
	return nullptr;
}
