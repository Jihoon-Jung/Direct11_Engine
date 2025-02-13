#include "pch.h"
#include "Scene.h"

void Scene::Start()
{
	if (ENGINE.GetEngineMode() == EngineMode::Edit || ENGINE.GetEngineMode() == EngineMode::Pause)
		SwitchMainCameraToEditorCamera();
	else
		SwitchMainCameraToMainCamera();

	_mainLignt = Find(L"MainLight");

	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		gameObject->Start();
	}
}

void Scene::Update()
{
	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		gameObject->Update();
	}
}

void Scene::LateUpdate()
{
	for (const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		gameObject->LateUpdate();
	}
	Picking();
	UIPicking();
	//CheckCollision();
	
}

void Scene::AddGameObject(shared_ptr<GameObject> gameObject)
{
	_gameObjects.push_back(gameObject);
}
void Scene::AddBoneGameObject(shared_ptr<GameObject> gameObject)
{
	_boneGameobjects.push_back(gameObject);
}
void Scene::RemoveGameObject(shared_ptr<GameObject> gameObject)
{
	gameObject->DetachFromParent();

	shared_ptr<GameObject> nonBoneChildrenParent = gameObject->GetNoneBoneChildrenParent();
	if (nonBoneChildrenParent)
	{
		nonBoneChildrenParent->GetBoneParentObject().lock()->RemoveActiveBoneIndex(nonBoneChildrenParent->GetBoneIndex());
		nonBoneChildrenParent->SetHasNoneBoneChildrenFlag(false);
	}

	vector<shared_ptr<GameObject>> children = gameObject->GetChildren(); // 복사본 생성
	for (shared_ptr<GameObject> child : children)
	{
		child->DetachFromParent();
		RemoveGameObject(child); // 재귀적으로 자식들도 제거
		SCENE.RemoveGameObjectFromXML(SCENE.GetActiveScene()->GetSceneName(), child->GetName());
	}

	auto it = std::find(_gameObjects.begin(), _gameObjects.end(), gameObject);
	if (it != _gameObjects.end())
	{
		_gameObjects.erase(it);
	}

	it = std::find(_boneGameobjects.begin(), _boneGameobjects.end(), gameObject);
	if (it != _boneGameobjects.end())
	{
		_boneGameobjects.erase(it);
	}
	RENDER.GetRenderableObject();
}

void Scene::Picking()
{
	if (!GUI.isSceneView()/*ENGINE.GetEngineMode() == EngineMode::Play*/)
		return;

	// ImGui 윈도우가 마우스 입력을 캡처하고 있는지 확인
	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse)
		return;

	Matrix worldMatrix;
	shared_ptr<Camera> cameraComponent = _mainCamera->GetComponent<Camera>();

	Matrix projectionMatrix = cameraComponent->GetProjectionMatrix();
	Matrix viewMatrix = cameraComponent->GetViewMatrix();
	shared_ptr<GameObject> billboard_Terrain;

	if (INPUT.GetPublicButtonDown(KEY_TYPE::LBUTTON) && !ImGuizmo::IsUsing() && !ImGuizmo::IsOver())
	{
		firstClickedMouseX = INPUT.GetPublicMousePos().x;
		firstClickedMouseY = INPUT.GetPublicMousePos().y;

		const auto& gameObjects = GetGameObjects();

		float minDistance = FLT_MAX;
		picked = nullptr;

		for (auto& gameObject : gameObjects)
		{
			shared_ptr<BaseCollider> collider = gameObject->GetComponent<BaseCollider>();
			if (collider != nullptr)
			{
				worldMatrix = gameObject->transform()->GetWorldMatrix();
				Ray ray = GP.GetViewport().GetRayFromScreenPoint(firstClickedMouseX, firstClickedMouseY, worldMatrix, viewMatrix, projectionMatrix, _mainCamera->transform()->GetWorldPosition());
				float distance = 0.f;
				if (gameObject->GetComponent<BaseCollider>()->Intersects(ray, OUT distance) == false)
					continue;

				if (distance < minDistance)
				{
					minDistance = distance;
					picked = gameObject;
				}
			}
		}
		for (auto& gameObject : gameObjects)
		{
			shared_ptr<Terrain> terrain = gameObject->GetComponent<Terrain>();
			if (terrain != nullptr)
			{
				float distance = 0.f;
				Ray ray = GP.GetViewport().GetRayFromScreenPoint(firstClickedMouseX, firstClickedMouseY, gameObject->transform()->GetWorldMatrix(), viewMatrix, projectionMatrix, _mainCamera->transform()->GetWorldPosition());
				
				if (terrain->Pick(ray, distance, hitPoint) == false)
					continue;
				else
					int a = 1;
				if (distance < minDistance)
				{
					minDistance = distance;
					picked = gameObject;

					//if (picked->GetComponent<Billboard>() != nullptr)
					{
						
						if (_billboard_obj->GetComponent<Transform>() == nullptr)
						{
							_billboard_Terrain_transform->SetPosition(hitPoint);
							_billboard_Terrain_transform->SetLocalScale(Vec3(0.5f));
							_billboard_obj->AddComponent(_billboard_Terrain_transform);
						}
							
						if (_billboard_obj->GetComponent<Billboard>() == nullptr)
							_billboard_obj->AddComponent(_billboard);

						_billboard_obj->SetName(L"Billboard");
						Vec2 scale = Vec2(0.1f);
						_billboard->Add(hitPoint, scale);
					}
				}
			}
			
		}
		/*if (Find(L"Billboard") == nullptr)
			AddGameObject(_billboard_obj);*/
	}

	//if (INPUT.GetButton(KEY_TYPE::LBUTTON))
	//{
	//	if (picked != nullptr)
	//	{
	//		int32 currentMouseX = INPUT.GetMousePos().x;
	//		int32 currentMouseY = INPUT.GetMousePos().y;

	//		Vec3 startScreenPos = Vec3(firstClickedMouseX, firstClickedMouseY, 0.0f);
	//		Vec3 endScreenPos = Vec3(currentMouseX, currentMouseY, 0.0f);

	//		Vec3 startWorldPos = GP.GetViewport().Unproject(startScreenPos, worldMatrix, viewMatrix, projectionMatrix);
	//		Vec3 endWorldPos = GP.GetViewport().Unproject(endScreenPos, worldMatrix, viewMatrix, projectionMatrix);
	//		Vec3 worldMoveRatio = endWorldPos - startWorldPos;

	//		picked->transform()->SetLocalPosition(picked->transform()->GetLocalPosition() + worldMoveRatio * 50.0f);

	//		// Update first clicked position for next frame
	//		firstClickedMouseX = currentMouseX;
	//		firstClickedMouseY = currentMouseY;
	//	}
	//		

	//}
	
	if (INPUT.GetPublicButtonUp(KEY_TYPE::LBUTTON))
	{
		firstClickedMouseX = 0;
		firstClickedMouseY = 0;
		//picked = nullptr;
	}
	// ESC 키를 누르면 선택 해제하도록 추가 (선택사항)
    if (INPUT.GetPublicButtonDown(KEY_TYPE::ESC))
    {
        picked = nullptr;
    }
}

void Scene::UIPicking()
{
	if (INPUT.GetPublicButtonDown(KEY_TYPE::LBUTTON) == false)
		return;
	POINT screenPoint = INPUT.GetPublicMousePos();

	const vector<shared_ptr<GameObject>>& gameObjects = GetGameObjects();

	for (shared_ptr<GameObject> gameObject : gameObjects)
	{
		if (!gameObject)
			continue;

		if (gameObject->GetComponent<Button>() != nullptr)
		{
			if (gameObject->GetComponent<Button>()->Picked(screenPoint))
			{
				if (ENGINE.GetEngineMode() == EngineMode::Play)
				{
					if (!GUI.isSceneView())
						gameObject->GetComponent<Button>()->InvokeOnClicked();
					else
						picked = gameObject;
				}
				else
					picked = gameObject;
			}
		}
		else if (gameObject->GetComponent<UIImage>() != nullptr)
		{
			if (GUI.isSceneView()/*ENGINE.GetEngineMode() == EngineMode::Edit || ENGINE.GetEngineMode() == EngineMode::Pause*/)
			{
				if (gameObject->GetComponent<UIImage>()->Picked(screenPoint))
					picked = gameObject;
			}

		}
	}

}

void Scene::CheckCollision()
{
	vector<shared_ptr<BaseCollider>> colliders;
	const auto& gameObjects = GetGameObjects();
	for (shared_ptr<GameObject> gameObject : gameObjects)
	{
		if (gameObject->GetComponent<BaseCollider>() == nullptr)
			continue;

		colliders.push_back(gameObject->GetComponent<BaseCollider>());
	}

	// BruteForce
	for (int32 i = 0; i < colliders.size(); i++)
	{
		for (int32 j = i + 1; j < colliders.size(); j++)
		{
			shared_ptr<BaseCollider>& other = colliders[j];
			if (colliders[i]->Intersects(other))
			{
				RemoveGameObject(colliders[i]->GetGameObject());
				RemoveGameObject(other->GetGameObject());
			}
		}
	}
}

shared_ptr<GameObject> Scene::Find(const wstring& name)
{
	for (shared_ptr<GameObject> gameObject : _gameObjects)
	{
		if (gameObject->GetName() == name)
			return gameObject;
	}
	for (shared_ptr<GameObject> gameObject : _boneGameobjects)
	{
		if (gameObject->GetName() == name)
			return gameObject;
	}
	return nullptr;
}

shared_ptr<GameObject> Scene::FindWithComponent(ComponentType type)
{
	for (shared_ptr<GameObject> gameObject : _gameObjects)
	{
		switch (type)
		{
		case ComponentType::Transform:
			if (gameObject->GetComponent<Transform>() != nullptr)
				return gameObject;
			break;
		case ComponentType::MeshRenderer:
			if (gameObject->GetComponent<MeshRenderer>() != nullptr)
				return gameObject;
			break;
		case ComponentType::Camera:
			if (gameObject->GetComponent<Camera>() != nullptr)
			{
				if (GUI.isSceneView()/*ENGINE.GetEngineMode() == EngineMode::Edit || ENGINE.GetEngineMode() == EngineMode::Pause*/)
				{
					if (gameObject->GetName() != L"EditorCamera")
						continue;
					else
						return gameObject;
				}
				else
				{
					if (gameObject->GetName() != L"MainCamera")
						continue;
					else
						return gameObject;
				}
			}
			break;
		case ComponentType::Animator:
			if (gameObject->GetComponent<Animator>() != nullptr)
				return gameObject;
			break;
		case ComponentType::Light:
			if (gameObject->GetComponent<Light>() != nullptr)
				return gameObject;
			break;
		default:
			break;
		}
	}
	return nullptr;
}

Vec3 Scene::GetCameraPos()
{
	for(const shared_ptr<GameObject>& gameObject : _gameObjects)
	{
		if (gameObject->GetComponent<Camera>() != nullptr)
		{
			Vec3 cameraPos = gameObject->transform()->GetWorldPosition();
			return cameraPos;
		}
			
		
	}
	return Vec3();
}


