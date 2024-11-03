#pragma once
class GUIManager
{
protected:
	GUIManager() {}
	virtual ~GUIManager() {}
public:
	GUIManager(const GUIManager&) = delete;
	static GUIManager& GetInstance()
	{
		static GUIManager instance;
		return instance;
	}
	void Init();
	void Update();
	void RenderUI_Start();
	void RenderUI();
	void RenderUI_End();
};

