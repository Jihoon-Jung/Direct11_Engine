#pragma once
#include "Graphics.h"
#include "ResourceManager.h"
#include "Texture.h"
#include "Material.h"
#include "Shader.h"
#include "Mesh.h"
#include "VertexDescription.h"
#include "Engine.h"
#include "Geometry.h"
#include "InputManager.h"
#include "TimeManager.h"
#include "SceneManager.h"
#include "RenderManager.h"

class EngineBody
{
public:
	EngineBody();
	~EngineBody();
	void Init(HWND hwnd, int width, int height);
	void Update();
	void Render();

private:
	int _rectWidth = 0;
	int _rectHeight = 0;
	HWND _hwnd;

	Graphics& _graphics;
	InputManager& _inputManager;
	TimeManager& _timeManager;
	SceneManager& _sceneManager;
	ResourceManager& _resourceManager;

};
