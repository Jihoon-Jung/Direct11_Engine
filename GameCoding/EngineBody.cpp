#include "pch.h"
#include "EngineBody.h"

EngineBody::EngineBody()
	: _graphics(GP), _resourceManager(RESOURCE),
	_inputManager(INPUT), _timeManager(TIME),
	_sceneManager(SCENE)
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
	SCENE.LoadScene(L"Test");
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
    SCENE.LoadScene(L"Test");
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



//////////////////
// Stop �Ŀ� �ٽ� �����ϸ� �׸��ڰ� ���� ����!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

