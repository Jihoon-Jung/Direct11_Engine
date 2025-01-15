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
        return;

    if (mode == EngineMode::Play)
    {
        // ���� ���� �����ؼ� �÷��̿� ���
        _editScene = SCENE.GetActiveScene();
        _playScene = SCENE.LoadPlayScene(_editScene->GetSceneName());


        SCENE.Reset();  // ���� �ʱ�ȭ
        RENDER.Reset(); // ���� �ʱ�ȭ

        SCENE.SetActiveScene(_playScene);
        SCENE.Init();
        RENDER.Init();
    }
    else if (mode == EngineMode::Edit)
    {
        // ���� ������ ����
        SCENE.Reset();  // ���� �ʱ�ȭ
        RENDER.Reset(); // ���� �ʱ�ȭ

        SCENE.SetActiveScene(_editScene);
        _playScene = nullptr;

        SCENE.Init();
        RENDER.Init();
    }

    _engineMode = mode;
}


