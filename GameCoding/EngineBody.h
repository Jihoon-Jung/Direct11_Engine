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
#include "MethodRegistry.h"
#include "ComponentFactory.h"

class EngineBody
{
private:
    EngineBody();
    ~EngineBody();

protected:
    EngineBody(const EngineBody&) = delete;
    EngineBody& operator=(const EngineBody&) = delete;

public:
    static EngineBody& GetInstance()
    {
        static EngineBody instance;
        return instance;
    }

    void Init(HWND hwnd, int width, int height);
    void Update();
    void Render();

    /*void Play() { isStop = false; }
    void Stop() { isStop = true; }*/
    void Play();

    //void Stop() { isStop = true; }
    void Stop();
    void Pause() { isPaused = true; }
    void Resume() { isPaused = false; }
    bool IsPlaying() const { return !isStop && !isPaused; }
    bool IsPaused() const { return isPaused; }
    bool IsStopped() const { return isStop; }
    void RenderInitialScreen();
private:
    void UpdateGame();    // 게임 로직 업데이트
    void UpdateEditor();  // 에디터(GUI) 업데이트

private:
    int _rectWidth = 0;
    int _rectHeight = 0;
    HWND _hwnd;

    Graphics& _graphics;
    InputManager& _inputManager;
    TimeManager& _timeManager;
    SceneManager& _sceneManager;
    ResourceManager& _resourceManager;
    MethodRegistry& _methodRegistry;
    ComponentFactory& _componentFactory;

    bool isStop = false;
    bool isPaused = false;
};
