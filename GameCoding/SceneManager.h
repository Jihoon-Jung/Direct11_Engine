#pragma once
#include "Scene.h"
#include "tinyxml2.h"
#include <filesystem>
#include "Utils.h"

class SceneManager
{
protected:
	SceneManager() {}
	virtual ~SceneManager() {}
public:
	SceneManager(const SceneManager&) = delete;
	static SceneManager& GetInstance()
	{
		static SceneManager instance;
		return instance;
	}
public:

	void Init();
	void Update();
	void LoadScene(wstring sceneName);
	void LoadTestScene();
	void LoadTestScene2();

	shared_ptr<Scene> GetActiveScene() { return _activeScene; }

	void SaveAndLoadGameObjectToXML(const wstring& sceneName, const wstring& name,
		const Vec3& position = Vec3::Zero,
		const Vec3& rotation = Vec3::Zero,
		const Vec3& scale = Vec3::One,
		shared_ptr<GameObject> parent = nullptr);

	void AddComponentToGameObjectAndSaveToXML(const wstring& path,
		const wstring& name,
		const shared_ptr<Component>& component,
		const wstring& material = L"",
		const wstring& mesh = L"",
		const wstring& model = L"");

	void UpdateGameObjectTransformInXML(const wstring& sceneName, const wstring& objectName, const Vec3& position, const Vec3& rotation, const Vec3& scale);
	void UpdateGameObjectColliderInXML(const wstring& sceneName, const wstring& objectName,
		const Vec3& center, const Vec3& scale, bool isBoxCollider);
	void UpdateGameObjectSphereColliderInXML(const wstring& sceneName, const wstring& objectName,
		const Vec3& center, float radius);

private:

    void SaveGameObjectToXML(const wstring& path, const wstring& name,
        const Vec3* position, const Vec3* rotation, const Vec3* scale,
        const shared_ptr<GameObject>& parent);

	shared_ptr<Scene> _activeScene;
	map<wstring, shared_ptr<Scene>> _scenes;
};

