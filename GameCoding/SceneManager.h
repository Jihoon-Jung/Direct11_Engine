#pragma once
#include "Scene.h"
#include "tinyxml2.h"
#include <filesystem>
#include "Utils.h"


struct AnimatorEventLoadData
{
	shared_ptr<GameObject> gameObject;
	shared_ptr<Animator> animator;
	string clipName;
	float time;
	string functionKey;
};
struct ButtonEventLoadData {
	shared_ptr<GameObject> gameObject;
	shared_ptr<Button> button;
	string functionKey;
};

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

	void Reset() {
		_activeScene = nullptr;
		_scenes.clear();
	}

	void Init();
	void Update();
	void LoadScene(wstring sceneName);
	void LoadTestScene(wstring sceneName);
	void LoadSceneXML(wstring sceneName);
	void LoadTestInstancingScene();
	void CreateNewScene(wstring sceneName);
	void CheckCollision();
	shared_ptr<GameObject> LoadPrefabToScene(wstring prefab);

	shared_ptr<Scene> LoadPlayScene(wstring sceneName);
	
	shared_ptr<Scene> GetActiveScene() { return _activeScene; }
	void SetActiveScene(shared_ptr<Scene> scene) { _activeScene = scene; }
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

	void UpdateGameObjectParentInXML(const wstring& sceneName, const wstring& objectName,
		const Vec3& localPosition, const Quaternion& localRotation, const Vec3& localScale,
		const wstring& parentName, const wstring& boneRootParentName = L"");

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

	void UpdateAnimatorClipEventsInXML(const wstring& sceneName, const wstring& objectName,
		const string& clipName, const vector<AnimationEvent>& events);

	void UpdateGameObjectMaterialInXML(const wstring& sceneName, const wstring& objectName, const wstring& materialName);

	void UpdateGameObjectRenderPassInXML(const wstring& sceneName, const wstring& objectName,
		Pass pass, bool useEnvironmentMap);
	void UpdateGameObjectParticleSystemInXML(const wstring& sceneName, const wstring& objectName,
		float speed, bool endParticle, ParticleType type);

	void UpdateButtonEventInXML(const wstring& sceneName, const wstring& objectName, const string& functionKey);

	shared_ptr<GameObject> CreateCubeToScene(const wstring& sceneName);
	shared_ptr<GameObject> CreateSphereToScene(const wstring& sceneName);
	shared_ptr<GameObject> CreateCylinderToScene(const wstring& sceneName);
	shared_ptr<GameObject> CreateQuadToScene(const wstring& sceneName);
	shared_ptr<GameObject> CreateGridToScene(const wstring& sceneName);
	shared_ptr<GameObject> CreateTerrainToScene(const wstring& sceneName);
	shared_ptr<GameObject> CreateParticleToScene(const wstring& sceneName);
	shared_ptr<GameObject> CreateAnimatedMeshToScene(const wstring& sceneName, const wstring& modelName);
	shared_ptr<GameObject> CreateStaticMeshToScene(const wstring& sceneName, const wstring& modelName);

private:

    void SaveGameObjectToXML(const wstring& path, const wstring& name,
        const Vec3* position, const Vec3* rotation, const Vec3* scale,
        const shared_ptr<GameObject>& parent);

	shared_ptr<Scene> _activeScene;
	map<wstring, shared_ptr<Scene>> _scenes;

	bool _isCreateNewScene = false;

};

