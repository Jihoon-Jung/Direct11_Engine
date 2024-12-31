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
	void RemoveComponentFromGameObjectInXML(const wstring& sceneName, const wstring& objectName, const shared_ptr<Component>& component);

	void UpdateGameObjectTransformInXML(const wstring& sceneName, const wstring& objectName, const Vec3& position, const Vec3& rotation, const Vec3& scale);
	void UpdateGameObjectColliderInXML(const wstring& sceneName, const wstring& objectName,
		const Vec3& center, const Vec3& scale, bool isBoxCollider);
	void UpdateGameObjectSphereColliderInXML(const wstring& sceneName, const wstring& objectName,
		const Vec3& center, float radius);
	void RemoveGameObjectFromXML(const wstring& sceneName, const wstring& objectName);

	void UpdateMeshInXML(const wstring& sceneName, const wstring& objectName, const string& meshName);
	void UpdateMaterialInXML(const wstring& sceneName, const wstring& objectName, const string& materialName);

	void UpdateAnimatorClipInXML(const wstring& sceneName, const wstring& objectName,
		const string& clipName, float speed, bool isLoop);

	void UpdateAnimatorTransitionInXML(const wstring& sceneName, const wstring& objectName,
		const string& clipAName, const string& clipBName,
		float duration, float offset, float exitTime, bool hasExitTime);

	void AddAnimatorParameterToXML(const wstring& sceneName, const wstring& objectName,
		const string& paramName, Parameter::Type paramType);

	void UpdateAnimatorParameterInXML(const wstring& sceneName, const wstring& objectName,
		const string& paramName, const Parameter& param);

	void UpdateAnimatorTransitionConditionInXML(const wstring& sceneName, const wstring& objectName,
		const string& clipAName, const string& clipBName,
		const vector<Condition>& conditions);

	void RemoveAnimatorTransitionFromXML(const wstring& sceneName, const wstring& objectName,
		const string& clipAName, const string& clipBName);

	void UpdateAnimatorEntryClipInXML(const wstring& sceneName, const wstring& objectName,
		const string& entryClipName);

	void AddAnimatorTransitionToXML(const wstring& sceneName, const wstring& objectName,
		const string& clipAName, const string& clipBName);

	void RemoveAnimatorParameterFromXML(const wstring& sceneName, const wstring& objectName, const string& paramName);

	void UpdateAnimatorNodePositionInXML(const wstring& sceneName, const wstring& objectName,
		const string& clipName, const ImVec2& position);

	ImVec2 GetAnimatorNodePositionFromXML(const wstring& sceneName, const wstring& objectName,
		const string& clipName);

	void UpdateAnimatorTransitionFlagInXML(const wstring& sceneName, const wstring& objectName,
		const string& clipAName, const string& clipBName,
		bool flag, bool hasCondition);

	void CreateCubeToScene(const wstring& sceneName);
	void CreateSphereToScene(const wstring& sceneName);
	void CreateCylinderToScene(const wstring& sceneName);

private:

    void SaveGameObjectToXML(const wstring& path, const wstring& name,
        const Vec3* position, const Vec3* rotation, const Vec3* scale,
        const shared_ptr<GameObject>& parent);

	shared_ptr<Scene> _activeScene;
	map<wstring, shared_ptr<Scene>> _scenes;
};

