#include "pch.h"
#include "EngineBody.h"

EngineBody::EngineBody()
	: _graphics(GP), _resourceManager(RESOURCE), _guiManager(GUI),
	_inputManager(INPUT), _timeManager(TIME),
	_sceneManager(SCENE), _methodRegistry(MR), _componentFactory(CF)
{
}

EngineBody::~EngineBody()
{
}

void EngineBody::Init(HWND hwnd, int width, int height)
{
	_hwnd = hwnd;
	_rectWidth = width;
	_rectHeight = height;

	GP.Initialize(hwnd, _rectWidth, _rectHeight);
	INPUT.Init(hwnd);
	TIME.Init();
	RESOURCE.Init();
	
    wstring sceneName = L"test_scene";
    tinyxml2::XMLDocument doc;
    string pathStr = "../../SceneInfo.xml";

    if (doc.LoadFile(pathStr.c_str()) == tinyxml2::XML_SUCCESS) {
        tinyxml2::XMLElement* root = doc.FirstChildElement("SceneInfo");
        if (root) {
            const char* sceneNameAttr = root->Attribute("sceneName");
            if (sceneNameAttr) {
                sceneName = Utils::ToWString(sceneNameAttr);
            }
        }
    }
    else {
        tinyxml2::XMLDocument newDoc;
        tinyxml2::XMLElement* root = newDoc.NewElement("SceneInfo");
        root->SetAttribute("sceneName", "test_scene");
        newDoc.InsertFirstChild(root);
        newDoc.SaveFile(pathStr.c_str());
    }

    SCENE.LoadScene(sceneName);

    _editScene = SCENE.GetActiveScene();
	GUI.Init();
}

void EngineBody::Update()
{
    TIME.Update();
    INPUT.Update();


    SCENE.Update();
    RENDER.Update();


    GUI.Update();
}


void EngineBody::Render()
{
    GP.SwapChain();
}


void EngineBody::SetEngineMode(EngineMode mode)
{
    if (_engineMode == mode)
    {
        if (mode == EngineMode::Pause)
            _engineMode = _prevEngineMode;
        else
            return;
    }

    else if (mode == EngineMode::Pause)
    {
        _prevEngineMode = _engineMode;
        _engineMode = mode;
    }
    else if (mode == EngineMode::Play)
    {
        // 현재 씬을 복사해서 플레이에 사용
        _editScene = SCENE.GetActiveScene();
        _playScene = SCENE.LoadPlayScene(_editScene->GetSceneName());


        SCENE.Reset();  // 완전 초기화
        RENDER.Reset(); // 완전 초기화
        GUI.ResetSelectedObject();

        _engineMode = mode;
        SCENE.SetActiveScene(_playScene);
        SCENE.Init();
        RENDER.Init();
    }
    else if (mode == EngineMode::Edit)
    {
        // 편집 씬으로 복구
        SCENE.Reset();  // 완전 초기화
        RENDER.Reset(); // 완전 초기화
        GUI.ResetSelectedObject();

        _engineMode = mode;
        SCENE.SetActiveScene(_editScene);
        _playScene = nullptr;

        SCENE.Init();
        RENDER.Init();
    }

    
}



