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
	void AddBoneGameObject(shared_ptr<GameObject> boneGameObject);
	void RemoveGameObject(shared_ptr<GameObject> gameObject);
	void Picking();
	void UIPicking();
	void CheckCollision();
	void AddPickedObject(shared_ptr<GameObject> pickedObject) { picked = pickedObject; }
	void SetMainCamera(shared_ptr<GameObject> camera) { _mainCamera = camera; }
	void SetSceneName(wstring sceneName) { _sceneName = sceneName; }
	void SwitchMainCameraToEditorCamera() { _mainCamera = Find(L"EditorCamera"); }
	void SwitchMainCameraToMainCamera() { _mainCamera = Find(L"MainCamera"); }
	void SortAndSweep(vector<shared_ptr<BaseCollider>>& colliders, int axis);
	float GetMinBound(const shared_ptr<BaseCollider>& collider, int axis);
	float GetMaxBound(const shared_ptr<BaseCollider>& collider, int axis);
	float CalculateSpread(const vector<shared_ptr<BaseCollider>>& colliders, int axis);
	const vector<shared_ptr<GameObject>>& GetGameObjects() { return _gameObjects; }
	shared_ptr<GameObject> Find(const wstring& name);

	shared_ptr<GameObject> FindWithComponent(ComponentType type);
	shared_ptr<GameObject> GetPickedObject() const { return picked; }
	shared_ptr<GameObject> GetMainCamera() { return _mainCamera; }
	shared_ptr<GameObject> GetMainLight() { return _mainLignt; }
	Vec3 GetCameraPos();
	wstring GetSceneName() { return _sceneName; }


private:
	vector<shared_ptr<GameObject>> _gameObjects;
	vector<shared_ptr<GameObject>> _boneGameobjects;
	shared_ptr<GameObject> _billboard_obj = make_shared<GameObject>();
	shared_ptr<Transform> _billboard_Terrain_transform = make_shared<Transform>();
	shared_ptr<Billboard> _billboard = make_shared<Billboard>();
	
	Vec3 hitPoint;

	shared_ptr<GameObject> picked = nullptr;
	int32 firstClickedMouseX = 0;
	int32 firstClickedMouseY = 0;

	wstring _sceneName = L"test_scene";

	shared_ptr<GameObject> _mainCamera;
	shared_ptr<GameObject> _mainLignt;
};

