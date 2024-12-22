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
// Stop 후에 다시 시작하면 그림자가 돌지 않음!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

