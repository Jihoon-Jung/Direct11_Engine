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

    // ���� ���� ������Ʈ�� ���Ǻ� ����
    if (!isStop && !isPaused)
    {
        UpdateGame();
    }

    // ������ ���� ������Ʈ�� �׻� ����
    UpdateEditor();
}

void EngineBody::UpdateGame()
{
    SCENE.Update();
    RENDER.Update();
}

void EngineBody::UpdateEditor()
{
    // GUI ���� ������Ʈ
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
        // ���� ���� �����ؼ� �÷��̿� ���
        _editScene = SCENE.GetActiveScene();
        _playScene = SCENE.LoadPlayScene(_editScene->GetSceneName());
        SCENE.GetActiveScene() = _playScene;
    }
    else if (mode == EngineMode::Edit)
    {
        // ���� ������ ����
        SCENE.GetActiveScene() = _editScene;
        _playScene = nullptr;
    }

    _engineMode = mode;
}


