#pragma once

#define _XM_NO_INTRINSICS_

#include "Types.h"
#include "Values.h"

// STL
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <memory>
#include <array>
using namespace std;

// WIN
#include <windows.h>
#include <assert.h>

// ImGUI
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
// ImGuizmo
#include "ImGuizmo.h"

// DX
#include <d3d11.h>
#include <d3dcompiler.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <DirectXTex/DirectXTex.h>
#include <DirectXTex/DirectXTex.inl>
using namespace DirectX;
using namespace Microsoft::WRL;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

#ifdef _DEBUG
#pragma comment(lib, "DirectXTex\\DirectXTex_debug.lib")
#else
#pragma comment(lib, "DirectXTex\\DirectXTex.lib")
#endif

#define CHECK(p)	assert(SUCCEEDED(p))

#include "Graphics.h"
#include "EngineBody.h"
#include "InputManager.h"
#include "TimeManager.h"
#include "ResourceManager.h"
#include "SceneManager.h"
#include "RenderManager.h"
#include "GUIManager.h"
//#include "MethodRegistry.h"
//#include "ComponentFactory.h"

// COMPONENT (have to add when create new Monobehaviour)
#include "Component.h"
#include "Transform.h"
#include "Camera.h"
#include "MeshRenderer.h"
#include "MonoBehaviour.h"
#include "Animator.h"
#include "Light.h"
#include "BaseCollider.h"
#include "SphereCollider.h"
#include "BoxCollider.h"
#include "Terrain.h"
#include "UIImage.h"
#include "Button.h"
#include "ParticleSystem.h"


#include "ShaderSlot.h"
#include "ConstantBufferType.h"
#include "RasterizerStateInfo.h"


#include "MathHelper.h"

// Buffer
#include "Buffer.h"
#include "TextureBuffer.h"


#define GP							Graphics::GetInstance()
#define DEVICE						Graphics::GetInstance().GetDevice()
#define DEVICECONTEXT				Graphics::GetInstance().GetDeviceContext()
#define TIME						TimeManager::GetInstance()
#define INPUT						InputManager::GetInstance()
#define RESOURCE					ResourceManager::GetInstance()
#define SCENE						SceneManager::GetInstance()
#define RENDER						RenderManager::GetInstance()
#define GUI							GUIManager::GetInstance()
#define ENGINE                      EngineBody::GetInstance()
#define MR							MethodRegistry::GetInstance()
#define CF							ComponentFactory::GetInstance()