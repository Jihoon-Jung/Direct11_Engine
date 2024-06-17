#include "pch.h"
#include "EngineBody.h"

EngineBody::EngineBody()
	:_graphics(Graphics::GetInstance()), _resourceManager(ResourceManager::GetInstance()),
	_inputManager(InputManager::GetInstance()), _timeManager(TimeManager::GetInstance()),
	_sceneManager(SceneManager::GetInstance())
{
}

EngineBody::~EngineBody()
{
}

void EngineBody::Init(HWND hwnd, int width, int height)
{
	_rectWidth = width;
	_rectHeight = height;

	GP.Initialize(hwnd, _rectWidth, _rectHeight);
	INPUT.Init(hwnd);
	TIME.Init();
	RENDER.Init();
	RESOURCE.Init();
	SCENE.LoadScene(L"Test");
}

void EngineBody::Update()
{
	TIME.Update();
	INPUT.Update();
	SCENE.Update();
}

void EngineBody::Render()
{
	RENDER.Update();
}


