#pragma once
#include "Scene.h"

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
	shared_ptr<Scene> GetActiveScene() { return _activeScene; }


private:
	shared_ptr<Scene> _activeScene;
	map<wstring, shared_ptr<Scene>> _scenes;
};

