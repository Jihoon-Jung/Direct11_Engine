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

	// 물리 업데이트 빈도 제어 (예: 60FPS에서 20FPS로)
	static float accumulatedTime = 0.0f;
	accumulatedTime += TIME.GetDeltaTime();

	if (accumulatedTime >= 0.05f)  // 20Hz로 제한
	{
		CheckCollision();
		accumulatedTime = 0.0f;
	}
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
	}
	
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

	// 최적 축 선택
	int bestAxis = 0; // 기본값은 x축

	if (colliders.size() > 1)
	{
		float xSpread = CalculateSpread(colliders, 0);
		float ySpread = CalculateSpread(colliders, 1);
		float zSpread = CalculateSpread(colliders, 2);

		// 가장 분산이 큰 축 선택 (객체 간격이 가장 넓은 축)
		if (ySpread > xSpread && ySpread > zSpread)
			bestAxis = 1; // y축
		else if (zSpread > xSpread && zSpread > ySpread)
			bestAxis = 2; // z축
	}

	vector<pair<shared_ptr<GameObject>, shared_ptr<GameObject>>> potentialCollisions;

	// 선택된 최적 축으로 정렬
	SortAndSweep(colliders, bestAxis);

	// 스윕 과정 - 선택된 축을 기준으로 진행
	vector<shared_ptr<BaseCollider>> activeList;
	for (int i = 0; i < colliders.size(); i++)
	{
		float currentMax = GetMaxBound(colliders[i], bestAxis);

		for (int j = 0; j < activeList.size(); j++)
		{
			float activeMin = GetMinBound(activeList[j], bestAxis);

			if (activeMin > currentMax)
			{
				activeList.erase(activeList.begin() + j);
				j--;
			}
			else
			{
				potentialCollisions.push_back({
					activeList[j]->GetGameObject(),
					colliders[i]->GetGameObject()
					});
			}
		}

		activeList.push_back(colliders[i]);
	}

	// 추가 필터링 - 다른 두 축에서도 겹치는지 확인하여 후보 더 줄이기
	vector<pair<shared_ptr<GameObject>, shared_ptr<GameObject>>> filteredCollisions;
	for (const auto& pair : potentialCollisions)
	{
		shared_ptr<BaseCollider> colliderA = pair.first->GetComponent<BaseCollider>();
		shared_ptr<BaseCollider> colliderB = pair.second->GetComponent<BaseCollider>();

		// 다른 두 축에서도 겹치는지 확인
		bool overlapsOnAllAxes = true;
		for (int axis = 0; axis < 3; axis++)
		{
			if (axis == bestAxis)
				continue; // 이미 검사한 축은 건너뜀

			if (GetMaxBound(colliderA, axis) < GetMinBound(colliderB, axis) ||
				GetMinBound(colliderA, axis) > GetMaxBound(colliderB, axis))
			{
				overlapsOnAllAxes = false;
				break;
			}
		}

		if (overlapsOnAllAxes)
			filteredCollisions.push_back(pair);
	}

	// 최종 충돌 검사 수행
	for (const auto& pair : filteredCollisions)
	{
		shared_ptr<BaseCollider> colliderA = pair.first->GetComponent<BaseCollider>();
		shared_ptr<BaseCollider> colliderB = pair.second->GetComponent<BaseCollider>();

		if (colliderA->Intersects(colliderB))
		{
			
			char buffer[100];
			sprintf_s(buffer, "collision object : %ls, %ls\n", pair.first->GetName().c_str(), pair.second->GetName().c_str());
			OutputDebugStringA(buffer);

			/*RemoveGameObject(pair.first);
			RemoveGameObject(pair.second);*/
		}
	}
}

void Scene::SortAndSweep(vector<shared_ptr<BaseCollider>>& colliders, int axis)
{
	// 지정된 축을 기준으로 충돌체들을 정렬
	std::sort(colliders.begin(), colliders.end(), [axis](const shared_ptr<BaseCollider>& a, const shared_ptr<BaseCollider>& b) {
		float aMin = 0.0f;
		float bMin = 0.0f;

		// 각 충돌체 타입에 따라 해당 축의 최소값 구하기
		if (a->GetColliderType() == ColliderType::Box)
		{
			shared_ptr<BoxCollider> boxCollider = dynamic_pointer_cast<BoxCollider>(a);
			BoundingOrientedBox box = boxCollider->GetBoundingBox();

			// 중심 위치와 크기 값 가져오기
			Vec3 center = Vec3(box.Center.x, box.Center.y, box.Center.z);
			Vec3 extents = Vec3(box.Extents.x, box.Extents.y, box.Extents.z);

			// 해당 축의 최소값 계산
			if (axis == 0) aMin = center.x - extents.x;  // x축
			else if (axis == 1) aMin = center.y - extents.y;  // y축
			else aMin = center.z - extents.z;  // z축
		}
		else if (a->GetColliderType() == ColliderType::Sphere)
		{
			shared_ptr<SphereCollider> sphereCollider = dynamic_pointer_cast<SphereCollider>(a);
			BoundingSphere sphere = sphereCollider->GetBoundingSphere();

			// 중심 위치와 반지름 값 가져오기
			Vec3 center = Vec3(sphere.Center.x, sphere.Center.y, sphere.Center.z);
			float radius = sphere.Radius;

			// 해당 축의 최소값 계산
			if (axis == 0) aMin = center.x - radius;  // x축
			else if (axis == 1) aMin = center.y - radius;  // y축
			else aMin = center.z - radius;  // z축
		}

		// 두 번째 충돌체에 대해서도 동일한 계산
		if (b->GetColliderType() == ColliderType::Box)
		{
			shared_ptr<BoxCollider> boxCollider = dynamic_pointer_cast<BoxCollider>(b);
			BoundingOrientedBox box = boxCollider->GetBoundingBox();

			Vec3 center = Vec3(box.Center.x, box.Center.y, box.Center.z);
			Vec3 extents = Vec3(box.Extents.x, box.Extents.y, box.Extents.z);

			if (axis == 0) bMin = center.x - extents.x;
			else if (axis == 1) bMin = center.y - extents.y;
			else bMin = center.z - extents.z;
		}
		else if (b->GetColliderType() == ColliderType::Sphere)
		{
			shared_ptr<SphereCollider> sphereCollider = dynamic_pointer_cast<SphereCollider>(b);
			BoundingSphere sphere = sphereCollider->GetBoundingSphere();

			Vec3 center = Vec3(sphere.Center.x, sphere.Center.y, sphere.Center.z);
			float radius = sphere.Radius;

			if (axis == 0) bMin = center.x - radius;
			else if (axis == 1) bMin = center.y - radius;
			else bMin = center.z - radius;
		}

		// 최소값을 기준으로 정렬
		return aMin < bMin;
		});
}

// 충돌체의 최소 경계값 구하기
float Scene::GetMinBound(const shared_ptr<BaseCollider>& collider, int axis)
{
	if (collider->GetColliderType() == ColliderType::Box)
	{
		shared_ptr<BoxCollider> boxCollider = dynamic_pointer_cast<BoxCollider>(collider);
		BoundingOrientedBox box = boxCollider->GetBoundingBox();
		Vec3 center(box.Center.x, box.Center.y, box.Center.z);
		Vec3 extents(box.Extents.x, box.Extents.y, box.Extents.z);

		if (axis == 0) return center.x - extents.x;
		else if (axis == 1) return center.y - extents.y;
		else return center.z - extents.z;
	}
	else if (collider->GetColliderType() == ColliderType::Sphere)
	{
		shared_ptr<SphereCollider> sphereCollider = dynamic_pointer_cast<SphereCollider>(collider);
		BoundingSphere sphere = sphereCollider->GetBoundingSphere();
		Vec3 center(sphere.Center.x, sphere.Center.y, sphere.Center.z);
		float radius = sphere.Radius;

		if (axis == 0) return center.x - radius;
		else if (axis == 1) return center.y - radius;
		else return center.z - radius;
	}

	return 0.0f;
}

// 충돌체의 최대 경계값 구하기
float Scene::GetMaxBound(const shared_ptr<BaseCollider>& collider, int axis)
{
	if (collider->GetColliderType() == ColliderType::Box)
	{
		shared_ptr<BoxCollider> boxCollider = dynamic_pointer_cast<BoxCollider>(collider);
		BoundingOrientedBox box = boxCollider->GetBoundingBox();
		Vec3 center(box.Center.x, box.Center.y, box.Center.z);
		Vec3 extents(box.Extents.x, box.Extents.y, box.Extents.z);

		if (axis == 0) return center.x + extents.x;
		else if (axis == 1) return center.y + extents.y;
		else return center.z + extents.z;
	}
	else if (collider->GetColliderType() == ColliderType::Sphere)
	{
		shared_ptr<SphereCollider> sphereCollider = dynamic_pointer_cast<SphereCollider>(collider);
		BoundingSphere sphere = sphereCollider->GetBoundingSphere();
		Vec3 center(sphere.Center.x, sphere.Center.y, sphere.Center.z);
		float radius = sphere.Radius;

		if (axis == 0) return center.x + radius;
		else if (axis == 1) return center.y + radius;
		else return center.z + radius;
	}

	return 0.0f;
}

float Scene::CalculateSpread(const vector<shared_ptr<BaseCollider>>& colliders, int axis)
{
	if (colliders.empty())
		return 0.0f;

	float minValue = FLT_MAX;
	float maxValue = -FLT_MAX;

	// 해당 축에서 최소값과 최대값 찾기
	for (const auto& collider : colliders)
	{
		float min = GetMinBound(collider, axis);
		float max = GetMaxBound(collider, axis);

		if (min < minValue) minValue = min;
		if (max > maxValue) maxValue = max;
	}

	// 분산 계산 (최대-최소 범위)
	return maxValue - minValue;
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
				if (GUI.isSceneView())
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


