#include "pch.h"
#include "EngineBody.h"

EngineBody::EngineBody()
	: _graphics(GP), _resourceManager(RESOURCE),
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
	SCENE.LoadScene(L"test_scene");
    _editScene = SCENE.GetActiveScene();

    SCENE.Init();
	RENDER.Init();
	GUI.Init();

    RenderInitialScreen();
 
}

void EngineBody::Update()
{
    TIME.Update();
    INPUT.Update();

    // 게임 로직 업데이트는 조건부 실행
    if (!isStop && !isPaused)
    {
        UpdateGame();
    }

    // 에디터 관련 업데이트는 항상 실행
    UpdateEditor();
}

void EngineBody::UpdateGame()
{
    SCENE.Update();
    RENDER.Update();
}

void EngineBody::UpdateEditor()
{
    // GUI 관련 업데이트
    GUI.Update();
}

void EngineBody::Render()
{
    GP.SwapChain();
}

void EngineBody::Play()
{
    isStop = false;
    isPaused = false;
}

void EngineBody::Stop()
{
    SCENE.LoadScene(L"test_scene");
    SCENE.Init();
    RENDER.Init();
    UpdateGame();
    isStop = true;
}

void EngineBody::RenderInitialScreen()
{
    TIME.Update();
    INPUT.Update();
    UpdateGame();
    //UpdateEditor();
    isStop = true;
    
}

void EngineBody::SetEngineMode(EngineMode mode)
{
    if (_engineMode == mode)
        return;

    if (mode == EngineMode::Play)
    {
        // 현재 씬을 복사해서 플레이에 사용
        _editScene = SCENE.GetActiveScene();
        _playScene = SCENE.LoadPlayScene(_editScene->GetSceneName());
        SCENE.GetActiveScene() = _playScene;
    }
    else if (mode == EngineMode::Edit)
    {
        // 편집 씬으로 복구
        SCENE.GetActiveScene() = _editScene;
        _playScene = nullptr;
    }

    _engineMode = mode;
}


