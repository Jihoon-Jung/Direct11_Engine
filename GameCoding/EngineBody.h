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


enum class EngineMode
{
    Edit,   // 에디터 모드
    Play,   // 실행 모드
    Pause   // 일시정지 모드
};

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


public:
    EngineMode GetEngineMode() { return _engineMode; }
    void SetEngineMode(EngineMode mode);
    bool IsEditMode() { return _engineMode == EngineMode::Edit; }
    bool IsPlayMode() { return _engineMode == EngineMode::Play; }
    bool IsPausedMode() { return _engineMode == EngineMode::Pause; }

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

    EngineMode _engineMode = EngineMode::Edit;
    shared_ptr<Scene> _editScene;
    shared_ptr<Scene> _playScene;

};
