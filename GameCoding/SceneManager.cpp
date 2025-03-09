#include "pch.h"
#include "SceneManager.h"
#include "GameObject.h"
#include "MoveObject.h"
#include "EditorCamera.h"
#include "RenderPass.h"
#include "TestEvent.h"

void SceneManager::Init()
{
	if (_activeScene != nullptr)
	{
		_activeScene->Start();
	}
}

void SceneManager::Update()
{
	if (_activeScene != nullptr)
	{
		_activeScene->Update();
		_activeScene->LateUpdate();
	}
}

void SceneManager::LoadScene(wstring sceneName)
{
	tinyxml2::XMLDocument doc;
	string pathStr = "../SceneInfo.xml";

	if (doc.LoadFile(pathStr.c_str()) != tinyxml2::XML_SUCCESS) {
		tinyxml2::XMLElement* root = doc.NewElement("SceneInfo");
		doc.InsertFirstChild(root);
	}

	tinyxml2::XMLElement* root = doc.FirstChildElement("SceneInfo");
	if (root) {
		root->SetAttribute("sceneName", Utils::ToString(sceneName).c_str());
		doc.SaveFile(pathStr.c_str());
	}

	SCENE.Reset();
	RENDER.Reset();
	GUI.ResetSelectedObject();

	LoadSceneXML(sceneName);
	//LoadTestScene(sceneName);
	//LoadTestInstancingScene();

	SCENE.Init();
	RENDER.Init();
}
void SceneManager::LoadSceneXML(wstring sceneName)
{
	_activeScene = make_shared<Scene>();
	_activeScene->SetSceneName(sceneName);
	// XML 파일 로드
	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root)
		return;

	vector<shared_ptr<GameObject>> modelObjects;

	// 모든 GameObject 순회
	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		shared_ptr<GameObject> gameObj = make_shared<GameObject>();

		// 기본 정보 설정
		wstring name = Utils::ToWString(gameObjElem->Attribute("name"));
		gameObj->SetName(name);

		// Transform 컴포넌트 처리
		if (auto transformElem = gameObjElem->FirstChildElement("Transform"))
		{
			auto transform = make_shared<Transform>();
			Vec3 pos(
				transformElem->FloatAttribute("posX"),
				transformElem->FloatAttribute("posY"),
				transformElem->FloatAttribute("posZ")
			);
			Vec3 rot(
				transformElem->FloatAttribute("rotX"),
				transformElem->FloatAttribute("rotY"),
				transformElem->FloatAttribute("rotZ")
			);
			Vec3 scale(
				transformElem->FloatAttribute("scaleX"),
				transformElem->FloatAttribute("scaleY"),
				transformElem->FloatAttribute("scaleZ")
			);

			transform->SetLocalPosition(pos);
			// 직접 쿼터니온 생성
			float pitch = XMConvertToRadians(rot.x);
			float yaw = XMConvertToRadians(rot.y);
			float roll = XMConvertToRadians(rot.z);
			Quaternion qtRot = Quaternion::CreateFromYawPitchRoll(yaw, pitch, roll);

			transform->SetQTRotation(qtRot);
			transform->SetLocalScale(scale);
			gameObj->AddComponent(transform);
		}

		// Camera 컴포넌트 처리
		if (auto cameraElem = gameObjElem->FirstChildElement("Camera"))
		{
			auto camera = make_shared<Camera>();
			camera->SetProjectionType(static_cast<ProjectionType>(cameraElem->IntAttribute("projectionType")));
			gameObj->AddComponent(camera);
		}

		// Light 컴포넌트 처리
		if (auto lightElem = gameObjElem->FirstChildElement("Light"))
		{
			auto light = make_shared<Light>();
			LightDesc desc;
			desc.ambient = Vec4(
				lightElem->FloatAttribute("ambientR"),
				lightElem->FloatAttribute("ambientG"),
				lightElem->FloatAttribute("ambientB"),
				lightElem->FloatAttribute("ambientA")
			);
			desc.diffuse = Vec4(
				lightElem->FloatAttribute("diffuseR"),
				lightElem->FloatAttribute("diffuseG"),
				lightElem->FloatAttribute("diffuseB"),
				lightElem->FloatAttribute("diffuseA")
			);
			desc.specular = Vec4(
				lightElem->FloatAttribute("specularR"),
				lightElem->FloatAttribute("specularG"),
				lightElem->FloatAttribute("specularB"),
				lightElem->FloatAttribute("specularA")
			);
			light->SetLightDesc(desc);
			gameObj->AddComponent(light);
		}

		// MeshRenderer 컴포넌트 처리
		if (auto rendererElem = gameObjElem->FirstChildElement("MeshRenderer"))
		{
			auto meshRenderer = make_shared<MeshRenderer>();

			// RasterizerState 설정
			D3D11_FILL_MODE fillMode = static_cast<D3D11_FILL_MODE>(rendererElem->IntAttribute("fillMode"));
			D3D11_CULL_MODE cullMode = static_cast<D3D11_CULL_MODE>(rendererElem->IntAttribute("cullMode"));
			bool frontCounterClockwise = rendererElem->BoolAttribute("frontCounterClockwise");
			meshRenderer->SetRasterzierState(fillMode, cullMode, frontCounterClockwise);

			// EnvironmentMap 사용 여부 설정
			meshRenderer->SetUseEnvironmentMap(rendererElem->BoolAttribute("useEnvironmentMap"));

			const char* materialName = rendererElem->Attribute("material");
			const char* meshName = rendererElem->Attribute("mesh");
			const char* modelName = rendererElem->Attribute("model");

			if (modelName)
			{
				auto model = RESOURCE.GetResource<Model>(Utils::ToWString(modelName));
				meshRenderer->SetModel(model);
				meshRenderer->SetMaterial(model->GetMaterials()[0]); // Model의 Material 직접 사용
				modelObjects.push_back(gameObj);
			}
			else
			{
				const char* materialName = rendererElem->Attribute("material");
				const char* meshName = rendererElem->Attribute("mesh");

				if (materialName)
					meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(Utils::ToWString(materialName)));
				if (meshName)
					meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(Utils::ToWString(meshName)));
			}


			// RenderPass 처리
			for (auto passElem = rendererElem->FirstChildElement("RenderPass");
				passElem; passElem = passElem->NextSiblingElement("RenderPass"))
			{
				meshRenderer->AddRenderPass();
				auto pass = meshRenderer->GetRenderPasses().back();
				pass->SetPass(static_cast<Pass>(passElem->IntAttribute("pass")));
				pass->SetMeshRenderer(meshRenderer);
				pass->SetTransform(gameObj->transform());
				pass->SetDepthStencilStateType(static_cast<DSState>(passElem->IntAttribute("depthStencilState")));
			}

			gameObj->AddComponent(meshRenderer);
		}

		// Collider 컴포넌트 처리
		if (auto colliderElem = gameObjElem->FirstChildElement("BoxCollider"))
		{
			auto boxCollider = make_shared<BoxCollider>();
			Vec3 scale(
				colliderElem->FloatAttribute("scaleX", 1.0f),
				colliderElem->FloatAttribute("scaleY", 1.0f),
				colliderElem->FloatAttribute("scaleZ", 1.0f)
			);
			Vec3 center(
				colliderElem->FloatAttribute("centerX", 0.0f),
				colliderElem->FloatAttribute("centerY", 0.0f),
				colliderElem->FloatAttribute("centerZ", 0.0f)
			);
			boxCollider->SetScale(scale);
			boxCollider->SetCenter(center);
			gameObj->AddComponent(boxCollider);
		}
		else if (auto colliderElem = gameObjElem->FirstChildElement("SphereCollider"))
		{
			auto sphereCollider = make_shared<SphereCollider>();

			// Center 설정
			Vec3 center(
				colliderElem->FloatAttribute("centerX"),
				colliderElem->FloatAttribute("centerY"),
				colliderElem->FloatAttribute("centerZ")
			);
			sphereCollider->SetCenter(center);

			// Radius 설정
			float radius = colliderElem->FloatAttribute("radius");
			sphereCollider->SetRadius(radius);
			sphereCollider->SetScale(Vec3(radius, radius, radius));

			gameObj->AddComponent(sphereCollider);
		}
		if (auto particleElem = gameObjElem->FirstChildElement("ParticleSystem"))
		{
			auto particleSystem = make_shared<ParticleSystem>();
			float speed = particleElem->FloatAttribute("speed");
			bool endParticle = particleElem->BoolAttribute("endParticle");
			ParticleType type = static_cast<ParticleType>(particleElem->IntAttribute("type"));

			particleSystem->SetSpeed(speed);
			particleSystem->SetEndParticleFlag(endParticle);
			particleSystem->SetParticleTyle(type);
			gameObj->AddComponent(particleSystem);
		}
		// UIImage 컴포넌트 처리
		if (auto uiImageElem = gameObjElem->FirstChildElement("UIImage"))
		{
			auto uiImage = make_shared<UIImage>();
			float posX = uiImageElem->FloatAttribute("posX");
			float posY = uiImageElem->FloatAttribute("posY");
			float sizeX = uiImageElem->FloatAttribute("sizeX");
			float sizeY = uiImageElem->FloatAttribute("sizeY");
			float rectLeft = uiImageElem->FloatAttribute("rectLeft");
			float rectTop = uiImageElem->FloatAttribute("rectTop");
			float rectRight = uiImageElem->FloatAttribute("rectRight");
			float rectBottom = uiImageElem->FloatAttribute("rectBottom");
			RECT rect;
			rect.left = rectLeft;
			rect.top = rectTop;
			rect.right = rectRight;
			rect.bottom = rectBottom;
			uiImage->SetScreenTransformAndRect(Vec2(posX, posY), Vec2(sizeX, sizeY), rect);
			gameObj->AddComponent(uiImage);
			gameObj->SetObjectType(GameObjectType::UIObject);
		}

		// Button 컴포넌트 처리
		if (auto buttonElem = gameObjElem->FirstChildElement("Button"))
		{
			auto button = make_shared<Button>();

			// 위치와 크기 설정
			Vec2 pos(
				buttonElem->FloatAttribute("posX"),
				buttonElem->FloatAttribute("posY")
			);
			Vec2 size(
				buttonElem->FloatAttribute("sizeX"),
				buttonElem->FloatAttribute("sizeY")
			);
			float rectLeft = buttonElem->FloatAttribute("rectLeft");
			float rectTop = buttonElem->FloatAttribute("rectTop");
			float rectRight = buttonElem->FloatAttribute("rectRight");
			float rectBottom = buttonElem->FloatAttribute("rectBottom");

			RECT rect;
			rect.left = rectLeft;
			rect.top = rectTop;
			rect.right = rectRight;
			rect.bottom = rectBottom;

			button->SetScreenTransformAndRect(pos, size, rect);

			// 클릭 이벤트 함수 설정
			if (const char* functionKey = buttonElem->Attribute("onClickedFunctionKey"))
			{
				button->AddOnClickedEvent(functionKey);
			}

			gameObj->AddComponent(button);
		}
		vector<AnimatorEventLoadData> eventLoadDataList;  // 임시 저장용 구조체

		if (auto animatorElem = gameObjElem->FirstChildElement("Animator"))
		{
			auto animator = make_shared<Animator>();

			// Parameters 로드
			for (auto paramElem = animatorElem->FirstChildElement("Parameter");
				paramElem; paramElem = paramElem->NextSiblingElement("Parameter"))
			{
				string paramName = paramElem->Attribute("name");
				Parameter::Type paramType = static_cast<Parameter::Type>(paramElem->IntAttribute("type"));

				animator->AddParameter(paramName, paramType);

				// 파라미터 값 로드
				Parameter* param = animator->GetParameter(paramName);
				if (param)
				{
					switch (paramType)
					{
					case Parameter::Type::Bool:
						param->value.boolValue = paramElem->BoolAttribute("value");
						break;
					case Parameter::Type::Int:
						param->value.intValue = paramElem->IntAttribute("value");
						break;
					case Parameter::Type::Float:
						param->value.floatValue = paramElem->FloatAttribute("value");
						break;
					}
				}
			}
			// Clips 로드
			for (auto clipElem = animatorElem->FirstChildElement("Clip");
				clipElem; clipElem = clipElem->NextSiblingElement("Clip"))
			{
				string clipName = clipElem->Attribute("name");
				int animIndex = clipElem->IntAttribute("animIndex");
				bool isLoop = clipElem->BoolAttribute("isLoop");
				float speed = clipElem->FloatAttribute("speed", 1.0f);

				// 노드 위치 정보 로드
				float posX = clipElem->FloatAttribute("posX", 0.0f);  // 기본값 0
				float posY = clipElem->FloatAttribute("posY", 0.0f);  // 기본값 0

				// 클립 추가
				animator->AddClip(clipName, animIndex, isLoop);

				auto clip = animator->GetClip(clipName);
				// UI에 표시될 노드 위치 설정
				if (auto clip = animator->GetClip(clipName))
				{
					clip->speed = speed;
					clip->pos = ImVec2(posX, posY);
				}

				for (auto eventElem = clipElem->FirstChildElement("Event");
					eventElem; eventElem = eventElem->NextSiblingElement("Event"))
				{
					AnimatorEventLoadData eventData;
					eventData.gameObject = gameObj;
					eventData.animator = animator;
					eventData.clipName = clipName;
					eventData.time = eventElem->FloatAttribute("time");
					eventData.functionKey = eventElem->Attribute("function");
					eventLoadDataList.push_back(eventData);
				}
			}

			// Entry Clip 로드
			if (auto entryElem = animatorElem->FirstChildElement("EntryClip"))
			{
				string entryClipName = entryElem->Attribute("name");
				animator->SetEntryClip(entryClipName);
			}

			// Transitions 로드
			for (auto transitionElem = animatorElem->FirstChildElement("Transition");
				transitionElem; transitionElem = transitionElem->NextSiblingElement("Transition"))
			{
				string clipAName = transitionElem->Attribute("clipA");
				string clipBName = transitionElem->Attribute("clipB");

				animator->AddTransition(clipAName, clipBName);

				// clipA에서 clipB로 가는 특정 트랜지션을 찾아야 함
				auto clipA = animator->GetClip(clipAName);
				if (!clipA) continue;

				shared_ptr<Transition> targetTransition = nullptr;
				for (const auto& trans : clipA->transitions)
				{
					if (trans->clipB.lock() == animator->GetClip(clipBName))
					{
						targetTransition = trans;
						break;
					}
				}

				if (targetTransition)
				{
					bool flag = transitionElem->BoolAttribute("flag");
					bool hasCondition = transitionElem->BoolAttribute("hasCondition");
					bool hasExitTime = transitionElem->BoolAttribute("hasExitTime");
					float offset = transitionElem->FloatAttribute("transitionOffset");
					float duration = transitionElem->FloatAttribute("transitionDuration");
					float exitTime = transitionElem->FloatAttribute("ExitTime");

					animator->SetTransitionFlag(targetTransition, flag);
					targetTransition->hasCondition = hasCondition;
					targetTransition->hasExitTime = hasExitTime;
					animator->SetTransitionOffset(targetTransition, offset);
					animator->SetTransitionDuration(targetTransition, duration);
					animator->SetTransitionExitTime(targetTransition, exitTime);

					// Conditions 로드
					targetTransition->conditions.clear();
					for (auto conditionElem = transitionElem->FirstChildElement("Condition");
						conditionElem; conditionElem = conditionElem->NextSiblingElement("Condition"))
					{
						string paramName = conditionElem->Attribute("parameterName");
						Parameter::Type paramType = static_cast<Parameter::Type>(conditionElem->IntAttribute("parameterType"));
						Condition::CompareType compareType = static_cast<Condition::CompareType>(conditionElem->IntAttribute("compareType"));

						animator->AddCondition(targetTransition, paramName, paramType, compareType);

						if (!targetTransition->conditions.empty())
						{
							Condition& condition = targetTransition->conditions.back();
							switch (paramType)
							{
							case Parameter::Type::Bool:
								condition.value.boolValue = conditionElem->BoolAttribute("value");
								break;
							case Parameter::Type::Int:
								condition.value.intValue = conditionElem->IntAttribute("value");
								break;
							case Parameter::Type::Float:
								condition.value.floatValue = conditionElem->FloatAttribute("value");
								break;
							}
						}
					}
				}
			}

			animator->SetCurrentTransition();
			gameObj->AddComponent(animator);
		}
		// Script 컴포넌트 로드
		for (auto scriptElem = gameObjElem->FirstChildElement("Script");
			scriptElem; scriptElem = scriptElem->NextSiblingElement("Script"))
		{

			string type = scriptElem->Attribute("type");

			// EditorCamera 특별 처리
			if (type == "EditorCamera")
			{
				shared_ptr<EditorCamera> editorCamera = make_shared<EditorCamera>();
				gameObj->AddComponent(editorCamera);
				continue;
			}

			// "class " 접두사가 있다면 제거
			if (type.substr(0, 6) == "class ")
			{
				type = type.substr(6);
			}

			const auto& scripts = CF.GetRegisteredScripts();
			auto it = scripts.find(type);
			if (it != scripts.end())
			{
				auto script = it->second.createFunc();
				gameObj->AddComponent(script);
			}
		}
		_activeScene->AddGameObject(gameObj);

		for (const auto& eventData : eventLoadDataList)
		{
			auto animator = eventData.animator;
			auto availableFunctions = animator->GetAvailableFunctions();

			for (const auto& func : availableFunctions)
			{
				if (func.functionKey == eventData.functionKey)
				{
					AnimationEvent event;
					event.time = eventData.time;
					event.function = func;

					if (auto clip = animator->GetClip(eventData.clipName))
					{
						clip->events.push_back(event);
					}
					break;
				}
			}
		}
	}

	for (shared_ptr<GameObject> obj : modelObjects)
	{
		shared_ptr<Model> model = obj->GetComponent<MeshRenderer>()->GetModel();
		vector<shared_ptr<ModelBone>> bones = model->GetBones();

		map<int32, shared_ptr<GameObject>> boneObjects;

		for (shared_ptr<ModelBone> bone : bones)
		{
			shared_ptr<GameObject> boneObject = make_shared<GameObject>();
			boneObject->SetName(bone->name);
			shared_ptr<Transform> transform = make_shared<Transform>();

			Matrix localTransform = bone->transform;

			Vec3 position;
			Quaternion rotation;
			Vec3 scale;
			localTransform.Decompose(scale, rotation, position);

			transform->SetLocalPosition(position);
			transform->SetQTRotation(rotation);
			transform->SetLocalScale(scale);

			boneObject->AddComponent(transform);

			boneObjects[bone->index] = boneObject;

			boneObject->SetParent(obj);

			boneObject->SetBoneObjectFlag(true);
			boneObject->SetBoneParentObject(obj);
			boneObject->SetBoneIndex(bone->index);
			_activeScene->AddBoneGameObject(boneObject);
		}
	}

	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (const char* parentAttr = gameObjElem->Attribute("parent"))
		{
			wstring childName = Utils::ToWString(gameObjElem->Attribute("name"));
			wstring parentName = Utils::ToWString(parentAttr);

			auto childObj = SCENE.GetActiveScene()->Find(childName);
			shared_ptr<GameObject> parentObj = nullptr;

			if (const char* boneRootParentAttr = gameObjElem->Attribute("boneRootParent"))
			{
				wstring boneRootParentName = Utils::ToWString(boneRootParentAttr);
				auto boneRootParent = SCENE.GetActiveScene()->Find(boneRootParentName);

				if (boneRootParent)
				{
					vector<shared_ptr<GameObject>> children = boneRootParent->GetChildren();
					for (shared_ptr<GameObject> child : children)
					{
						if (child->GetName() == parentName)
						{
							parentObj = child;
							break;
						}
					}
				}
			}
			else
				parentObj = SCENE.GetActiveScene()->Find(parentName);

			if (childObj && parentObj)
			{
				childObj->SetParent(parentObj);
			}
		}
	}
}
void SceneManager::LoadTestScene(wstring sceneName)
{
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";

	// 기존 XML 파일이 있다면 삭제
	if (filesystem::exists(pathStr))
	{
		filesystem::remove(pathStr);
	}

	_activeScene = make_shared<Scene>();


	// EditorCamera
	SaveAndLoadGameObjectToXML(sceneName, L"EditorCamera",
		GP.centerPos + Vec3(-2.0f, 2.f, -10.0f));
	auto editorCameraComp = make_shared<Camera>();
	editorCameraComp->SetProjectionType(ProjectionType::Perspective);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"EditorCamera", editorCameraComp);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"EditorCamera", make_shared<EditorCamera>());

	 // MainCamera
	SaveAndLoadGameObjectToXML(sceneName, L"MainCamera",
		GP.centerPos + Vec3(-2.0f, 2.f, -10.0f));
	auto camera = make_shared<Camera>();
	camera->SetProjectionType(ProjectionType::Perspective);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"MainCamera", camera);
	//AddComponentToGameObjectAndSaveToXML(sceneName, L"MainCamera", make_shared<EditorCamera>());

	// UICamera
	SaveAndLoadGameObjectToXML(sceneName, L"UICamera", Vec3(0, 0, -3));
	auto uiCamera = make_shared<Camera>();
	uiCamera->SetProjectionType(ProjectionType::Orthographic);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"UICamera", uiCamera);

	// MainLight
	SaveAndLoadGameObjectToXML(sceneName, L"MainLight",
		Vec3(-28.0f, 37.0f, 112.0f), Vec3::Zero, Vec3(10.0f, 10.0f, 10.0f));
	AddComponentToGameObjectAndSaveToXML(sceneName, L"MainLight", make_shared<Light>());
	auto lightRenderer = make_shared<MeshRenderer>();
	lightRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Sphere"));
	lightRenderer->SetModel(nullptr);
	lightRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"SimpleMaterial"));
	lightRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	lightRenderer->AddRenderPass();
	lightRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
	lightRenderer->GetRenderPasses()[0]->SetMeshRenderer(lightRenderer);
	lightRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"MainLight")->transform());
	lightRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"MainLight", lightRenderer,
		L"SimpleMaterial", L"Sphere");

	// Environment Sphere
	SaveAndLoadGameObjectToXML(sceneName, L"Env_Sphere",
		Vec3(4.0f, 1.f, 122.f));
	auto envSphereRenderer = make_shared<MeshRenderer>();
	envSphereRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Sphere"));
	envSphereRenderer->SetModel(nullptr);
	envSphereRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"DefaultMaterial"));
	envSphereRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	envSphereRenderer->AddRenderPass();
	envSphereRenderer->GetRenderPasses()[0]->SetPass(Pass::ENVIRONMENTMAP_RENDER);
	envSphereRenderer->GetRenderPasses()[0]->SetMeshRenderer(envSphereRenderer);
	envSphereRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"Env_Sphere")->transform());
	envSphereRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	envSphereRenderer->SetUseEnvironmentMap(true);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"Env_Sphere", envSphereRenderer,
		L"DefaultMaterial", L"Sphere");
	auto sphereCollider = make_shared<SphereCollider>();
	sphereCollider->SetRadius(0.5f);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"Env_Sphere", sphereCollider);

	// Normal Sphere
	SaveAndLoadGameObjectToXML(sceneName, L"Sphere",
		Vec3(-6.5f, 5.6f, 122.f));
	auto sphereRenderer = make_shared<MeshRenderer>();
	sphereRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Sphere"));
	sphereRenderer->SetModel(nullptr);
	sphereRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"DefaultMaterial"));
	sphereRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	sphereRenderer->AddRenderPass();
	sphereRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
	sphereRenderer->GetRenderPasses()[0]->SetMeshRenderer(sphereRenderer);
	sphereRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"Sphere")->transform());
	sphereRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"Sphere", sphereRenderer,
		L"DefaultMaterial", L"Sphere");
	sphereCollider = make_shared<SphereCollider>();
	sphereCollider->SetRadius(1.0f);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"Sphere", sphereCollider);

	// Cube
	SaveAndLoadGameObjectToXML(sceneName, L"cube",
		Vec3(-6.5f, 0.6f, 122.f));
	auto cubeRenderer = make_shared<MeshRenderer>();
	cubeRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Cube"));
	cubeRenderer->SetModel(nullptr);
	cubeRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"DefaultMaterial"));
	cubeRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	cubeRenderer->AddRenderPass();
	cubeRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
	cubeRenderer->GetRenderPasses()[0]->SetMeshRenderer(cubeRenderer);
	cubeRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"cube")->transform());
	cubeRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"cube", cubeRenderer,
		L"DefaultMaterial", L"Cube");
	auto boxCollider = make_shared<BoxCollider>();
	boxCollider->SetScale(Vec3(1.0f, 1.0f, 1.0f));
	AddComponentToGameObjectAndSaveToXML(sceneName, L"cube", boxCollider);

	// Tessellation Cylinder
	SaveAndLoadGameObjectToXML(sceneName, L"tessellation_cube",
		Vec3(-7.0f, 1.5f, 120.0f));
	auto tessellationRenderer = make_shared<MeshRenderer>();
	tessellationRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Cylinder"));
	tessellationRenderer->SetModel(nullptr);
	tessellationRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"Tessellation_Material"));
	tessellationRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	tessellationRenderer->AddRenderPass();
	tessellationRenderer->GetRenderPasses()[0]->SetPass(Pass::TESSELLATION_RENDER);
	tessellationRenderer->GetRenderPasses()[0]->SetMeshRenderer(tessellationRenderer);
	tessellationRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"tessellation_cube")->transform());
	tessellationRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"tessellation_cube", tessellationRenderer,
		L"Tessellation_Material", L"Cylinder");
	boxCollider = make_shared<BoxCollider>();
	boxCollider->SetScale(Vec3(1.0f, 1.0f, 1.0f));
	AddComponentToGameObjectAndSaveToXML(sceneName, L"tessellation_cube", boxCollider);

	// Stencil Cube 1
	SaveAndLoadGameObjectToXML(sceneName, L"stencil_cube1",
		Vec3(-10.5f, 0.f, 120.f), Vec3::Zero, Vec3(1.0f));
	auto stencilRenderer1 = make_shared<MeshRenderer>();
	stencilRenderer1->SetMesh(RESOURCE.GetResource<Mesh>(L"Cube"));
	stencilRenderer1->SetModel(nullptr);
	stencilRenderer1->SetMaterial(RESOURCE.GetResource<Material>(L"DefaultMaterial"));
	stencilRenderer1->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	stencilRenderer1->AddRenderPass();
	stencilRenderer1->GetRenderPasses()[0]->SetPass(Pass::OUTLINE_RENDER);
	stencilRenderer1->GetRenderPasses()[0]->SetMeshRenderer(stencilRenderer1);
	stencilRenderer1->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"stencil_cube1")->transform());
	stencilRenderer1->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::CUSTOM1);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"stencil_cube1", stencilRenderer1,
		L"DefaultMaterial", L"Cube");

	//// Stencil Cube 2
	//SaveAndLoadGameObjectToXML(L"test_scene", L"stencil_cube2",
	//	Vec3(-10.5f, 0.f, 120.f), Vec3::Zero, Vec3(1.1f));
	//auto stencilRenderer2 = make_shared<MeshRenderer>();
	//stencilRenderer2->SetMesh(RESOURCE.GetResource<Mesh>(L"Cube"));
	//stencilRenderer2->SetModel(nullptr);
	//stencilRenderer2->SetMaterial(RESOURCE.GetResource<Material>(L"LightMaterial"));
	//stencilRenderer2->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	//stencilRenderer2->AddRenderPass();
	//stencilRenderer2->GetRenderPasses()[0]->SetPass(Pass::OUTLINE_RENDER);
	//stencilRenderer2->GetRenderPasses()[0]->SetMeshRenderer(stencilRenderer2);
	//stencilRenderer2->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"stencil_cube2")->transform());
	//stencilRenderer2->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::CUSTOM2);
	//AddComponentToGameObjectAndSaveToXML(L"test_scene", L"stencil_cube2", stencilRenderer2,
	//	L"LightMaterial", L"Cube");

	// Terrain
	SaveAndLoadGameObjectToXML(sceneName, L"Terrain_obj", Vec3::Zero);
	auto terrainRenderer = make_shared<MeshRenderer>();
	terrainRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Terrain"));
	terrainRenderer->SetModel(nullptr);
	terrainRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"TerrainMaterial"));
	terrainRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	terrainRenderer->AddRenderPass();
	terrainRenderer->GetRenderPasses()[0]->SetPass(Pass::TERRAIN_RENDER);
	terrainRenderer->GetRenderPasses()[0]->SetMeshRenderer(terrainRenderer);
	terrainRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"Terrain_obj")->transform());
	terrainRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"Terrain_obj", terrainRenderer,
		L"TerrainMaterial", L"Terrain");

	// UI Quad
	SaveAndLoadGameObjectToXML(sceneName, L"UI_Quad", Vec3::Zero);
	auto uiImage = make_shared<UIImage>();
	Vec2 imageSize = RESOURCE.GetResource<Material>(L"Debug_UI_Material")->GetTexture()->GetSize() * 0.09765625f;
	uiImage->SetTransformAndRect(Vec2(900, 200), imageSize);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"UI_Quad", uiImage);
	_activeScene->Find(L"UI_Quad")->transform()->SetLocalPosition(uiImage->GetNDCPosition());
	_activeScene->Find(L"UI_Quad")->transform()->SetLocalScale(uiImage->GetSize());
	UpdateGameObjectTransformInXML(sceneName, L"UI_Quad", uiImage->GetNDCPosition(), Vec3(0, 0, 0), uiImage->GetSize());
	auto uiRenderer = make_shared<MeshRenderer>();
	uiRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Quad"));
	uiRenderer->SetModel(nullptr);
	uiRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"Debug_UI_Material"));
	uiRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_NONE, false);
	uiRenderer->AddRenderPass();
	uiRenderer->GetRenderPasses()[0]->SetPass(Pass::UI_RENDER);
	uiRenderer->GetRenderPasses()[0]->SetMeshRenderer(uiRenderer);
	uiRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"UI_Quad")->transform());
	uiRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::UI);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"UI_Quad", uiRenderer,
		L"Debug_UI_Material", L"Quad");

	// UI Button
	SaveAndLoadGameObjectToXML(sceneName, L"UI_Button", Vec3::Zero);

	auto button = make_shared<Button>();
	string className = typeid(TestEvent).name();
	string functionKey = className + "::TestLog";
	button->AddOnClickedEvent(functionKey);

	imageSize = RESOURCE.GetResource<Material>(L"Debug_UI_Material")->GetTexture()->GetSize() * 0.09765625f;
	button->SetTransformAndRect(Vec2(50, 574), imageSize);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"UI_Button", button);
	_activeScene->Find(L"UI_Button")->transform()->SetLocalPosition(button->GetNDCPosition());
	_activeScene->Find(L"UI_Button")->transform()->SetLocalScale(button->GetSize());
	UpdateGameObjectTransformInXML(sceneName, L"UI_Button", button->GetNDCPosition(), Vec3(0, 0, 0), button->GetSize());

	auto buttonRenderer = make_shared<MeshRenderer>();
	buttonRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Quad"));
	buttonRenderer->SetModel(nullptr);
	buttonRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"Debug_UI_Material"));
	buttonRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_NONE, false);
	buttonRenderer->AddRenderPass();
	buttonRenderer->GetRenderPasses()[0]->SetPass(Pass::UI_RENDER);
	buttonRenderer->GetRenderPasses()[0]->SetMeshRenderer(buttonRenderer);
	buttonRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"UI_Button")->transform());
	buttonRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::UI);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"UI_Button", buttonRenderer,
		L"Debug_UI_Material", L"Quad");

	// House
	SaveAndLoadGameObjectToXML(sceneName, L"house",
		Vec3(0.f, 0.f, 130.f), Vec3::Zero, Vec3(0.005f));
	auto houseRenderer = make_shared<MeshRenderer>();
	houseRenderer->SetMesh(nullptr);
	houseRenderer->SetModel(RESOURCE.GetResource<Model>(L"HouseModel"));
	houseRenderer->SetMaterial(houseRenderer->GetModel()->GetMaterials()[0]);
	houseRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	houseRenderer->AddRenderPass();
	houseRenderer->GetRenderPasses()[0]->SetPass(Pass::STATIC_MESH_RENDER);
	houseRenderer->GetRenderPasses()[0]->SetMeshRenderer(houseRenderer);
	houseRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"house")->transform());
	houseRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"house", houseRenderer,
		L"", L"", L"HouseModel");
	boxCollider = make_shared<BoxCollider>();
	boxCollider->SetScale(Vec3(1.0f, 1.0f, 1.0f));
	AddComponentToGameObjectAndSaveToXML(sceneName, L"house", boxCollider);

	// Kachujin
	SaveAndLoadGameObjectToXML(sceneName, L"Kachujin_OBJ",
		GP.centerPos - Vec3(2.0f, 0.0f, 0.0f), Vec3::Zero, Vec3(0.01f));
	auto kachujinRenderer = make_shared<MeshRenderer>();
	kachujinRenderer->SetMesh(nullptr);
	kachujinRenderer->SetModel(RESOURCE.GetResource<Model>(L"Kachujin"));
	kachujinRenderer->SetMaterial(kachujinRenderer->GetModel()->GetMaterials()[0]);
	kachujinRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	kachujinRenderer->AddRenderPass();
	kachujinRenderer->GetRenderPasses()[0]->SetPass(Pass::ANIMATED_MESH_RENDER);
	kachujinRenderer->GetRenderPasses()[0]->SetMeshRenderer(kachujinRenderer);
	kachujinRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"Kachujin_OBJ")->transform());
	kachujinRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"Kachujin_OBJ", kachujinRenderer,
		L"", L"", L"Kachujin");
	boxCollider = make_shared<BoxCollider>();
	boxCollider->SetScale(Vec3(1.0f, 1.0f, 1.0f));
	AddComponentToGameObjectAndSaveToXML(sceneName, L"Kachujin_OBJ", boxCollider);
	


	// Dreyar
	SaveAndLoadGameObjectToXML(sceneName, L"Dreyar_OBJ",
		GP.centerPos, Vec3::Zero, Vec3(0.001f, 0.001f, 0.001f));
	auto dreyarRenderer = make_shared<MeshRenderer>();
	dreyarRenderer->SetMesh(nullptr);
	dreyarRenderer->SetModel(RESOURCE.GetResource<Model>(L"Dreyar"));
	dreyarRenderer->SetMaterial(dreyarRenderer->GetModel()->GetMaterials()[0]);
	dreyarRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	dreyarRenderer->AddRenderPass();
	dreyarRenderer->GetRenderPasses()[0]->SetPass(Pass::ANIMATED_MESH_RENDER);
	dreyarRenderer->GetRenderPasses()[0]->SetMeshRenderer(dreyarRenderer);
	dreyarRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"Dreyar_OBJ")->transform());
	dreyarRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"Dreyar_OBJ", dreyarRenderer,
		L"", L"", L"Dreyar");
	boxCollider = make_shared<BoxCollider>();
	boxCollider->SetScale(Vec3(1.0f, 1.0f, 1.0f));
	AddComponentToGameObjectAndSaveToXML(sceneName, L"Dreyar_OBJ", boxCollider);


	// Particle System
	SaveAndLoadGameObjectToXML(sceneName, L"FireParticle",
		GP.centerPos - Vec3(2.0f, 0.0f, 0.0f), Vec3::Zero, Vec3(1.0f));
	auto particleRenderer = make_shared<MeshRenderer>();
	particleRenderer->SetMesh(nullptr);
	particleRenderer->SetModel(nullptr);
	particleRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"ParticleMaterial"));
	particleRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	particleRenderer->AddRenderPass();
	particleRenderer->GetRenderPasses()[0]->SetPass(Pass::PARTICLE_RENDER);
	particleRenderer->GetRenderPasses()[0]->SetMeshRenderer(particleRenderer);
	particleRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"FireParticle")->transform());
	particleRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::CUSTOM3);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"FireParticle", particleRenderer, L"ParticleMaterial");
	auto particleSystem = make_shared<ParticleSystem>();
	AddComponentToGameObjectAndSaveToXML(sceneName, L"FireParticle", particleSystem);

	// SkyBox
	SaveAndLoadGameObjectToXML(sceneName, L"skyBox", Vec3::Zero);
	auto skyBoxRenderer = make_shared<MeshRenderer>();
	skyBoxRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Sphere"));
	skyBoxRenderer->SetModel(nullptr);
	skyBoxRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"GrassSkybox_Material"));
	skyBoxRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, true);
	skyBoxRenderer->AddRenderPass();
	skyBoxRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
	skyBoxRenderer->GetRenderPasses()[0]->SetMeshRenderer(skyBoxRenderer);
	skyBoxRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"skyBox")->transform());
	skyBoxRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"skyBox", skyBoxRenderer,
		L"GrassSkybox_Material", L"Sphere");
}

#include <random>
void SceneManager::LoadTestInstancingScene()
{
	_activeScene = make_shared<Scene>();

	 // MainCamera
	SaveAndLoadGameObjectToXML(L"test_scene", L"MainCamera",
		GP.centerPos + Vec3(-2.0f, 2.f, -10.0f));
	auto camera = make_shared<Camera>();
	camera->SetProjectionType(ProjectionType::Perspective);
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"MainCamera", camera);
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"MainCamera", make_shared<EditorCamera>());

	// UICamera
	SaveAndLoadGameObjectToXML(L"test_scene", L"UICamera", Vec3(0, 0, -3));
	auto uiCamera = make_shared<Camera>();
	uiCamera->SetProjectionType(ProjectionType::Orthographic);
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"UICamera", uiCamera);

	// MainLight
	SaveAndLoadGameObjectToXML(L"test_scene", L"MainLight",
		Vec3(-28.0f, 37.0f, 112.0f), Vec3::Zero, Vec3(10.0f, 10.0f, 10.0f));
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"MainLight", make_shared<Light>());
	auto lightRenderer = make_shared<MeshRenderer>();
	lightRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Sphere"));
	lightRenderer->SetModel(nullptr);
	lightRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"SimpleMaterial"));
	lightRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	lightRenderer->AddRenderPass();
	lightRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
	lightRenderer->GetRenderPasses()[0]->SetMeshRenderer(lightRenderer);
	lightRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"MainLight")->transform());
	lightRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"MainLight", lightRenderer,
		L"SimpleMaterial", L"Sphere");

	// Terrain
	SaveAndLoadGameObjectToXML(L"test_scene", L"Terrain_obj", Vec3::Zero);
	auto terrainRenderer = make_shared<MeshRenderer>();
	terrainRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Terrain"));
	terrainRenderer->SetModel(nullptr);
	terrainRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"TerrainMaterial"));
	terrainRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	terrainRenderer->AddRenderPass();
	terrainRenderer->GetRenderPasses()[0]->SetPass(Pass::TERRAIN_RENDER);
	terrainRenderer->GetRenderPasses()[0]->SetMeshRenderer(terrainRenderer);
	terrainRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"Terrain_obj")->transform());
	terrainRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"Terrain_obj", terrainRenderer,
		L"TerrainMaterial", L"Terrain");

	// Environment Sphere
	SaveAndLoadGameObjectToXML(L"test_scene", L"Env_Sphere",
		Vec3(4.0f, 1.f, 122.f));
	auto envSphereRenderer = make_shared<MeshRenderer>();
	envSphereRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Sphere"));
	envSphereRenderer->SetModel(nullptr);
	envSphereRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"DefaultMaterial"));
	envSphereRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	envSphereRenderer->AddRenderPass();
	envSphereRenderer->GetRenderPasses()[0]->SetPass(Pass::ENVIRONMENTMAP_RENDER);
	envSphereRenderer->GetRenderPasses()[0]->SetMeshRenderer(envSphereRenderer);
	envSphereRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"Env_Sphere")->transform());
	envSphereRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	envSphereRenderer->SetUseEnvironmentMap(true);
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"Env_Sphere", envSphereRenderer,
		L"DefaultMaterial", L"Sphere");
	auto sphereCollider = make_shared<SphereCollider>();
	sphereCollider->SetRadius(0.5f);
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"Env_Sphere", sphereCollider);

	// Normal Sphere
	int count = 0;
	for (int i = 0; i < 20; i++)
	{
		wstring baseName = L"Cube";
		wstring newName = baseName + to_wstring(count);
		SaveAndLoadGameObjectToXML(L"test_scene", newName,
			Vec3(rand() % 10, 0, rand() % 10));
		auto sphereRenderer = make_shared<MeshRenderer>();
		sphereRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Cube"));
		sphereRenderer->SetModel(nullptr);
		sphereRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"DefaultMaterial"));
		sphereRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		sphereRenderer->AddRenderPass();
		sphereRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
		sphereRenderer->GetRenderPasses()[0]->SetMeshRenderer(sphereRenderer);
		sphereRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(newName)->transform());
		sphereRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
		AddComponentToGameObjectAndSaveToXML(L"test_scene", newName, sphereRenderer,
			L"DefaultMaterial", L"Cube");
		auto sphereCollider = make_shared<SphereCollider>();
		sphereCollider->SetRadius(1.0f);
		AddComponentToGameObjectAndSaveToXML(L"test_scene", newName, sphereCollider);
		
		{
			if (i < 1)
			{
				wstring baseName = L"Dreyar";
				wstring newName = baseName + to_wstring(count);
				SaveAndLoadGameObjectToXML(L"test_scene", newName,
					Vec3(rand() % 10, 0, rand() % 10), Vec3(0, 0, 0), Vec3(0.001f, 0.001f, 0.001f));

				auto dreyarRenderer = make_shared<MeshRenderer>();
				dreyarRenderer->SetMesh(nullptr);
				dreyarRenderer->SetModel(RESOURCE.GetResource<Model>(L"Dreyar"));
				dreyarRenderer->SetMaterial(dreyarRenderer->GetModel()->GetMaterials()[0]);
				dreyarRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
				dreyarRenderer->AddRenderPass();
				dreyarRenderer->GetRenderPasses()[0]->SetPass(Pass::ANIMATED_MESH_RENDER);
				dreyarRenderer->GetRenderPasses()[0]->SetMeshRenderer(dreyarRenderer);
				dreyarRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(newName)->transform());
				dreyarRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
				AddComponentToGameObjectAndSaveToXML(L"test_scene", newName, dreyarRenderer,
					L"", L"", L"Dreyar");
				auto boxCollider = make_shared<BoxCollider>();
				boxCollider->SetScale(Vec3(443.388f, 1653.448f, 691.212f));
				boxCollider->SetCenter(Vec3(0.0f, 0.8f, 0.0f));
				AddComponentToGameObjectAndSaveToXML(L"test_scene", newName, boxCollider);

				auto animator = make_shared<Animator>();
				std::vector<int> indices = { 0, 1, 2 };

				// 랜덤 엔진 초기화
				std::random_device rd;
				std::mt19937 g(rd());

				// 리스트 셔플
				std::shuffle(indices.begin(), indices.end(), g);

				// 섞인 순서로 AddClip 호출
				animator->AddClip("Kick", indices[0], false);
				animator->AddClip("Dance", indices[1], false);
				animator->AddClip("Fall", indices[2], false);
				animator->SetEntryClip("Kick");
				animator->AddTransition("Kick", "Dance");
				animator->AddTransition("Dance", "Fall");
				animator->AddTransition("Fall", "Kick");
				AddComponentToGameObjectAndSaveToXML(L"test_scene", newName, animator);
			}
			
		}

		{
			wstring baseName = L"House";
			wstring newName = baseName + to_wstring(count);

			SaveAndLoadGameObjectToXML(L"test_scene", newName,
				Vec3(rand() % 10, 0, rand() % 10), Vec3::Zero, Vec3(0.0005f));
			auto houseRenderer = make_shared<MeshRenderer>();
			houseRenderer->SetMesh(nullptr);
			houseRenderer->SetModel(RESOURCE.GetResource<Model>(L"HouseModel"));
			houseRenderer->SetMaterial(houseRenderer->GetModel()->GetMaterials()[0]);
			houseRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
			houseRenderer->AddRenderPass();
			houseRenderer->GetRenderPasses()[0]->SetPass(Pass::STATIC_MESH_RENDER);
			houseRenderer->GetRenderPasses()[0]->SetMeshRenderer(houseRenderer);
			houseRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(newName)->transform());
			houseRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
			AddComponentToGameObjectAndSaveToXML(L"test_scene", newName, houseRenderer,
				L"", L"", L"HouseModel");
			auto boxCollider = make_shared<BoxCollider>();
			boxCollider->SetScale(Vec3(1.0f, 1.0f, 1.0f));
			AddComponentToGameObjectAndSaveToXML(L"test_scene", newName, boxCollider);
		}
		
		count++;
	}
}
void SceneManager::CreateNewScene(wstring sceneName)
{
	_isCreateNewScene = true;

	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";

	// 기존 XML 파일이 있다면 삭제
	if (filesystem::exists(pathStr))
	{
		filesystem::remove(pathStr);
	}


	// EditorCamera
	wstring path = L"Resource/Scene/" + sceneName + L".xml";
	Vec3 editorCameraPos = Vec3(0.0f, 5.0f, -10.0f);
	Vec3 editorCameraRotation = Vec3(22.0f, 0.0f, 0.0f);
	Vec3 editorCameraScale = Vec3(1.0f, 1.0f, 1.0f);
	SaveGameObjectToXML(path, L"EditorCamera", &editorCameraPos, &editorCameraRotation, &editorCameraScale, nullptr);
	auto editorCameraComp = make_shared<Camera>();
	editorCameraComp->SetProjectionType(ProjectionType::Perspective);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"EditorCamera", editorCameraComp);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"EditorCamera", make_shared<EditorCamera>());

	// MainCamera
	Vec3 mainCameraPos = Vec3(0.0f, 5.0f, -10.0f);
	Vec3 mainCameraRotation = Vec3(22.0f, 0.0f, 0.0f);
	Vec3 mainCameraScale = Vec3(1.0f, 1.0f, 1.0f);
	SaveGameObjectToXML(path, L"MainCamera", &mainCameraPos, &mainCameraRotation, &mainCameraScale, nullptr);
	auto camera = make_shared<Camera>();
	camera->SetProjectionType(ProjectionType::Perspective);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"MainCamera", camera);

	// UICamera
	Vec3 uiCameraPos = Vec3(0, 0, -3);
	Vec3 uiCameraRotation = Vec3::Zero;
	Vec3 uiCameraScale = Vec3(1.0f, 1.0f, 1.0f);
	SaveGameObjectToXML(path, L"UICamera", &uiCameraPos, &uiCameraRotation, &uiCameraScale, nullptr);
	auto uiCamera = make_shared<Camera>();
	uiCamera->SetProjectionType(ProjectionType::Orthographic);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"UICamera", uiCamera);

	// MainLight
	Vec3 mainLightPos = Vec3(-50.0f, 37.0f, 0.0f);
	Vec3 mainLightRotation = Vec3::Zero;
	Vec3 mainLightScale = Vec3(10.0f, 10.0f, 10.0f);
	SaveGameObjectToXML(path, L"MainLight", &mainLightPos, &mainLightRotation, &mainLightScale, nullptr);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"MainLight", make_shared<Light>());
	auto lightRenderer = make_shared<MeshRenderer>();
	lightRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Sphere"));
	lightRenderer->SetModel(nullptr);
	lightRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"SimpleMaterial"));
	lightRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	lightRenderer->AddRenderPass();
	lightRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
	lightRenderer->GetRenderPasses()[0]->SetMeshRenderer(lightRenderer);
	lightRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"MainLight")->transform());
	lightRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"MainLight", lightRenderer,
		L"SimpleMaterial", L"Sphere");


	// SkyBox
	Vec3 skyboxPosition = Vec3::Zero;
	Vec3 skyboxRotation = Vec3::Zero;
	Vec3 skyboxScale = Vec3::One;
	SaveGameObjectToXML(path, L"skyBox", &skyboxPosition, &skyboxRotation, &skyboxScale, nullptr);
	auto skyBoxRenderer = make_shared<MeshRenderer>();
	skyBoxRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Sphere"));
	skyBoxRenderer->SetModel(nullptr);
	skyBoxRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"GrassSkybox_Material"));
	skyBoxRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, true);
	skyBoxRenderer->AddRenderPass();
	skyBoxRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
	skyBoxRenderer->GetRenderPasses()[0]->SetMeshRenderer(skyBoxRenderer);
	skyBoxRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"skyBox")->transform());
	skyBoxRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(sceneName, L"skyBox", skyBoxRenderer,
		L"GrassSkybox_Material", L"Sphere");

	_isCreateNewScene = false;
}

shared_ptr<GameObject> SceneManager::LoadPrefabToScene(wstring prefab)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return nullptr;

	// XML 파일 로드
	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Prefab/" + Utils::ToString(prefab) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Prefab");
	if (!root)
		return nullptr;

	vector<shared_ptr<GameObject>> modelObjects;
	bool isModelObject = false;
	shared_ptr<GameObject> leaderObject;

	map<wstring, wstring> nameMapping;

	// 모든 GameObject 순회
	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		shared_ptr<GameObject> gameObj = make_shared<GameObject>();
		if (!leaderObject)
			leaderObject = gameObj;
		// 기본 정보 설정
		wstring name = Utils::ToWString(gameObjElem->Attribute("name"));

		int count = 1;
		wstring baseName = name;
		wstring newName = baseName;

		// 이미 존재하는 오브젝트 이름인지 확인
		while (_activeScene->Find(newName) != nullptr)
		{
			newName = baseName + to_wstring(count);
			count++;
		}
		gameObj->SetName(newName);

		nameMapping[name] = newName;

		// Transform 컴포넌트 처리
		if (auto transformElem = gameObjElem->FirstChildElement("Transform"))
		{
			auto transform = make_shared<Transform>();
			Vec3 pos(
				transformElem->FloatAttribute("posX"),
				transformElem->FloatAttribute("posY"),
				transformElem->FloatAttribute("posZ")
			);
			Vec3 rot(
				transformElem->FloatAttribute("rotX"),
				transformElem->FloatAttribute("rotY"),
				transformElem->FloatAttribute("rotZ")
			);
			Vec3 scale(
				transformElem->FloatAttribute("scaleX"),
				transformElem->FloatAttribute("scaleY"),
				transformElem->FloatAttribute("scaleZ")
			);

			transform->SetLocalPosition(pos);
			// 직접 쿼터니온 생성
			float pitch = XMConvertToRadians(rot.x);
			float yaw = XMConvertToRadians(rot.y);
			float roll = XMConvertToRadians(rot.z);
			Quaternion qtRot = Quaternion::CreateFromYawPitchRoll(yaw, pitch, roll);

			transform->SetQTRotation(qtRot);
			//transform->SetLocalRotation(rot);
			transform->SetLocalScale(scale);
			gameObj->AddComponent(transform);
		}

		// Camera 컴포넌트 처리
		if (auto cameraElem = gameObjElem->FirstChildElement("Camera"))
		{
			auto camera = make_shared<Camera>();
			camera->SetProjectionType(static_cast<ProjectionType>(cameraElem->IntAttribute("projectionType")));
			gameObj->AddComponent(camera);
		}

		// Light 컴포넌트 처리
		if (auto lightElem = gameObjElem->FirstChildElement("Light"))
		{
			auto light = make_shared<Light>();
			LightDesc desc;
			desc.ambient = Vec4(
				lightElem->FloatAttribute("ambientR"),
				lightElem->FloatAttribute("ambientG"),
				lightElem->FloatAttribute("ambientB"),
				lightElem->FloatAttribute("ambientA")
			);
			desc.diffuse = Vec4(
				lightElem->FloatAttribute("diffuseR"),
				lightElem->FloatAttribute("diffuseG"),
				lightElem->FloatAttribute("diffuseB"),
				lightElem->FloatAttribute("diffuseA")
			);
			desc.specular = Vec4(
				lightElem->FloatAttribute("specularR"),
				lightElem->FloatAttribute("specularG"),
				lightElem->FloatAttribute("specularB"),
				lightElem->FloatAttribute("specularA")
			);
			light->SetLightDesc(desc);
			gameObj->AddComponent(light);
		}

		// MeshRenderer 컴포넌트 처리
		if (auto rendererElem = gameObjElem->FirstChildElement("MeshRenderer"))
		{
			auto meshRenderer = make_shared<MeshRenderer>();

			// RasterizerState 설정
			D3D11_FILL_MODE fillMode = static_cast<D3D11_FILL_MODE>(rendererElem->IntAttribute("fillMode"));
			D3D11_CULL_MODE cullMode = static_cast<D3D11_CULL_MODE>(rendererElem->IntAttribute("cullMode"));
			bool frontCounterClockwise = rendererElem->BoolAttribute("frontCounterClockwise");
			meshRenderer->SetRasterzierState(fillMode, cullMode, frontCounterClockwise);

			// EnvironmentMap 사용 여부 설정
			meshRenderer->SetUseEnvironmentMap(rendererElem->BoolAttribute("useEnvironmentMap"));

			const char* materialName = rendererElem->Attribute("material");
			const char* meshName = rendererElem->Attribute("mesh");
			const char* modelName = rendererElem->Attribute("model");

			if (modelName)
			{
				auto model = RESOURCE.GetResource<Model>(Utils::ToWString(modelName));
				meshRenderer->SetModel(model);
				meshRenderer->SetMaterial(model->GetMaterials()[0]); // Model의 Material 직접 사용
				modelObjects.push_back(gameObj);
				isModelObject = true;
			}
			else
			{
				const char* materialName = rendererElem->Attribute("material");
				const char* meshName = rendererElem->Attribute("mesh");

				if (materialName)
					meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(Utils::ToWString(materialName)));
				if (meshName)
					meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(Utils::ToWString(meshName)));
			}


			// RenderPass 처리
			for (auto passElem = rendererElem->FirstChildElement("RenderPass");
				passElem; passElem = passElem->NextSiblingElement("RenderPass"))
			{
				meshRenderer->AddRenderPass();
				auto pass = meshRenderer->GetRenderPasses().back();
				pass->SetPass(static_cast<Pass>(passElem->IntAttribute("pass")));
				pass->SetMeshRenderer(meshRenderer);
				pass->SetTransform(gameObj->transform());
				pass->SetDepthStencilStateType(static_cast<DSState>(passElem->IntAttribute("depthStencilState")));
			}

			gameObj->AddComponent(meshRenderer);
		}

		// Collider 컴포넌트 처리
		if (auto colliderElem = gameObjElem->FirstChildElement("BoxCollider"))
		{
			auto boxCollider = make_shared<BoxCollider>();
			Vec3 scale(
				colliderElem->FloatAttribute("scaleX", 1.0f),
				colliderElem->FloatAttribute("scaleY", 1.0f),
				colliderElem->FloatAttribute("scaleZ", 1.0f)
			);
			Vec3 center(
				colliderElem->FloatAttribute("centerX", 0.0f),
				colliderElem->FloatAttribute("centerY", 0.0f),
				colliderElem->FloatAttribute("centerZ", 0.0f)
			);
			boxCollider->SetScale(scale);
			boxCollider->SetCenter(center);
			gameObj->AddComponent(boxCollider);
		}
		else if (auto colliderElem = gameObjElem->FirstChildElement("SphereCollider"))
		{
			auto sphereCollider = make_shared<SphereCollider>();

			// Center 설정
			Vec3 center(
				colliderElem->FloatAttribute("centerX"),
				colliderElem->FloatAttribute("centerY"),
				colliderElem->FloatAttribute("centerZ")
			);
			sphereCollider->SetCenter(center);

			// Radius 설정
			float radius = colliderElem->FloatAttribute("radius");
			sphereCollider->SetRadius(radius);
			sphereCollider->SetScale(Vec3(radius, radius, radius));

			gameObj->AddComponent(sphereCollider);
		}
		if (auto particleElem = gameObjElem->FirstChildElement("ParticleSystem"))
		{
			auto particleSystem = make_shared<ParticleSystem>();
			float speed = particleElem->FloatAttribute("speed");
			bool endParticle = particleElem->BoolAttribute("endParticle");
			ParticleType type = static_cast<ParticleType>(particleElem->IntAttribute("type"));

			particleSystem->SetSpeed(speed);
			particleSystem->SetEndParticleFlag(endParticle);
			particleSystem->SetParticleTyle(type);
			gameObj->AddComponent(particleSystem);
		}
		// UIImage 컴포넌트 처리
		if (auto uiImageElem = gameObjElem->FirstChildElement("UIImage"))
		{
			auto uiImage = make_shared<UIImage>();
			float posX = uiImageElem->FloatAttribute("posX");
			float posY = uiImageElem->FloatAttribute("posY");
			float sizeX = uiImageElem->FloatAttribute("sizeX");
			float sizeY = uiImageElem->FloatAttribute("sizeY");
			float rectLeft = uiImageElem->FloatAttribute("rectLeft");
			float rectTop = uiImageElem->FloatAttribute("rectTop");
			float rectRight = uiImageElem->FloatAttribute("rectRight");
			float rectBottom = uiImageElem->FloatAttribute("rectBottom");
			RECT rect;
			rect.left = rectLeft;
			rect.top = rectTop;
			rect.right = rectRight;
			rect.bottom = rectBottom;
			uiImage->SetScreenTransformAndRect(Vec2(posX, posY), Vec2(sizeX, sizeY), rect);
			gameObj->AddComponent(uiImage);
			gameObj->SetObjectType(GameObjectType::UIObject);
		}

		// Button 컴포넌트 처리
		if (auto buttonElem = gameObjElem->FirstChildElement("Button"))
		{
			auto button = make_shared<Button>();

			// 위치와 크기 설정
			Vec2 pos(
				buttonElem->FloatAttribute("posX"),
				buttonElem->FloatAttribute("posY")
			);
			Vec2 size(
				buttonElem->FloatAttribute("sizeX"),
				buttonElem->FloatAttribute("sizeY")
			);
			float rectLeft = buttonElem->FloatAttribute("rectLeft");
			float rectTop = buttonElem->FloatAttribute("rectTop");
			float rectRight = buttonElem->FloatAttribute("rectRight");
			float rectBottom = buttonElem->FloatAttribute("rectBottom");

			RECT rect;
			rect.left = rectLeft;
			rect.top = rectTop;
			rect.right = rectRight;
			rect.bottom = rectBottom;

			button->SetScreenTransformAndRect(pos, size, rect);

			// 클릭 이벤트 함수 설정
			if (const char* functionKey = buttonElem->Attribute("onClickedFunctionKey"))
			{
				button->AddOnClickedEvent(functionKey);
			}

			gameObj->AddComponent(button);
		}
		vector<AnimatorEventLoadData> eventLoadDataList;  // 임시 저장용 구조체

		if (auto animatorElem = gameObjElem->FirstChildElement("Animator"))
		{
			auto animator = make_shared<Animator>();

			// Parameters 로드
			for (auto paramElem = animatorElem->FirstChildElement("Parameter");
				paramElem; paramElem = paramElem->NextSiblingElement("Parameter"))
			{
				string paramName = paramElem->Attribute("name");
				Parameter::Type paramType = static_cast<Parameter::Type>(paramElem->IntAttribute("type"));

				animator->AddParameter(paramName, paramType);

				// 파라미터 값 로드
				Parameter* param = animator->GetParameter(paramName);
				if (param)
				{
					switch (paramType)
					{
					case Parameter::Type::Bool:
						param->value.boolValue = paramElem->BoolAttribute("value");
						break;
					case Parameter::Type::Int:
						param->value.intValue = paramElem->IntAttribute("value");
						break;
					case Parameter::Type::Float:
						param->value.floatValue = paramElem->FloatAttribute("value");
						break;
					}
				}
			}
			// Clips 로드
			for (auto clipElem = animatorElem->FirstChildElement("Clip");
				clipElem; clipElem = clipElem->NextSiblingElement("Clip"))
			{
				string clipName = clipElem->Attribute("name");
				int animIndex = clipElem->IntAttribute("animIndex");
				bool isLoop = clipElem->BoolAttribute("isLoop");
				float speed = clipElem->FloatAttribute("speed", 1.0f);

				// 노드 위치 정보 로드
				float posX = clipElem->FloatAttribute("posX", 0.0f);  // 기본값 0
				float posY = clipElem->FloatAttribute("posY", 0.0f);  // 기본값 0

				// 클립 추가
				animator->AddClip(clipName, animIndex, isLoop);

				auto clip = animator->GetClip(clipName);
				// UI에 표시될 노드 위치 설정
				if (auto clip = animator->GetClip(clipName))
				{
					clip->speed = speed;
					clip->pos = ImVec2(posX, posY);
				}

				for (auto eventElem = clipElem->FirstChildElement("Event");
					eventElem; eventElem = eventElem->NextSiblingElement("Event"))
				{
					AnimatorEventLoadData eventData;
					eventData.gameObject = gameObj;
					eventData.animator = animator;
					eventData.clipName = clipName;
					eventData.time = eventElem->FloatAttribute("time");
					eventData.functionKey = eventElem->Attribute("function");
					eventLoadDataList.push_back(eventData);
				}
			}

			// Entry Clip 로드
			if (auto entryElem = animatorElem->FirstChildElement("EntryClip"))
			{
				string entryClipName = entryElem->Attribute("name");
				animator->SetEntryClip(entryClipName);
			}

			// Transitions 로드
			for (auto transitionElem = animatorElem->FirstChildElement("Transition");
				transitionElem; transitionElem = transitionElem->NextSiblingElement("Transition"))
			{
				string clipAName = transitionElem->Attribute("clipA");
				string clipBName = transitionElem->Attribute("clipB");

				animator->AddTransition(clipAName, clipBName);

				// clipA에서 clipB로 가는 특정 트랜지션을 찾아야 함
				auto clipA = animator->GetClip(clipAName);
				if (!clipA) continue;

				shared_ptr<Transition> targetTransition = nullptr;
				for (const auto& trans : clipA->transitions)
				{
					if (trans->clipB.lock() == animator->GetClip(clipBName))
					{
						targetTransition = trans;
						break;
					}
				}

				if (targetTransition)
				{
					bool flag = transitionElem->BoolAttribute("flag");
					bool hasCondition = transitionElem->BoolAttribute("hasCondition");
					bool hasExitTime = transitionElem->BoolAttribute("hasExitTime");
					float offset = transitionElem->FloatAttribute("transitionOffset");
					float duration = transitionElem->FloatAttribute("transitionDuration");
					float exitTime = transitionElem->FloatAttribute("ExitTime");

					animator->SetTransitionFlag(targetTransition, flag);
					targetTransition->hasCondition = hasCondition;
					targetTransition->hasExitTime = hasExitTime;
					animator->SetTransitionOffset(targetTransition, offset);
					animator->SetTransitionDuration(targetTransition, duration);
					animator->SetTransitionExitTime(targetTransition, exitTime);

					// Conditions 로드
					targetTransition->conditions.clear();
					for (auto conditionElem = transitionElem->FirstChildElement("Condition");
						conditionElem; conditionElem = conditionElem->NextSiblingElement("Condition"))
					{
						string paramName = conditionElem->Attribute("parameterName");
						Parameter::Type paramType = static_cast<Parameter::Type>(conditionElem->IntAttribute("parameterType"));
						Condition::CompareType compareType = static_cast<Condition::CompareType>(conditionElem->IntAttribute("compareType"));

						animator->AddCondition(targetTransition, paramName, paramType, compareType);

						if (!targetTransition->conditions.empty())
						{
							Condition& condition = targetTransition->conditions.back();
							switch (paramType)
							{
							case Parameter::Type::Bool:
								condition.value.boolValue = conditionElem->BoolAttribute("value");
								break;
							case Parameter::Type::Int:
								condition.value.intValue = conditionElem->IntAttribute("value");
								break;
							case Parameter::Type::Float:
								condition.value.floatValue = conditionElem->FloatAttribute("value");
								break;
							}
						}
					}
				}
			}

			animator->SetCurrentTransition();
			gameObj->AddComponent(animator);
		}
		// Script 컴포넌트 로드
		for (auto scriptElem = gameObjElem->FirstChildElement("Script");
			scriptElem; scriptElem = scriptElem->NextSiblingElement("Script"))
		{

			string type = scriptElem->Attribute("type");

			// EditorCamera 특별 처리
			if (type == "EditorCamera")
			{
				shared_ptr<EditorCamera> editorCamera = make_shared<EditorCamera>();
				gameObj->AddComponent(editorCamera);
				continue;
			}

			// "class " 접두사가 있다면 제거
			if (type.substr(0, 6) == "class ")
			{
				type = type.substr(6);
			}

			const auto& scripts = CF.GetRegisteredScripts();
			auto it = scripts.find(type);
			if (it != scripts.end())
			{
				auto script = it->second.createFunc();
				gameObj->AddComponent(script);
			}
		}
		_activeScene->AddGameObject(gameObj);

		for (const auto& eventData : eventLoadDataList)
		{
			auto animator = eventData.animator;
			auto availableFunctions = animator->GetAvailableFunctions();

			for (const auto& func : availableFunctions)
			{
				if (func.functionKey == eventData.functionKey)
				{
					AnimationEvent event;
					event.time = eventData.time;
					event.function = func;

					if (auto clip = animator->GetClip(eventData.clipName))
					{
						clip->events.push_back(event);
					}
					break;
				}
			}
		}
	}

	for (shared_ptr<GameObject> obj : modelObjects)
	{
		shared_ptr<Model> model = obj->GetComponent<MeshRenderer>()->GetModel();
		vector<shared_ptr<ModelBone>> bones = model->GetBones();

		map<int32, shared_ptr<GameObject>> boneObjects;

		for (shared_ptr<ModelBone> bone : bones)
		{
			shared_ptr<GameObject> boneObject = make_shared<GameObject>();
			boneObject->SetName(bone->name);
			shared_ptr<Transform> transform = make_shared<Transform>();

			Matrix localTransform = bone->transform;

			Vec3 position;
			Quaternion rotation;
			Vec3 scale;
			localTransform.Decompose(scale, rotation, position);

			transform->SetLocalPosition(position);
			transform->SetQTRotation(rotation);
			transform->SetLocalScale(scale);

			boneObject->AddComponent(transform);

			boneObjects[bone->index] = boneObject;

			boneObject->SetParent(obj);

			boneObject->SetBoneObjectFlag(true);
			boneObject->SetBoneParentObject(obj);
			boneObject->SetBoneIndex(bone->index);
			_activeScene->AddBoneGameObject(boneObject);
		}

		shared_ptr<Animator> animator = obj->GetComponent<Animator>();
		if (animator)
		{
			animator->SetBoneObjects(boneObjects);

		}
	}

	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (const char* parentAttr = gameObjElem->Attribute("parent"))
		{
			wstring oldChildName = Utils::ToWString(gameObjElem->Attribute("name"));
			wstring oldParentName = Utils::ToWString(parentAttr);

			// nameMapping에서 새로운 이름 가져오기
			wstring newChildName = nameMapping[oldChildName];
			wstring newParentName = nameMapping[oldParentName];

			shared_ptr<GameObject> childObj = SCENE.GetActiveScene()->Find(newChildName);
			shared_ptr<GameObject> parentObj = SCENE.GetActiveScene()->Find(newParentName);

			if (!parentObj)
			{
				vector<shared_ptr<GameObject>> children = leaderObject->GetChildren();
				for (shared_ptr<GameObject> child : children)
				{
					if (child->GetName() == oldParentName)
					{
						parentObj = child;
						break;
					}
				}
			}
			if (childObj && parentObj)
			{
				childObj->SetParent(parentObj);
			}
		}
	}

	RENDER.GetRenderableObject();

	// 프리팹 내용을 현재 씬의 XML에 복사
	{
		// 현재 씬의 XML 로드
		tinyxml2::XMLDocument sceneDoc;
		string scenePathStr = "Resource/Scene/" + Utils::ToString(_activeScene->GetSceneName()) + ".xml";
		sceneDoc.LoadFile(scenePathStr.c_str());

		tinyxml2::XMLElement* sceneRoot = sceneDoc.FirstChildElement("Scene");
		if (sceneRoot)
		{
			// 프리팹의 모든 GameObject를 씬 XML에 복사
			for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
				gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
			{
				// 새로운 GameObject 요소 생성
				auto newGameObj = sceneDoc.NewElement("GameObject");

				// 모든 속성 복사
				const tinyxml2::XMLAttribute* attr = gameObjElem->FirstAttribute();
				while (attr)
				{
					if (string(attr->Name()) == "name")
					{
						// name 속성만 새 이름으로 변경
						wstring oldName = Utils::ToWString(attr->Value());
						newGameObj->SetAttribute("name", Utils::ToString(nameMapping[oldName]).c_str());
					}
					else if (string(attr->Name()) == "parent")
					{
						// parent 속성도 새 이름으로 변경
						wstring oldParentName = Utils::ToWString(attr->Value());
						if (nameMapping.find(oldParentName) != nameMapping.end() && !nameMapping[oldParentName].empty())
						{
							newGameObj->SetAttribute("parent", Utils::ToString(nameMapping[oldParentName]).c_str());
						}
						else
						{
							// nameMapping에 없거나 매핑된 이름이 빈 문자열인 경우 원래 parent 값을 사용
							newGameObj->SetAttribute("parent", attr->Value());
						}
					}
					else if (string(attr->Name()) == "boneRootParent")
					{
						// boneRootParent 속성도 새 이름으로 변경
						wstring oldBoneRootParentName = Utils::ToWString(attr->Value());
						if (nameMapping.find(oldBoneRootParentName) != nameMapping.end() && !nameMapping[oldBoneRootParentName].empty())
						{
							newGameObj->SetAttribute("boneRootParent", Utils::ToString(nameMapping[oldBoneRootParentName]).c_str());
						}
						else
						{
							// nameMapping에 없거나 매핑된 이름이 빈 문자열인 경우 원래 boneRootParent 값을 사용
							newGameObj->SetAttribute("boneRootParent", attr->Value());
						}
					}
					else
					{
						// 나머지 속성은 그대로 복사
						newGameObj->SetAttribute(attr->Name(), attr->Value());
					}
					attr = attr->Next();
				}

				// 모든 자식 요소 복사
				for (tinyxml2::XMLElement* child = gameObjElem->FirstChildElement();
					child; child = child->NextSiblingElement())
				{
					auto newChild = child->DeepClone(&sceneDoc)->ToElement();
					newGameObj->InsertEndChild(newChild);
				}

				sceneRoot->InsertEndChild(newGameObj);
			}

			sceneDoc.SaveFile(scenePathStr.c_str());
		}
	}
	return leaderObject;
}

shared_ptr<Scene> SceneManager::LoadPlayScene(wstring sceneName)
{
	shared_ptr<Scene> playScene = make_shared<Scene>();

	// XML 파일 로드
	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root)
		return nullptr;

	vector<shared_ptr<GameObject>> modelObjects;

	// 모든 GameObject 순회
	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		shared_ptr<GameObject> gameObj = make_shared<GameObject>();

		// 기본 정보 설정
		wstring name = Utils::ToWString(gameObjElem->Attribute("name"));
		gameObj->SetName(name);

		if (name == L"EditorCamera")
		{
			shared_ptr<EditorCamera> editorCameraComp = make_shared<EditorCamera>();
			gameObj->AddComponent(editorCameraComp);
		}

		// Transform 컴포넌트 처리
		if (auto transformElem = gameObjElem->FirstChildElement("Transform"))
		{
			auto transform = make_shared<Transform>();
			Vec3 pos(
				transformElem->FloatAttribute("posX"),
				transformElem->FloatAttribute("posY"),
				transformElem->FloatAttribute("posZ")
			);
			Vec3 rot(
				transformElem->FloatAttribute("rotX"),
				transformElem->FloatAttribute("rotY"),
				transformElem->FloatAttribute("rotZ")
			);
			Vec3 scale(
				transformElem->FloatAttribute("scaleX"),
				transformElem->FloatAttribute("scaleY"),
				transformElem->FloatAttribute("scaleZ")
			);

			transform->SetLocalPosition(pos);
			// 직접 쿼터니온 생성
			float pitch = XMConvertToRadians(rot.x);
			float yaw = XMConvertToRadians(rot.y);
			float roll = XMConvertToRadians(rot.z);
			Quaternion qtRot = Quaternion::CreateFromYawPitchRoll(yaw, pitch, roll);

			transform->SetQTRotation(qtRot);
			//transform->SetLocalRotation(rot);
			transform->SetLocalScale(scale);
			gameObj->AddComponent(transform);
		}

		// Camera 컴포넌트 처리
		if (auto cameraElem = gameObjElem->FirstChildElement("Camera"))
		{
			auto camera = make_shared<Camera>();
			camera->SetProjectionType(static_cast<ProjectionType>(cameraElem->IntAttribute("projectionType")));
			gameObj->AddComponent(camera);
		}

		// Light 컴포넌트 처리
		if (auto lightElem = gameObjElem->FirstChildElement("Light"))
		{
			auto light = make_shared<Light>();
			LightDesc desc;
			desc.ambient = Vec4(
				lightElem->FloatAttribute("ambientR"),
				lightElem->FloatAttribute("ambientG"),
				lightElem->FloatAttribute("ambientB"),
				lightElem->FloatAttribute("ambientA")
			);
			desc.diffuse = Vec4(
				lightElem->FloatAttribute("diffuseR"),
				lightElem->FloatAttribute("diffuseG"),
				lightElem->FloatAttribute("diffuseB"),
				lightElem->FloatAttribute("diffuseA")
			);
			desc.specular = Vec4(
				lightElem->FloatAttribute("specularR"),
				lightElem->FloatAttribute("specularG"),
				lightElem->FloatAttribute("specularB"),
				lightElem->FloatAttribute("specularA")
			);
			light->SetLightDesc(desc);
			gameObj->AddComponent(light);
		}

		// MeshRenderer 컴포넌트 처리
		if (auto rendererElem = gameObjElem->FirstChildElement("MeshRenderer"))
		{
			auto meshRenderer = make_shared<MeshRenderer>();

			// RasterizerState 설정
			D3D11_FILL_MODE fillMode = static_cast<D3D11_FILL_MODE>(rendererElem->IntAttribute("fillMode"));
			D3D11_CULL_MODE cullMode = static_cast<D3D11_CULL_MODE>(rendererElem->IntAttribute("cullMode"));
			bool frontCounterClockwise = rendererElem->BoolAttribute("frontCounterClockwise");
			meshRenderer->SetRasterzierState(fillMode, cullMode, frontCounterClockwise);

			// EnvironmentMap 사용 여부 설정
			meshRenderer->SetUseEnvironmentMap(rendererElem->BoolAttribute("useEnvironmentMap"));

			const char* materialName = rendererElem->Attribute("material");
			const char* meshName = rendererElem->Attribute("mesh");
			const char* modelName = rendererElem->Attribute("model");

			if (modelName)
			{
				auto model = RESOURCE.GetResource<Model>(Utils::ToWString(modelName));
				meshRenderer->SetModel(model);
				meshRenderer->SetMaterial(model->GetMaterials()[0]); // Model의 Material 직접 사용
				modelObjects.push_back(gameObj);
			}
			else
			{
				const char* materialName = rendererElem->Attribute("material");
				const char* meshName = rendererElem->Attribute("mesh");

				if (materialName)
					meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(Utils::ToWString(materialName)));
				if (meshName)
					meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(Utils::ToWString(meshName)));
			}


			// RenderPass 처리
			for (auto passElem = rendererElem->FirstChildElement("RenderPass");
				passElem; passElem = passElem->NextSiblingElement("RenderPass"))
			{
				meshRenderer->AddRenderPass();
				auto pass = meshRenderer->GetRenderPasses().back();
				pass->SetPass(static_cast<Pass>(passElem->IntAttribute("pass")));
				pass->SetMeshRenderer(meshRenderer);
				pass->SetTransform(gameObj->transform());
				pass->SetDepthStencilStateType(static_cast<DSState>(passElem->IntAttribute("depthStencilState")));
			}

			gameObj->AddComponent(meshRenderer);
		}

		// Collider 컴포넌트 처리
		if (auto colliderElem = gameObjElem->FirstChildElement("BoxCollider"))
		{
			auto boxCollider = make_shared<BoxCollider>();
			Vec3 scale(
				colliderElem->FloatAttribute("scaleX", 1.0f),
				colliderElem->FloatAttribute("scaleY", 1.0f),
				colliderElem->FloatAttribute("scaleZ", 1.0f)
			);
			Vec3 center(
				colliderElem->FloatAttribute("centerX", 0.0f),
				colliderElem->FloatAttribute("centerY", 0.0f),
				colliderElem->FloatAttribute("centerZ", 0.0f)
			);
			boxCollider->SetScale(scale);
			boxCollider->SetCenter(center);
			gameObj->AddComponent(boxCollider);
		}
		if (auto particleElem = gameObjElem->FirstChildElement("ParticleSystem"))
		{
			auto particleSystem = make_shared<ParticleSystem>();
			float speed = particleElem->FloatAttribute("speed");
			bool endParticle = particleElem->BoolAttribute("endParticle");
			ParticleType type = static_cast<ParticleType>(particleElem->IntAttribute("type"));

			particleSystem->SetSpeed(speed);
			particleSystem->SetEndParticleFlag(endParticle);
			particleSystem->SetParticleTyle(type);
			gameObj->AddComponent(particleSystem);
		}
		else if (auto colliderElem = gameObjElem->FirstChildElement("SphereCollider"))
		{
			auto sphereCollider = make_shared<SphereCollider>();

			// Center 설정
			Vec3 center(
				colliderElem->FloatAttribute("centerX"),
				colliderElem->FloatAttribute("centerY"),
				colliderElem->FloatAttribute("centerZ")
			);
			sphereCollider->SetCenter(center);

			// Radius 설정
			float radius = colliderElem->FloatAttribute("radius");
			sphereCollider->SetRadius(radius);
			sphereCollider->SetScale(Vec3(radius, radius, radius));

			gameObj->AddComponent(sphereCollider);
		}

		// UIImage 컴포넌트 처리
		if (auto uiImageElem = gameObjElem->FirstChildElement("UIImage"))
		{
			auto uiImage = make_shared<UIImage>();
			float posX = uiImageElem->FloatAttribute("posX");
			float posY = uiImageElem->FloatAttribute("posY");
			float sizeX = uiImageElem->FloatAttribute("sizeX");
			float sizeY = uiImageElem->FloatAttribute("sizeY");
			float rectLeft = uiImageElem->FloatAttribute("rectLeft");
			float rectTop = uiImageElem->FloatAttribute("rectTop");
			float rectRight = uiImageElem->FloatAttribute("rectRight");
			float rectBottom = uiImageElem->FloatAttribute("rectBottom");
			RECT rect;
			rect.left = rectLeft;
			rect.top = rectTop;
			rect.right = rectRight;
			rect.bottom = rectBottom;
			uiImage->SetScreenTransformAndRect(Vec2(posX, posY), Vec2(sizeX, sizeY), rect);
			gameObj->AddComponent(uiImage);
			gameObj->SetObjectType(GameObjectType::UIObject);
		}

		// Button 컴포넌트 처리
		if (auto buttonElem = gameObjElem->FirstChildElement("Button"))
		{
			auto button = make_shared<Button>();

			// 위치와 크기 설정
			Vec2 pos(
				buttonElem->FloatAttribute("posX"),
				buttonElem->FloatAttribute("posY")
			);
			Vec2 size(
				buttonElem->FloatAttribute("sizeX"),
				buttonElem->FloatAttribute("sizeY")
			);
			float rectLeft = buttonElem->FloatAttribute("rectLeft");
			float rectTop = buttonElem->FloatAttribute("rectTop");
			float rectRight = buttonElem->FloatAttribute("rectRight");
			float rectBottom = buttonElem->FloatAttribute("rectBottom");

			RECT rect;
			rect.left = rectLeft;
			rect.top = rectTop;
			rect.right = rectRight;
			rect.bottom = rectBottom;

			button->SetScreenTransformAndRect(pos, size, rect);

			// 클릭 이벤트 함수 설정
			if (const char* functionKey = buttonElem->Attribute("onClickedFunctionKey"))
			{
				button->AddOnClickedEvent(functionKey);
			}

			gameObj->AddComponent(button);
		}

		vector<AnimatorEventLoadData> eventLoadDataList;  // 임시 저장용 구조체

		if (auto animatorElem = gameObjElem->FirstChildElement("Animator"))
		{
			auto animator = make_shared<Animator>();

			// Parameters 로드
			for (auto paramElem = animatorElem->FirstChildElement("Parameter");
				paramElem; paramElem = paramElem->NextSiblingElement("Parameter"))
			{
				string paramName = paramElem->Attribute("name");
				Parameter::Type paramType = static_cast<Parameter::Type>(paramElem->IntAttribute("type"));

				animator->AddParameter(paramName, paramType);

				// 파라미터 값 로드
				Parameter* param = animator->GetParameter(paramName);
				if (param)
				{
					switch (paramType)
					{
					case Parameter::Type::Bool:
						param->value.boolValue = paramElem->BoolAttribute("value");
						break;
					case Parameter::Type::Int:
						param->value.intValue = paramElem->IntAttribute("value");
						break;
					case Parameter::Type::Float:
						param->value.floatValue = paramElem->FloatAttribute("value");
						break;
					}
				}
			}
			// Clips 로드
			for (auto clipElem = animatorElem->FirstChildElement("Clip");
				clipElem; clipElem = clipElem->NextSiblingElement("Clip"))
			{
				string clipName = clipElem->Attribute("name");
				int animIndex = clipElem->IntAttribute("animIndex");
				bool isLoop = clipElem->BoolAttribute("isLoop");
				float speed = clipElem->FloatAttribute("speed", 1.0f);

				// 노드 위치 정보 로드
				float posX = clipElem->FloatAttribute("posX", 0.0f);  // 기본값 0
				float posY = clipElem->FloatAttribute("posY", 0.0f);  // 기본값 0

				// 클립 추가
				animator->AddClip(clipName, animIndex, isLoop);

				auto clip = animator->GetClip(clipName);
				// UI에 표시될 노드 위치 설정
				if (auto clip = animator->GetClip(clipName))
				{
					clip->speed = speed;
					clip->pos = ImVec2(posX, posY);
				}

				for (auto eventElem = clipElem->FirstChildElement("Event");
					eventElem; eventElem = eventElem->NextSiblingElement("Event"))
				{
					AnimatorEventLoadData eventData;
					eventData.gameObject = gameObj;
					eventData.animator = animator;
					eventData.clipName = clipName;
					eventData.time = eventElem->FloatAttribute("time");
					eventData.functionKey = eventElem->Attribute("function");
					eventLoadDataList.push_back(eventData);
				}
			}

			// Entry Clip 로드
			if (auto entryElem = animatorElem->FirstChildElement("EntryClip"))
			{
				string entryClipName = entryElem->Attribute("name");
				animator->SetEntryClip(entryClipName);
			}

			// Transitions 로드
			for (auto transitionElem = animatorElem->FirstChildElement("Transition");
				transitionElem; transitionElem = transitionElem->NextSiblingElement("Transition"))
			{
				string clipAName = transitionElem->Attribute("clipA");
				string clipBName = transitionElem->Attribute("clipB");

				animator->AddTransition(clipAName, clipBName);

				// clipA에서 clipB로 가는 특정 트랜지션을 찾아야 함
				auto clipA = animator->GetClip(clipAName);
				if (!clipA) continue;

				shared_ptr<Transition> targetTransition = nullptr;
				for (const auto& trans : clipA->transitions)
				{
					if (trans->clipB.lock() == animator->GetClip(clipBName))
					{
						targetTransition = trans;
						break;
					}
				}

				if (targetTransition)
				{
					bool flag = transitionElem->BoolAttribute("flag");
					bool hasCondition = transitionElem->BoolAttribute("hasCondition");
					bool hasExitTime = transitionElem->BoolAttribute("hasExitTime");
					float offset = transitionElem->FloatAttribute("transitionOffset");
					float duration = transitionElem->FloatAttribute("transitionDuration");
					float exitTime = transitionElem->FloatAttribute("ExitTime");

					animator->SetTransitionFlag(targetTransition, flag);
					targetTransition->hasCondition = hasCondition;
					targetTransition->hasExitTime = hasExitTime;
					animator->SetTransitionOffset(targetTransition, offset);
					animator->SetTransitionDuration(targetTransition, duration);
					animator->SetTransitionExitTime(targetTransition, exitTime);

					// Conditions 로드
					targetTransition->conditions.clear();
					for (auto conditionElem = transitionElem->FirstChildElement("Condition");
						conditionElem; conditionElem = conditionElem->NextSiblingElement("Condition"))
					{
						string paramName = conditionElem->Attribute("parameterName");
						Parameter::Type paramType = static_cast<Parameter::Type>(conditionElem->IntAttribute("parameterType"));
						Condition::CompareType compareType = static_cast<Condition::CompareType>(conditionElem->IntAttribute("compareType"));

						animator->AddCondition(targetTransition, paramName, paramType, compareType);

						if (!targetTransition->conditions.empty())
						{
							Condition& condition = targetTransition->conditions.back();
							switch (paramType)
							{
							case Parameter::Type::Bool:
								condition.value.boolValue = conditionElem->BoolAttribute("value");
								break;
							case Parameter::Type::Int:
								condition.value.intValue = conditionElem->IntAttribute("value");
								break;
							case Parameter::Type::Float:
								condition.value.floatValue = conditionElem->FloatAttribute("value");
								break;
							}
						}
					}
				}
			}

			animator->SetCurrentTransition();
			gameObj->AddComponent(animator);
		}
		// Script 컴포넌트 로드
		for (auto scriptElem = gameObjElem->FirstChildElement("Script");
			scriptElem; scriptElem = scriptElem->NextSiblingElement("Script"))
		{
			string type = scriptElem->Attribute("type");
			// "class " 접두사가 있다면 제거
			if (type.substr(0, 6) == "class ")
			{
				type = type.substr(6);
			}

			const auto& scripts = CF.GetRegisteredScripts();
			auto it = scripts.find(type);
			if (it != scripts.end())
			{
				auto script = it->second.createFunc();
				gameObj->AddComponent(script);
			}
		}
		playScene->AddGameObject(gameObj);

		for (const auto& eventData : eventLoadDataList)
		{
			auto animator = eventData.animator;
			auto availableFunctions = animator->GetAvailableFunctions();

			for (const auto& func : availableFunctions)
			{
				if (func.functionKey == eventData.functionKey)
				{
					AnimationEvent event;
					event.time = eventData.time;
					event.function = func;

					if (auto clip = animator->GetClip(eventData.clipName))
					{
						clip->events.push_back(event);
					}
					break;
				}
			}
		}
	}

	for (shared_ptr<GameObject> obj : modelObjects)
	{
		shared_ptr<Model> model = obj->GetComponent<MeshRenderer>()->GetModel();
		vector<shared_ptr<ModelBone>> bones = model->GetBones();

		map<int32, shared_ptr<GameObject>> boneObjects;

		for (shared_ptr<ModelBone> bone : bones)
		{
			shared_ptr<GameObject> boneObject = make_shared<GameObject>();
			boneObject->SetName(bone->name);
			shared_ptr<Transform> transform = make_shared<Transform>();

			// World 공간의 본 행렬을 Local 공간으로 변환
			Matrix localTransform = bone->transform;

			Vec3 position;
			Quaternion rotation;
			Vec3 scale;
			localTransform.Decompose(scale, rotation, position);

			transform->SetLocalPosition(position);
			transform->SetQTRotation(rotation);
			transform->SetLocalScale(scale);

			boneObject->AddComponent(transform);
			boneObjects[bone->index] = boneObject;

			boneObject->SetParent(obj);
			boneObject->SetBoneObjectFlag(true);
			boneObject->SetBoneParentObject(obj);
			boneObject->SetBoneIndex(bone->index);
			playScene->AddBoneGameObject(boneObject);
		}

		shared_ptr<Animator> animator = obj->GetComponent<Animator>();
		if (animator)
		{
			animator->SetBoneObjects(boneObjects);

		}
			
	}

	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (const char* parentAttr = gameObjElem->Attribute("parent"))
		{
			wstring childName = Utils::ToWString(gameObjElem->Attribute("name"));
			wstring parentName = Utils::ToWString(parentAttr);

			auto childObj = playScene->Find(childName);
			shared_ptr<GameObject> parentObj = nullptr;

			if (const char* boneRootParentAttr = gameObjElem->Attribute("boneRootParent"))
			{
				wstring boneRootParentName = Utils::ToWString(boneRootParentAttr);
				auto boneRootParent = playScene->Find(boneRootParentName);

				if (boneRootParent)
				{
					vector<shared_ptr<GameObject>> children = boneRootParent->GetChildren();
					for (shared_ptr<GameObject> child : children)
					{
						if (child->GetName() == parentName)
						{
							parentObj = child;
							break;
						}
					}
				}
			}
			else
				parentObj = playScene->Find(parentName);

			if (childObj && parentObj)
			{
				childObj->SetParent(parentObj);
				if (parentObj->GetBoneObjectFlag())
				{
					parentObj->GetBoneParentObject().lock()->AddActiveBoneIndex(parentObj->GetBoneIndex());
					if (!childObj->GetBoneObjectFlag())
					{
						parentObj->SetHasNoneBoneChildrenFlag(true);
						childObj->SetNonBoneChildrenParent(parentObj);
					}

				}
			}
		}
	}

	return playScene;
}

void SceneManager::SaveAndLoadGameObjectToXML(const wstring& sceneName, const wstring& name, const Vec3& position, const Vec3& rotation, const Vec3& scale, shared_ptr<GameObject> parent)
{
	wstring path = L"Resource/Scene/" + sceneName + L".xml";

	// XML에 저장
	SaveGameObjectToXML(path, name, &position, &rotation, &scale, parent);


	// 실제 GameObject 생성 및 Scene에 추가
	shared_ptr<GameObject> gameObject = make_shared<GameObject>();
	shared_ptr<Transform> transform = make_shared<Transform>();

	transform->SetLocalPosition(position);
	transform->SetLocalRotation(rotation);
	transform->SetLocalScale(scale);
	gameObject->AddComponent(transform);

	if (parent != nullptr)
		gameObject->SetParent(parent);

	gameObject->SetName(name);

	_activeScene->AddGameObject(gameObject);
}

void SceneManager::AddComponentToGameObjectAndSaveToXML(const wstring& path, const wstring& name, const shared_ptr<Component>& component, const wstring& material, const wstring& mesh, const wstring& model)
{
	// 실제 GameObject에 컴포넌트 추가
	if (!_isCreateNewScene)
	{
		if (auto gameObject = _activeScene->Find(name))
		{
			gameObject->AddComponent(component);
		}
	}

	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	// 기존 XML 파일 로드
	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(path) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	tinyxml2::XMLElement* gameObj = nullptr;

	// GameObject 찾기
	for (tinyxml2::XMLElement* elem = root->FirstChildElement("GameObject");
		elem; elem = elem->NextSiblingElement("GameObject"))
	{
		if (string(elem->Attribute("name")) == Utils::ToString(name))
		{
			gameObj = elem;
			break;
		}
	}

	if (!gameObj)
		return;

	// 컴포넌트 타입별로 XML 요소 생성
	if (auto camera = dynamic_pointer_cast<Camera>(component))
	{
		tinyxml2::XMLElement* cameraElem = doc.NewElement("Camera");
		cameraElem->SetAttribute("projectionType", static_cast<int>(camera->GetProjectionType()));
		gameObj->InsertEndChild(cameraElem);
	}
	else if (auto light = dynamic_pointer_cast<Light>(component))
	{
		tinyxml2::XMLElement* lightElem = doc.NewElement("Light");
		LightDesc desc = light->GetLightDesc();

		// ambient
		lightElem->SetAttribute("ambientR", desc.ambient.x);
		lightElem->SetAttribute("ambientG", desc.ambient.y);
		lightElem->SetAttribute("ambientB", desc.ambient.z);
		lightElem->SetAttribute("ambientA", desc.ambient.w);

		// diffuse
		lightElem->SetAttribute("diffuseR", desc.diffuse.x);
		lightElem->SetAttribute("diffuseG", desc.diffuse.y);
		lightElem->SetAttribute("diffuseB", desc.diffuse.z);
		lightElem->SetAttribute("diffuseA", desc.diffuse.w);

		// specular
		lightElem->SetAttribute("specularR", desc.specular.x);
		lightElem->SetAttribute("specularG", desc.specular.y);
		lightElem->SetAttribute("specularB", desc.specular.z);
		lightElem->SetAttribute("specularA", desc.specular.w);

		// emissive
		lightElem->SetAttribute("emissiveR", desc.emissive.x);
		lightElem->SetAttribute("emissiveG", desc.emissive.y);
		lightElem->SetAttribute("emissiveB", desc.emissive.z);
		lightElem->SetAttribute("emissiveA", desc.emissive.w);

		gameObj->InsertEndChild(lightElem);
	}
	else if (auto meshRenderer = dynamic_pointer_cast<MeshRenderer>(component))
	{
		tinyxml2::XMLElement* rendererElem = doc.NewElement("MeshRenderer");

		// EnvironmentMap 사용 여부 저장
		rendererElem->SetAttribute("useEnvironmentMap", meshRenderer->GetUseEnvironmentMap());

		// RasterizerState 저장
		const auto& rasterState = meshRenderer->GetRasterzerStates();
		rendererElem->SetAttribute("fillMode", static_cast<int>(rasterState.fillMode));
		rendererElem->SetAttribute("cullMode", static_cast<int>(rasterState.cullMode));
		rendererElem->SetAttribute("frontCounterClockwise", rasterState.frontCounterClockwise);

		// Model이 있는 경우
		if (meshRenderer->GetModel())
		{
			rendererElem->SetAttribute("model", Utils::ToString(model).c_str());
			// Model의 Material은 따로 저장하지 않음 (Model에서 가져올 것이므로)
		}
		else
		{
			// 기존 방식대로 저장
			if (!material.empty())
				rendererElem->SetAttribute("material", Utils::ToString(material).c_str());
			if (!mesh.empty())
				rendererElem->SetAttribute("mesh", Utils::ToString(mesh).c_str());
		}

		// RenderPass 정보 저장
		for (const auto& pass : meshRenderer->GetRenderPasses())
		{
			tinyxml2::XMLElement* passElem = doc.NewElement("RenderPass");
			passElem->SetAttribute("pass", static_cast<int>(pass->GetPass()));
			passElem->SetAttribute("depthStencilState", static_cast<int>(pass->GetDepthStencilStateType()));
			rendererElem->InsertEndChild(passElem);
		}
		gameObj->InsertEndChild(rendererElem);
	}
	else if (auto boxCollider = dynamic_pointer_cast<BoxCollider>(component))
	{
		tinyxml2::XMLElement* colliderElem = doc.NewElement("BoxCollider");
		const Vec3& scale = boxCollider->GetScale();
		const Vec3& center = boxCollider->GetCenter();
		colliderElem->SetAttribute("scaleX", scale.x);
		colliderElem->SetAttribute("scaleY", scale.y);
		colliderElem->SetAttribute("scaleZ", scale.z);
		colliderElem->SetAttribute("centerX", center.x);
		colliderElem->SetAttribute("centerY", center.y);
		colliderElem->SetAttribute("centerZ", center.z);
		gameObj->InsertEndChild(colliderElem);
	}
	else if (auto sphereCollider = dynamic_pointer_cast<SphereCollider>(component))
	{
		tinyxml2::XMLElement* colliderElem = doc.NewElement("SphereCollider");
		const Vec3& center = sphereCollider->GetCenter();
		colliderElem->SetAttribute("radius", sphereCollider->GetRadius());
		colliderElem->SetAttribute("centerX", center.x);
		colliderElem->SetAttribute("centerY", center.y);
		colliderElem->SetAttribute("centerZ", center.z);
		gameObj->InsertEndChild(colliderElem);
	}
	else if (auto uiImage = dynamic_pointer_cast<UIImage>(component))
	{
		tinyxml2::XMLElement* uiElem = doc.NewElement("UIImage");
		const Vec3& pos = uiImage->GetNDCPosition();
		const Vec3& size = uiImage->GetSize();
		const RECT& rect = uiImage->GetRect();
		uiElem->SetAttribute("posX", pos.x);
		uiElem->SetAttribute("posY", pos.y);
		uiElem->SetAttribute("sizeX", size.x);
		uiElem->SetAttribute("sizeY", size.y);
		uiElem->SetAttribute("rectLeft", (float)rect.left);
		uiElem->SetAttribute("rectTop", (float)rect.top);
		uiElem->SetAttribute("rectRight", (float)rect.right);
		uiElem->SetAttribute("rectBottom", (float)rect.bottom);
		gameObj->InsertEndChild(uiElem);
	}
	else if (auto button = dynamic_pointer_cast<Button>(component))
	{
		tinyxml2::XMLElement* buttonElem = doc.NewElement("Button");
		const Vec3& pos = button->GetNDCPosition();
		const Vec3& size = button->GetSize();
		const RECT& rect = button->GetRect();
		buttonElem->SetAttribute("posX", pos.x);
		buttonElem->SetAttribute("posY", pos.y);
		buttonElem->SetAttribute("sizeX", size.x);
		buttonElem->SetAttribute("sizeY", size.y);
		buttonElem->SetAttribute("rectLeft", (float)rect.left);
		buttonElem->SetAttribute("rectTop", (float)rect.top);
		buttonElem->SetAttribute("rectRight", (float)rect.right);
		buttonElem->SetAttribute("rectBottom", (float)rect.bottom);
		buttonElem->SetAttribute("onClickedFunctionKey", button->GetOnClickedFuntionKey().c_str());
		gameObj->InsertEndChild(buttonElem);
	}
	else if (auto particleSystem = dynamic_pointer_cast<ParticleSystem>(component))
	{
		tinyxml2::XMLElement* particleElem = doc.NewElement("ParticleSystem");
		particleElem->SetAttribute("speed", particleSystem->GetSpeed());
		particleElem->SetAttribute("endParticle", particleSystem->GetEndParticleFlag());
		particleElem->SetAttribute("type", particleSystem->GetParticleType());
		gameObj->InsertEndChild(particleElem);
	}
	else if (auto animator = dynamic_pointer_cast<Animator>(component))
	{
		tinyxml2::XMLElement* animatorElem = doc.NewElement("Animator");

		// Parameters 정보 저장
		for (const auto& param : animator->_parameters)
		{
			tinyxml2::XMLElement* paramElem = doc.NewElement("Parameter");
			paramElem->SetAttribute("name", param.name.c_str());
			paramElem->SetAttribute("type", static_cast<int>(param.type));

			// 파라미터 값 저장
			switch (param.type)
			{
			case Parameter::Type::Bool:
				paramElem->SetAttribute("value", param.value.boolValue);
				break;
			case Parameter::Type::Int:
				paramElem->SetAttribute("value", param.value.intValue);
				break;
			case Parameter::Type::Float:
				paramElem->SetAttribute("value", param.value.floatValue);
				break;
			}

			animatorElem->InsertEndChild(paramElem);
		}
		// Entry clip 정보 저장
		if (animator->_entry)
		{
			tinyxml2::XMLElement* entryElem = doc.NewElement("EntryClip");
			entryElem->SetAttribute("name", animator->_entry->name.c_str());
			animatorElem->InsertEndChild(entryElem);
		}

		// Clips 정보 저장
		for (const auto& clipPair : animator->_clips)
		{
			tinyxml2::XMLElement* clipElem = doc.NewElement("Clip");
			const auto& clip = clipPair.second;

			clipElem->SetAttribute("name", clip->name.c_str());
			clipElem->SetAttribute("animIndex", clip->animIndex);
			clipElem->SetAttribute("isLoop", clip->isLoop);

			animatorElem->InsertEndChild(clipElem);
		}

		// Transitions 정보 저장
		for (const auto& transition : animator->_transitions)
		{
			tinyxml2::XMLElement* transitionElem = doc.NewElement("Transition");

			// clipA와 clipB의 이름을 저장
			if (auto clipA = transition->clipA.lock())
				transitionElem->SetAttribute("clipA", clipA->name.c_str());
			if (auto clipB = transition->clipB.lock())
				transitionElem->SetAttribute("clipB", clipB->name.c_str());

			transitionElem->SetAttribute("flag", transition->flag);
			transitionElem->SetAttribute("hasCondition", transition->hasCondition);
			transitionElem->SetAttribute("hasExitTime", transition->hasExitTime);
			transitionElem->SetAttribute("ExitTime", transition->exitTime);
			transitionElem->SetAttribute("transitionOffset", transition->transitionOffset);
			transitionElem->SetAttribute("transitionDuration", transition->transitionDuration);

			animatorElem->InsertEndChild(transitionElem);
		}

		gameObj->InsertEndChild(animatorElem);
	}
	else if (auto script = dynamic_pointer_cast<MonoBehaviour>(component))
	{
		tinyxml2::XMLElement* scriptElem = doc.NewElement("Script");
		string typeName = typeid(*script).name();

		// "class " 접두사 제거
		if (typeName.substr(0, 6) == "class ")
		{
			typeName = typeName.substr(6);
		}

		scriptElem->SetAttribute("type", typeName.c_str());
		gameObj->InsertEndChild(scriptElem);
	}
	// 다른 MonoBehaviour 스크립트들도 여기에 추가...
	doc.SaveFile(pathStr.c_str());
}

void SceneManager::RemoveComponentFromGameObjectInXML(const wstring& sceneName, const wstring& objectName, const shared_ptr<Component>& component)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	tinyxml2::XMLDocument doc;
	if (doc.LoadFile(pathStr.c_str()) != tinyxml2::XML_SUCCESS)
		return;

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root)
		return;

	// GameObject 찾기
	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (Utils::ToWString(gameObjElem->Attribute("name")) == objectName)
		{
			// 컴포넌트 타입에 따라 XML 노드 제거
			if (dynamic_pointer_cast<BoxCollider>(component))
			{
				if (auto colliderElem = gameObjElem->FirstChildElement("BoxCollider"))
				{
					gameObjElem->DeleteChild(colliderElem);
				}
			}
			else if (dynamic_pointer_cast<SphereCollider>(component))
			{
				if (auto colliderElem = gameObjElem->FirstChildElement("SphereCollider"))
				{
					gameObjElem->DeleteChild(colliderElem);
				}
			}
			else if (dynamic_pointer_cast<MeshRenderer>(component))
			{
				if (auto rendererElem = gameObjElem->FirstChildElement("MeshRenderer"))
				{
					gameObjElem->DeleteChild(rendererElem);
				}
			}
			else if (dynamic_pointer_cast<Button>(component))
			{
				if (auto buttonElem = gameObjElem->FirstChildElement("Button"))
				{
					gameObjElem->DeleteChild(buttonElem);
				}
			}
			else if (dynamic_pointer_cast<UIImage>(component))
			{
				if (auto uiImageElem = gameObjElem->FirstChildElement("UIImage"))
				{
					gameObjElem->DeleteChild(uiImageElem);
				}
			}
			else if (dynamic_pointer_cast<Animator>(component))
			{
				if (auto animatorElem = gameObjElem->FirstChildElement("Animator"))
				{
					gameObjElem->DeleteChild(animatorElem);
				}
			}
			else if (dynamic_pointer_cast<ParticleSystem>(component))
			{
				if (auto particleElem = gameObjElem->FirstChildElement("ParticleSystem"))
				{
					gameObjElem->DeleteChild(particleElem);
				}
			}
			else if (auto script = dynamic_pointer_cast<MonoBehaviour>(component))
			{
				// Script 엘리먼트들을 순회하면서 해당하는 타입의 스크립트 찾기
				for (auto scriptElem = gameObjElem->FirstChildElement("Script");
					scriptElem; scriptElem = scriptElem->NextSiblingElement("Script"))
				{
					string typeName = typeid(*script).name();
					if (typeName.substr(0, 6) == "class ")
					{
						typeName = typeName.substr(6);
					}

					if (string(scriptElem->Attribute("type")) == typeName)
					{
						gameObjElem->DeleteChild(scriptElem);
						doc.SaveFile(pathStr.c_str());
						break;
					}
				}
			}

			doc.SaveFile(pathStr.c_str());
			break;
		}
	}
}

void SceneManager::UpdateGameObjectTransformInXML(const wstring& sceneName, const wstring& objectName, const Vec3& position, const Vec3& rotation, const Vec3& scale)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root)
		return;

	// GameObject 찾기
	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (Utils::ToWString(gameObjElem->Attribute("name")) == objectName)
		{
			// Transform 엘리먼트 찾기
			if (auto transformElem = gameObjElem->FirstChildElement("Transform"))
			{
				// 위치 업데이트
				transformElem->SetAttribute("posX", position.x);
				transformElem->SetAttribute("posY", position.y);
				transformElem->SetAttribute("posZ", position.z);

				// 회전 업데이트
				transformElem->SetAttribute("rotX", rotation.x);
				transformElem->SetAttribute("rotY", rotation.y);
				transformElem->SetAttribute("rotZ", rotation.z);

				// 스케일 업데이트
				transformElem->SetAttribute("scaleX", scale.x);
				transformElem->SetAttribute("scaleY", scale.y);
				transformElem->SetAttribute("scaleZ", scale.z);

				doc.SaveFile(pathStr.c_str());
				
			}

			// UIImage 엘리먼트 찾기
			if (auto uiImageElem = gameObjElem->FirstChildElement("UIImage"))
			{
				// 위치 업데이트
				uiImageElem->SetAttribute("posX", position.x);
				uiImageElem->SetAttribute("posY", position.y);

				// 스케일 업데이트
				uiImageElem->SetAttribute("scaleX", scale.x);
				uiImageElem->SetAttribute("scaleY", scale.y);

				RECT rect = SCENE.GetActiveScene()->Find(objectName)->GetComponent<UIImage>()->GetRect();
				uiImageElem->SetAttribute("rectLeft", (float)rect.left);
				uiImageElem->SetAttribute("rectTop", (float)rect.top);
				uiImageElem->SetAttribute("rectRight", (float)rect.right);
				uiImageElem->SetAttribute("rectBottom", (float)rect.bottom);
				
				doc.SaveFile(pathStr.c_str());
			}

			// Button 엘리먼트 찾기
			if (auto buttonElem = gameObjElem->FirstChildElement("Button"))
			{
				// 위치 업데이트
				buttonElem->SetAttribute("posX", position.x);
				buttonElem->SetAttribute("posY", position.y);

				// 스케일 업데이트
				buttonElem->SetAttribute("scaleX", scale.x);
				buttonElem->SetAttribute("scaleY", scale.y);

				RECT rect = SCENE.GetActiveScene()->Find(objectName)->GetComponent<Button>()->GetRect();
				buttonElem->SetAttribute("rectLeft", (float)rect.left);
				buttonElem->SetAttribute("rectTop", (float)rect.top);
				buttonElem->SetAttribute("rectRight", (float)rect.right);
				buttonElem->SetAttribute("rectBottom", (float)rect.bottom);
				
				doc.SaveFile(pathStr.c_str());
			}
		}
	}
}


void SceneManager::SaveGameObjectToXML(const wstring& path, const wstring& name, const Vec3* position, const Vec3* rotation, const Vec3* scale, const shared_ptr<GameObject>& parent)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	tinyxml2::XMLDocument doc;

	// 기존 파일이 있으면 로드
	string pathStr = Utils::ToString(path);
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root)
	{
		root = doc.NewElement("Scene");
		doc.InsertFirstChild(root);
	}

	// 이미 존재하는 GameObject인지 확인
	tinyxml2::XMLElement* existingObj = root->FirstChildElement("GameObject");
	while (existingObj)
	{
		if (string(existingObj->Attribute("name")) == Utils::ToString(name))
		{
			root->DeleteChild(existingObj);
			break;
		}
		existingObj = existingObj->NextSiblingElement("GameObject");
	}

	// GameObject 요소 생성
	tinyxml2::XMLElement* gameObjectElem = doc.NewElement("GameObject");
	gameObjectElem->SetAttribute("name", Utils::ToString(name).c_str());

	if (parent)
		gameObjectElem->SetAttribute("parent", Utils::ToString(parent->GetName()).c_str());

	// Transform 정보 저장
	tinyxml2::XMLElement* transformElem = doc.NewElement("Transform");
	transformElem->SetAttribute("posX", position->x);
	transformElem->SetAttribute("posY", position->y);
	transformElem->SetAttribute("posZ", position->z);
	transformElem->SetAttribute("rotX", rotation->x);
	transformElem->SetAttribute("rotY", rotation->y);
	transformElem->SetAttribute("rotZ", rotation->z);
	transformElem->SetAttribute("scaleX", scale->x);
	transformElem->SetAttribute("scaleY", scale->y);
	transformElem->SetAttribute("scaleZ", scale->z);
	gameObjectElem->InsertEndChild(transformElem);

	root->InsertEndChild(gameObjectElem);
	doc.SaveFile(pathStr.c_str());
}

void SceneManager::UpdateGameObjectColliderInXML(const wstring& sceneName, const wstring& objectName,
	const Vec3& center, const Vec3& scale, bool isBoxCollider)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root) return;

	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (Utils::ToWString(gameObjElem->Attribute("name")) == objectName)
		{
			auto colliderElem = gameObjElem->FirstChildElement(isBoxCollider ? "BoxCollider" : "SphereCollider");
			if (colliderElem)
			{
				colliderElem->SetAttribute("centerX", center.x);
				colliderElem->SetAttribute("centerY", center.y);
				colliderElem->SetAttribute("centerZ", center.z);

				if (isBoxCollider)
				{
					colliderElem->SetAttribute("scaleX", scale.x);
					colliderElem->SetAttribute("scaleY", scale.y);
					colliderElem->SetAttribute("scaleZ", scale.z);
				}
				doc.SaveFile(pathStr.c_str());
			}
			break;
		}
	}
}

void SceneManager::UpdateGameObjectSphereColliderInXML(const wstring& sceneName, const wstring& objectName,
	const Vec3& center, float radius)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root) return;

	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (Utils::ToWString(gameObjElem->Attribute("name")) == objectName)
		{
			if (auto colliderElem = gameObjElem->FirstChildElement("SphereCollider"))
			{
				colliderElem->SetAttribute("centerX", center.x);
				colliderElem->SetAttribute("centerY", center.y);
				colliderElem->SetAttribute("centerZ", center.z);
				colliderElem->SetAttribute("radius", radius);
				doc.SaveFile(pathStr.c_str());
			}
			break;
		}
	}
}

void SceneManager::RemoveGameObjectFromXML(const wstring& sceneName, const wstring& objectName)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	tinyxml2::XMLDocument doc;
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root)
		return;

	// GameObject 찾기
	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (Utils::ToWString(gameObjElem->Attribute("name")) == objectName)
		{
			// GameObject를 찾으면 삭제
			root->DeleteChild(gameObjElem);
			doc.SaveFile(pathStr.c_str());
			break;
		}
	}
}

void SceneManager::UpdateGameObjectParentInXML(const wstring& sceneName, const wstring& objectName,
	const Vec3& localPosition, const Quaternion& localRotation, const Vec3& localScale,
	const wstring& parentName, const wstring& boneRootParentName)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root)
		return;

	// GameObject 찾기
	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (Utils::ToWString(gameObjElem->Attribute("name")) == objectName)
		{
			// 부모 설정
			if (parentName.empty())
				gameObjElem->DeleteAttribute("parent");
			else
				gameObjElem->SetAttribute("parent", Utils::ToString(parentName).c_str());

			// bone root parent 설정
			if (boneRootParentName.empty())
				gameObjElem->DeleteAttribute("boneRootParent");
			else
				gameObjElem->SetAttribute("boneRootParent", Utils::ToString(boneRootParentName).c_str());

			// Transform 업데이트
			if (auto transformElem = gameObjElem->FirstChildElement("Transform"))
			{
				// 위치 업데이트
				transformElem->SetAttribute("posX", localPosition.x);
				transformElem->SetAttribute("posY", localPosition.y);
				transformElem->SetAttribute("posZ", localPosition.z);

				// 회전 업데이트 (쿼터니온을 오일러 각도로 변환)
				Transform tempTransform;  // 임시 Transform 객체 생성
				Vec3 eulerAngles = tempTransform.ToEulerAngles(localRotation);
				transformElem->SetAttribute("rotX", XMConvertToDegrees(eulerAngles.x));
				transformElem->SetAttribute("rotY", XMConvertToDegrees(eulerAngles.y));
				transformElem->SetAttribute("rotZ", XMConvertToDegrees(eulerAngles.z));

				// 스케일 업데이트
				transformElem->SetAttribute("scaleX", localScale.x);
				transformElem->SetAttribute("scaleY", localScale.y);
				transformElem->SetAttribute("scaleZ", localScale.z);
			}

			doc.SaveFile(pathStr.c_str());
			break;
		}
	}
}

void SceneManager::UpdateMeshInXML(const wstring& sceneName, const wstring& objectName, const string& meshName)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root)
		return;

	// GameObject 찾기
	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (Utils::ToWString(gameObjElem->Attribute("name")) == objectName)
		{
			// MeshRenderer 엘리먼트 찾기
			if (auto rendererElem = gameObjElem->FirstChildElement("MeshRenderer"))
			{
				// Mesh 속성 업데이트
				rendererElem->SetAttribute("mesh", meshName.c_str());
				doc.SaveFile(pathStr.c_str());
			}
			break;
		}
	}
}

void SceneManager::UpdateMaterialInXML(const wstring& sceneName, const wstring& objectName, const string& materialName)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root)
		return;

	// GameObject 찾기
	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (Utils::ToWString(gameObjElem->Attribute("name")) == objectName)
		{
			// MeshRenderer 엘리먼트 찾기
			if (auto rendererElem = gameObjElem->FirstChildElement("MeshRenderer"))
			{
				// Material 속성 업데이트
				rendererElem->SetAttribute("material", materialName.c_str());
				doc.SaveFile(pathStr.c_str());
			}
			break;
		}
	}
}

void SceneManager::UpdateAnimatorClipInXML(const wstring& sceneName, const wstring& objectName,
	const string& clipName, float speed, bool isLoop)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root) return;

	// GameObject 찾기
	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (Utils::ToWString(gameObjElem->Attribute("name")) == objectName)
		{
			// Animator 컴포넌트 찾기
			if (auto animatorElem = gameObjElem->FirstChildElement("Animator"))
			{
				// Clip 찾기
				for (auto clipElem = animatorElem->FirstChildElement("Clip");
					clipElem; clipElem = clipElem->NextSiblingElement("Clip"))
				{
					if (string(clipElem->Attribute("name")) == clipName)
					{
						clipElem->SetAttribute("speed", speed);
						clipElem->SetAttribute("isLoop", isLoop);
						doc.SaveFile(pathStr.c_str());
						break;
					}
				}
			}
			break;
		}
	}
}

void SceneManager::UpdateAnimatorTransitionInXML(const wstring& sceneName, const wstring& objectName,
	const string& clipAName, const string& clipBName,
	float duration, float offset, float exitTime, bool hasExitTime)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root) return;

	// GameObject 찾기
	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (Utils::ToWString(gameObjElem->Attribute("name")) == objectName)
		{
			// Animator 컴포넌트 찾기
			if (auto animatorElem = gameObjElem->FirstChildElement("Animator"))
			{
				// Transition 찾기
				for (auto transitionElem = animatorElem->FirstChildElement("Transition");
					transitionElem; transitionElem = transitionElem->NextSiblingElement("Transition"))
				{
					if (string(transitionElem->Attribute("clipA")) == clipAName &&
						string(transitionElem->Attribute("clipB")) == clipBName)
					{
						transitionElem->SetAttribute("transitionDuration", duration);
						transitionElem->SetAttribute("transitionOffset", offset);
						transitionElem->SetAttribute("ExitTime", exitTime);
						transitionElem->SetAttribute("hasExitTime", hasExitTime);
						doc.SaveFile(pathStr.c_str());
						break;
					}
				}
			}
			break;
		}
	}
}

void SceneManager::AddAnimatorParameterToXML(const wstring& sceneName, const wstring& objectName, const string& paramName, Parameter::Type paramType)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root) return;

	// GameObject 찾기
	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (Utils::ToWString(gameObjElem->Attribute("name")) == objectName)
		{
			// Animator 컴포넌트 찾기
			if (auto animatorElem = gameObjElem->FirstChildElement("Animator"))
			{
				// 새로운 Parameter 추가
				tinyxml2::XMLElement* paramElem = doc.NewElement("Parameter");
				paramElem->SetAttribute("name", paramName.c_str());
				paramElem->SetAttribute("type", static_cast<int>(paramType));

				// 기본값 설정
				switch (paramType)
				{
				case Parameter::Type::Bool:
					paramElem->SetAttribute("value", false);
					break;
				case Parameter::Type::Int:
					paramElem->SetAttribute("value", 0);
					break;
				case Parameter::Type::Float:
					paramElem->SetAttribute("value", 0.0f);
					break;
				}

				animatorElem->InsertFirstChild(paramElem);
				doc.SaveFile(pathStr.c_str());
				break;
			}
			break;
		}
	}
}

void SceneManager::UpdateAnimatorParameterInXML(const wstring& sceneName, const wstring& objectName,
	const string& paramName, const Parameter& param)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root) return;

	// GameObject 찾기
	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (Utils::ToWString(gameObjElem->Attribute("name")) == objectName)
		{
			// Animator 컴포넌트 찾기
			if (auto animatorElem = gameObjElem->FirstChildElement("Animator"))
			{
				// Parameter 찾기
				for (auto paramElem = animatorElem->FirstChildElement("Parameter");
					paramElem; paramElem = paramElem->NextSiblingElement("Parameter"))
				{
					if (string(paramElem->Attribute("name")) == paramName)
					{
						// 값 업데이트
						switch (param.type)
						{
						case Parameter::Type::Bool:
							paramElem->SetAttribute("value", param.value.boolValue);
							break;
						case Parameter::Type::Int:
							paramElem->SetAttribute("value", param.value.intValue);
							break;
						case Parameter::Type::Float:
							paramElem->SetAttribute("value", param.value.floatValue);
							break;
						}
						doc.SaveFile(pathStr.c_str());
						break;
					}
				}
			}
			break;
		}
	}
}

shared_ptr<GameObject> SceneManager::CreateCubeToScene(const wstring& sceneName)
{
	// 새로운 오브젝트 생성 시 기본 이름 설정
	int count = 1;
	wstring baseName = L"cube";
	wstring newName = baseName;

	// 이미 존재하는 오브젝트 이름인지 확인
	while (_activeScene->Find(newName) != nullptr)
	{
		newName = baseName + to_wstring(count);
		count++;
	}

	SaveAndLoadGameObjectToXML(sceneName, newName,
		Vec3(0.0f, 0.0f, 0.0f));
	auto cubeRenderer = make_shared<MeshRenderer>();
	cubeRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Cube"));
	cubeRenderer->SetModel(nullptr);
	cubeRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"SolidWhiteMaterial"));
	cubeRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	cubeRenderer->AddRenderPass();
	cubeRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
	cubeRenderer->GetRenderPasses()[0]->SetMeshRenderer(cubeRenderer);
	cubeRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(newName)->transform());
	cubeRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(sceneName, newName, cubeRenderer,
		L"SolidWhiteMaterial", L"Cube");
	auto boxCollider = make_shared<BoxCollider>();
	boxCollider->SetScale(Vec3(1.0f, 1.0f, 1.0f));
	AddComponentToGameObjectAndSaveToXML(sceneName, newName, boxCollider);

	RENDER.GetRenderableObject();

	return _activeScene->Find(newName);
}

shared_ptr<GameObject> SceneManager::CreateSphereToScene(const wstring& sceneName)
{
	// 새로운 오브젝트 생성 시 기본 이름 설정
	int count = 1;
	wstring baseName = L"sphere";
	wstring newName = baseName;

	// 이미 존재하는 오브젝트 이름인지 확인
	while (_activeScene->Find(newName) != nullptr)
	{
		newName = baseName + to_wstring(count);
		count++;
	}

	SaveAndLoadGameObjectToXML(sceneName, newName,
		Vec3(0.0f, 0.0f, 0.0f));
	auto sphereRenderer = make_shared<MeshRenderer>();
	sphereRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Sphere"));
	sphereRenderer->SetModel(nullptr);
	sphereRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"SolidWhiteMaterial"));
	sphereRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	sphereRenderer->AddRenderPass();
	sphereRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
	sphereRenderer->GetRenderPasses()[0]->SetMeshRenderer(sphereRenderer);
	sphereRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(newName)->transform());
	sphereRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(sceneName, newName, sphereRenderer,
		L"SolidWhiteMaterial", L"Sphere");
	auto sphereCollider = make_shared<SphereCollider>();
	sphereCollider->SetRadius(0.5f);
	AddComponentToGameObjectAndSaveToXML(sceneName, newName, sphereCollider);

	RENDER.GetRenderableObject();

	return _activeScene->Find(newName);
}

shared_ptr<GameObject> SceneManager::CreateCylinderToScene(const wstring& sceneName)
{
	// 새로운 오브젝트 생성 시 기본 이름 설정
	int count = 1;
	wstring baseName = L"cylinder";
	wstring newName = baseName;

	// 이미 존재하는 오브젝트 이름인지 확인
	while (_activeScene->Find(newName) != nullptr)
	{
		newName = baseName + to_wstring(count);
		count++;
	}

	SaveAndLoadGameObjectToXML(sceneName, newName,
		Vec3(0.0f, 0.0f, 0.0f));
	auto cylinderRenderer = make_shared<MeshRenderer>();
	cylinderRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Cylinder"));
	cylinderRenderer->SetModel(nullptr);
	cylinderRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"SolidWhiteMaterial"));
	cylinderRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	cylinderRenderer->AddRenderPass();
	cylinderRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
	cylinderRenderer->GetRenderPasses()[0]->SetMeshRenderer(cylinderRenderer);
	cylinderRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(newName)->transform());
	cylinderRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(sceneName, newName, cylinderRenderer,
		L"SolidWhiteMaterial", L"Cylinder");
	auto boxCollider = make_shared<BoxCollider>();
	boxCollider->SetScale(Vec3(0.866f, 3.0f, 1.0f));
	AddComponentToGameObjectAndSaveToXML(sceneName, newName, boxCollider);

	RENDER.GetRenderableObject();

	return _activeScene->Find(newName);
}

shared_ptr<GameObject> SceneManager::CreateQuadToScene(const wstring& sceneName)
{
	// 새로운 오브젝트 생성 시 기본 이름 설정
	int count = 1;
	wstring baseName = L"Quad";
	wstring newName = baseName;

	// 이미 존재하는 오브젝트 이름인지 확인
	while (_activeScene->Find(newName) != nullptr)
	{
		newName = baseName + to_wstring(count);
		count++;
	}

	SaveAndLoadGameObjectToXML(sceneName, newName,
		Vec3(0.0f, 0.0f, 0.0f));
	auto cylinderRenderer = make_shared<MeshRenderer>();
	cylinderRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Quad"));
	cylinderRenderer->SetModel(nullptr);
	cylinderRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"SolidWhiteMaterial"));
	cylinderRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_NONE, false);
	cylinderRenderer->AddRenderPass();
	cylinderRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
	cylinderRenderer->GetRenderPasses()[0]->SetMeshRenderer(cylinderRenderer);
	cylinderRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(newName)->transform());
	cylinderRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(sceneName, newName, cylinderRenderer,
		L"SolidWhiteMaterial", L"Quad");
	auto boxCollider = make_shared<BoxCollider>();
	boxCollider->SetScale(Vec3(2.0f, 2.0f, 0.001f));
	AddComponentToGameObjectAndSaveToXML(sceneName, newName, boxCollider);

	RENDER.GetRenderableObject();

	return _activeScene->Find(newName);
}

shared_ptr<GameObject> SceneManager::CreateGridToScene(const wstring& sceneName)
{
	// 새로운 오브젝트 생성 시 기본 이름 설정
	int count = 1;
	wstring baseName = L"Grid";
	wstring newName = baseName;

	// 이미 존재하는 오브젝트 이름인지 확인
	while (_activeScene->Find(newName) != nullptr)
	{
		newName = baseName + to_wstring(count);
		count++;
	}

	SaveAndLoadGameObjectToXML(sceneName, newName,
		Vec3(0.0f, 0.0f, 0.0f));
	auto cylinderRenderer = make_shared<MeshRenderer>();
	cylinderRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Grid"));
	cylinderRenderer->SetModel(nullptr);
	cylinderRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"SolidWhiteMaterial"));
	cylinderRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_NONE, false);
	cylinderRenderer->AddRenderPass();
	cylinderRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
	cylinderRenderer->GetRenderPasses()[0]->SetMeshRenderer(cylinderRenderer);
	cylinderRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(newName)->transform());
	cylinderRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(sceneName, newName, cylinderRenderer,
		L"SolidWhiteMaterial", L"Grid");
	auto boxCollider = make_shared<BoxCollider>();
	boxCollider->SetCenter(Vec3(50.0f, 0.0f, 50.0f));
	boxCollider->SetScale(Vec3(100.0f, 0.001f, 100.0f));
	AddComponentToGameObjectAndSaveToXML(sceneName, newName, boxCollider);

	RENDER.GetRenderableObject();

	return _activeScene->Find(newName);
}

shared_ptr<GameObject> SceneManager::CreateTerrainToScene(const wstring& sceneName)
{
	// 새로운 오브젝트 생성 시 기본 이름 설정
	int count = 1;
	wstring baseName = L"Terrain";
	wstring newName = baseName;

	// 이미 존재하는 오브젝트 이름인지 확인
	while (_activeScene->Find(newName) != nullptr)
	{
		newName = baseName + to_wstring(count);
		count++;
	}

	// Terrain
	SaveAndLoadGameObjectToXML(sceneName, newName, Vec3::Zero);
	auto terrainRenderer = make_shared<MeshRenderer>();
	terrainRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Terrain"));
	terrainRenderer->SetModel(nullptr);
	terrainRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"TerrainMaterial"));
	terrainRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	terrainRenderer->AddRenderPass();
	terrainRenderer->GetRenderPasses()[0]->SetPass(Pass::TERRAIN_RENDER);
	terrainRenderer->GetRenderPasses()[0]->SetMeshRenderer(terrainRenderer);
	terrainRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(newName)->transform());
	terrainRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(sceneName, newName, terrainRenderer,
		L"TerrainMaterial", L"Terrain");

	RENDER.GetRenderableObject();

	return _activeScene->Find(newName);
}

shared_ptr<GameObject> SceneManager::CreateParticleToScene(const wstring& sceneName)
{
	// 새로운 오브젝트 생성 시 기본 이름 설정
	int count = 1;
	wstring baseName = L"Particle";
	wstring newName = baseName;

	// 이미 존재하는 오브젝트 이름인지 확인
	while (_activeScene->Find(newName) != nullptr)
	{
		newName = baseName + to_wstring(count);
		count++;
	}

	// Particle System
	SaveAndLoadGameObjectToXML(sceneName, newName,
		GP.centerPos - Vec3(2.0f, 0.0f, 0.0f), Vec3::Zero, Vec3(1.0f));
	auto particleRenderer = make_shared<MeshRenderer>();
	particleRenderer->SetMesh(nullptr);
	particleRenderer->SetModel(nullptr);
	particleRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"ParticleMaterial"));
	particleRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	particleRenderer->AddRenderPass();
	particleRenderer->GetRenderPasses()[0]->SetPass(Pass::PARTICLE_RENDER);
	particleRenderer->GetRenderPasses()[0]->SetMeshRenderer(particleRenderer);
	particleRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(newName)->transform());
	particleRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::CUSTOM3);
	AddComponentToGameObjectAndSaveToXML(sceneName, newName, particleRenderer, L"ParticleMaterial");
	auto particleSystem = make_shared<ParticleSystem>();
	particleSystem->SetEndParticleFlag(false);
	particleSystem->SetSpeed(1.0f);
	particleSystem->SetParticleTyle(ParticleType::FLARE);
	AddComponentToGameObjectAndSaveToXML(sceneName, newName, particleSystem);

	RENDER.GetRenderableObject();

	return _activeScene->Find(newName);
}

shared_ptr<GameObject> SceneManager::CreateAnimatedMeshToScene(const wstring& sceneName, const wstring& modelName)
{
	// 새로운 오브젝트 생성 시 기본 이름 설정
	int count = 1;
	wstring baseName = modelName;
	wstring newName = baseName;

	// 이미 존재하는 오브젝트 이름인지 확인
	while (_activeScene->Find(newName) != nullptr)
	{
		newName = baseName + to_wstring(count);
		count++;
	}

	SaveAndLoadGameObjectToXML(sceneName, newName,
		 Vec3(0.0f, 0.0f, 0.0f), Vec3::Zero, Vec3(0.01f));

	auto meshRenderer = make_shared<MeshRenderer>();
	meshRenderer->SetMesh(nullptr);
	meshRenderer->SetModel(RESOURCE.GetResource<Model>(modelName));
	meshRenderer->SetMaterial(meshRenderer->GetModel()->GetMaterials()[0]);
	meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	meshRenderer->AddRenderPass();
	meshRenderer->GetRenderPasses()[0]->SetPass(Pass::ANIMATED_MESH_RENDER);
	meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
	meshRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(newName)->transform());
	meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(sceneName, newName, meshRenderer,
		L"", L"", modelName);
	auto boxCollider = make_shared<BoxCollider>();
	boxCollider->SetScale(Vec3(1.0f, 1.0f, 1.0f));
	AddComponentToGameObjectAndSaveToXML(sceneName, newName, boxCollider);

	shared_ptr<Model> model = meshRenderer->GetModel();
	vector<shared_ptr<ModelBone>> bones = model->GetBones();

	map<int32, shared_ptr<GameObject>> boneObjects;

	for (shared_ptr<ModelBone> bone : bones)
	{
		shared_ptr<GameObject> boneObject = make_shared<GameObject>();
		boneObject->SetName(bone->name);
		shared_ptr<Transform> transform = make_shared<Transform>();

		Matrix localTransform = bone->transform;

		Vec3 position;
		Quaternion rotation;
		Vec3 scale;
		localTransform.Decompose(scale, rotation, position);

		transform->SetLocalPosition(position);
		transform->SetQTRotation(rotation);
		transform->SetLocalScale(scale);

		boneObject->AddComponent(transform);

		boneObjects[bone->index] = boneObject;

		boneObject->SetParent(_activeScene->Find(newName));

		boneObject->SetBoneObjectFlag(true);
		boneObject->SetBoneParentObject(_activeScene->Find(newName));
		boneObject->SetBoneIndex(bone->index);
		_activeScene->AddBoneGameObject(boneObject);
	}

	RENDER.GetRenderableObject();

	return _activeScene->Find(newName);
}

shared_ptr<GameObject> SceneManager::CreateStaticMeshToScene(const wstring& sceneName, const wstring& modelName)
{
	// 새로운 오브젝트 생성 시 기본 이름 설정
	int count = 1;
	wstring baseName = modelName;
	wstring newName = baseName;

	// 이미 존재하는 오브젝트 이름인지 확인
	while (_activeScene->Find(newName) != nullptr)
	{
		newName = baseName + to_wstring(count);
		count++;
	}

	SaveAndLoadGameObjectToXML(sceneName, newName,
		Vec3(0.f, 0.f, 130.f), Vec3::Zero, Vec3(0.005f));
	auto meshRenderer = make_shared<MeshRenderer>();
	meshRenderer->SetMesh(nullptr);
	meshRenderer->SetModel(RESOURCE.GetResource<Model>(modelName));
	meshRenderer->SetMaterial(meshRenderer->GetModel()->GetMaterials()[0]);
	meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	meshRenderer->AddRenderPass();
	meshRenderer->GetRenderPasses()[0]->SetPass(Pass::STATIC_MESH_RENDER);
	meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
	meshRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(newName)->transform());
	meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(sceneName, newName, meshRenderer,
		L"", L"", modelName);
	auto boxCollider = make_shared<BoxCollider>();
	boxCollider->SetScale(Vec3(1.0f, 1.0f, 1.0f));
	AddComponentToGameObjectAndSaveToXML(sceneName, newName, boxCollider);

	shared_ptr<Model> model = meshRenderer->GetModel();
	vector<shared_ptr<ModelBone>> bones = model->GetBones();

	map<int32, shared_ptr<GameObject>> boneObjects;

	for (shared_ptr<ModelBone> bone : bones)
	{
		shared_ptr<GameObject> boneObject = make_shared<GameObject>();
		boneObject->SetName(bone->name);
		shared_ptr<Transform> transform = make_shared<Transform>();

		Matrix localTransform = bone->transform;

		Vec3 position;
		Quaternion rotation;
		Vec3 scale;
		localTransform.Decompose(scale, rotation, position);

		transform->SetLocalPosition(position);
		transform->SetQTRotation(rotation);
		transform->SetLocalScale(scale);

		boneObject->AddComponent(transform);

		boneObjects[bone->index] = boneObject;

		boneObject->SetParent(_activeScene->Find(newName));

		boneObject->SetBoneObjectFlag(true);
		boneObject->SetBoneParentObject(_activeScene->Find(newName));
		boneObject->SetBoneIndex(bone->index);
		_activeScene->AddBoneGameObject(boneObject);
	}

	RENDER.GetRenderableObject();

	return _activeScene->Find(newName);
}

void SceneManager::UpdateAnimatorTransitionConditionInXML(const wstring& sceneName, const wstring& objectName,
	const string& clipAName, const string& clipBName,
	const vector<Condition>& conditions)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root) return;

	// GameObject 찾기
	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (Utils::ToWString(gameObjElem->Attribute("name")) == objectName)
		{
			// Animator 컴포넌트 찾기
			if (auto animatorElem = gameObjElem->FirstChildElement("Animator"))
			{
				// Transition 찾기
				for (auto transitionElem = animatorElem->FirstChildElement("Transition");
					transitionElem; transitionElem = transitionElem->NextSiblingElement("Transition"))
				{
					if (string(transitionElem->Attribute("clipA")) == clipAName &&
						string(transitionElem->Attribute("clipB")) == clipBName)
					{
						// 기존 Conditions 삭제
						while (auto conditionElem = transitionElem->FirstChildElement("Condition"))
						{
							transitionElem->DeleteChild(conditionElem);
						}

						// hasCondition 값 업데이트 추가
						transitionElem->SetAttribute("hasCondition", !conditions.empty());

						// 새로운 Conditions 추가
						for (const auto& condition : conditions)
						{
							auto conditionElem = doc.NewElement("Condition");
							conditionElem->SetAttribute("parameterName", condition.parameterName.c_str());
							conditionElem->SetAttribute("parameterType", static_cast<int>(condition.parameterType));
							conditionElem->SetAttribute("compareType", static_cast<int>(condition.compareType));

							// 값 저장
							switch (condition.parameterType)
							{
							case Parameter::Type::Bool:
								conditionElem->SetAttribute("value", condition.value.boolValue);
								break;
							case Parameter::Type::Int:
								conditionElem->SetAttribute("value", condition.value.intValue);
								break;
							case Parameter::Type::Float:
								conditionElem->SetAttribute("value", condition.value.floatValue);
								break;
							}

							transitionElem->InsertEndChild(conditionElem);
						}

						doc.SaveFile(pathStr.c_str());
						break;
					}
				}
			}
			break;
		}
	}
}

void SceneManager::RemoveAnimatorTransitionFromXML(const wstring& sceneName, const wstring& objectName,
	const string& clipAName, const string& clipBName)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root) return;

	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (Utils::ToWString(gameObjElem->Attribute("name")) == objectName)
		{
			if (auto animatorElem = gameObjElem->FirstChildElement("Animator"))
			{
				for (auto transitionElem = animatorElem->FirstChildElement("Transition");
					transitionElem; transitionElem = transitionElem->NextSiblingElement("Transition"))
				{
					if (string(transitionElem->Attribute("clipA")) == clipAName &&
						string(transitionElem->Attribute("clipB")) == clipBName)
					{
						animatorElem->DeleteChild(transitionElem);
						doc.SaveFile(pathStr.c_str());
						break;
					}
				}
			}
			break;
		}
	}
}

void SceneManager::UpdateAnimatorEntryClipInXML(const wstring& sceneName, const wstring& objectName,
	const string& entryClipName)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root) return;

	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (Utils::ToWString(gameObjElem->Attribute("name")) == objectName)
		{
			if (auto animatorElem = gameObjElem->FirstChildElement("Animator"))
			{
				// 기존 EntryClip 엘리먼트 찾아서 삭제
				if (auto entryElem = animatorElem->FirstChildElement("EntryClip"))
				{
					animatorElem->DeleteChild(entryElem);
				}

				// 새로운 EntryClip 엘리먼트 생성
				auto newEntryElem = doc.NewElement("EntryClip");
				newEntryElem->SetAttribute("name", entryClipName.c_str());
				animatorElem->InsertFirstChild(newEntryElem);

				doc.SaveFile(pathStr.c_str());
				break;
			}
			break;
		}
	}
}

void SceneManager::AddAnimatorTransitionToXML(const wstring& sceneName, const wstring& objectName,
	const string& clipAName, const string& clipBName)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root) return;

	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (Utils::ToWString(gameObjElem->Attribute("name")) == objectName)
		{
			if (auto animatorElem = gameObjElem->FirstChildElement("Animator"))
			{
				// 새로운 Transition 엘리먼트 생성
				auto transitionElem = doc.NewElement("Transition");
				transitionElem->SetAttribute("clipA", clipAName.c_str());
				transitionElem->SetAttribute("clipB", clipBName.c_str());
				transitionElem->SetAttribute("transitionDuration", 0.3f);
				transitionElem->SetAttribute("transitionOffset", 0.0f);
				transitionElem->SetAttribute("hasExitTime", true);
				transitionElem->SetAttribute("ExitTime", 1.0f);

				animatorElem->InsertEndChild(transitionElem);
				doc.SaveFile(pathStr.c_str());
				break;
			}
			break;
		}
	}
}
void SceneManager::RemoveAnimatorParameterFromXML(const wstring& sceneName, const wstring& objectName, const string& paramName)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root) return;

	// GameObject 찾기
	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (Utils::ToWString(gameObjElem->Attribute("name")) == objectName)
		{
			// Animator 컴포넌트 찾기
			if (auto animatorElem = gameObjElem->FirstChildElement("Animator"))
			{
				// Parameter 찾아서 삭제
				for (auto paramElem = animatorElem->FirstChildElement("Parameter");
					paramElem; paramElem = paramElem->NextSiblingElement("Parameter"))
				{
					if (string(paramElem->Attribute("name")) == paramName)
					{
						animatorElem->DeleteChild(paramElem);

						// Transition의 Condition들도 삭제
						for (auto transitionElem = animatorElem->FirstChildElement("Transition");
							transitionElem; transitionElem = transitionElem->NextSiblingElement("Transition"))
						{
							auto conditionElem = transitionElem->FirstChildElement("Condition");
							while (conditionElem)
							{
								auto nextCondition = conditionElem->NextSiblingElement("Condition");
								if (string(conditionElem->Attribute("parameterName")) == paramName)
								{
									transitionElem->DeleteChild(conditionElem);
								}
								conditionElem = nextCondition;
							}
						}

						doc.SaveFile(pathStr.c_str());
						break;
					}
				}
			}
			break;
		}
	}
}

void SceneManager::UpdateAnimatorNodePositionInXML(const wstring& sceneName, const wstring& objectName,
	const string& clipName, const ImVec2& position)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root) return;

	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (Utils::ToWString(gameObjElem->Attribute("name")) == objectName)
		{
			if (auto animatorElem = gameObjElem->FirstChildElement("Animator"))
			{
				for (auto clipElem = animatorElem->FirstChildElement("Clip");
					clipElem; clipElem = clipElem->NextSiblingElement("Clip"))
				{
					if (string(clipElem->Attribute("name")) == clipName)
					{
						clipElem->SetAttribute("posX", position.x);
						clipElem->SetAttribute("posY", position.y);
						doc.SaveFile(pathStr.c_str());
						break;
					}
				}
			}
			break;
		}
	}
}

ImVec2 SceneManager::GetAnimatorNodePositionFromXML(const wstring& sceneName, const wstring& objectName,
	const string& clipName)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return ImVec2(0, 0);

	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root) return ImVec2(0, 0);

	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (Utils::ToWString(gameObjElem->Attribute("name")) == objectName)
		{
			if (auto animatorElem = gameObjElem->FirstChildElement("Animator"))
			{
				for (auto clipElem = animatorElem->FirstChildElement("Clip");
					clipElem; clipElem = clipElem->NextSiblingElement("Clip"))
				{
					if (string(clipElem->Attribute("name")) == clipName)
					{
						return ImVec2(
							clipElem->FloatAttribute("posX", 0.0f),
							clipElem->FloatAttribute("posY", 0.0f)
						);
					}
				}
			}
			break;
		}
	}
	return ImVec2(0, 0);
}

void SceneManager::UpdateAnimatorTransitionFlagInXML(const wstring& sceneName, const wstring& objectName,
	const string& clipAName, const string& clipBName, bool flag, bool hasCondition)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root) return;

	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (Utils::ToWString(gameObjElem->Attribute("name")) == objectName)
		{
			if (auto animatorElem = gameObjElem->FirstChildElement("Animator"))
			{
				for (auto transitionElem = animatorElem->FirstChildElement("Transition");
					transitionElem; transitionElem = transitionElem->NextSiblingElement("Transition"))
				{
					if (string(transitionElem->Attribute("clipA")) == clipAName &&
						string(transitionElem->Attribute("clipB")) == clipBName)
					{
						transitionElem->SetAttribute("flag", flag);
						transitionElem->SetAttribute("hasCondition", hasCondition);
						doc.SaveFile(pathStr.c_str());
						break;
					}
				}
			}
			break;
		}
	}
}

void SceneManager::UpdateAnimatorClipEventsInXML(const wstring& sceneName, const wstring& objectName,
	const string& clipName, const vector<AnimationEvent>& events)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root) return;

	// GameObject 찾기
	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (Utils::ToWString(gameObjElem->Attribute("name")) == objectName)
		{
			// Animator 컴포넌트 찾기
			if (auto animatorElem = gameObjElem->FirstChildElement("Animator"))
			{
				// Clip 찾기
				for (auto clipElem = animatorElem->FirstChildElement("Clip");
					clipElem; clipElem = clipElem->NextSiblingElement("Clip"))
				{
					if (string(clipElem->Attribute("name")) == clipName)
					{
						// 기존 Events 삭제
						while (auto eventElem = clipElem->FirstChildElement("Event"))
						{
							clipElem->DeleteChild(eventElem);
						}

						// 새로운 Events 추가
						for (const auto& event : events)
						{
							auto eventElem = doc.NewElement("Event");
							eventElem->SetAttribute("time", event.time);
							eventElem->SetAttribute("function", event.function.functionKey.c_str());
							clipElem->InsertEndChild(eventElem);
						}

						doc.SaveFile(pathStr.c_str());
						break;
					}
				}
			}
			break;
		}
	}
}

void SceneManager::UpdateGameObjectMaterialInXML(const wstring& sceneName, const wstring& objectName, const wstring& materialName)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root) return;

	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (Utils::ToWString(gameObjElem->Attribute("name")) == objectName)
		{
			if (auto meshRendererElem = gameObjElem->FirstChildElement("MeshRenderer"))
			{
				// material은 MeshRenderer의 attribute로 존재
				meshRendererElem->SetAttribute("material", Utils::ToString(materialName).c_str());
				doc.SaveFile(pathStr.c_str());
			}
			break;
		}
	}
}

void SceneManager::UpdateGameObjectRenderPassInXML(const wstring& sceneName, const wstring& objectName, Pass pass, bool useEnvironmentMap)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root) return;

	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (Utils::ToWString(gameObjElem->Attribute("name")) == objectName)
		{
			if (auto meshRendererElem = gameObjElem->FirstChildElement("MeshRenderer"))
			{
				// useEnvironmentMap 업데이트
				meshRendererElem->SetAttribute("useEnvironmentMap", useEnvironmentMap);

				// RenderPass 업데이트
				if (auto passElem = meshRendererElem->FirstChildElement("RenderPass"))
				{
					passElem->SetAttribute("pass", static_cast<int>(pass));
					if (static_cast<int>(pass) == 1)
						passElem->SetAttribute("depthStencilState", 1);
					else if (static_cast<int>(pass) == 11)
						passElem->SetAttribute("depthStencilState", 4);
					else
						passElem->SetAttribute("depthStencilState", 0);
				}
				doc.SaveFile(pathStr.c_str());
			}
			break;
		}
	}
}

void SceneManager::UpdateGameObjectParticleSystemInXML(const wstring& sceneName, const wstring& objectName, float speed, bool endParticle, ParticleType type)
{
	if (ENGINE.GetEngineMode() != EngineMode::Edit)
		return;

	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root) return;

	// GameObject 찾기
	for (tinyxml2::XMLElement* gameObjElem = root->FirstChildElement("GameObject");
		gameObjElem; gameObjElem = gameObjElem->NextSiblingElement("GameObject"))
	{
		if (Utils::ToWString(gameObjElem->Attribute("name")) == objectName)
		{
			// ParticleSystem 엘리먼트 찾기
			auto particleElem = gameObjElem->FirstChildElement("ParticleSystem");
			if (particleElem)
			{
				// 속성 업데이트
				particleElem->SetAttribute("speed", speed);
				particleElem->SetAttribute("endParticle", endParticle);
				particleElem->SetAttribute("type", static_cast<int>(type));  // ParticleType 저장
			}
			else
			{
				// ParticleSystem 엘리먼트가 없으면 새로 생성
				particleElem = doc.NewElement("ParticleSystem");
				particleElem->SetAttribute("speed", speed);
				particleElem->SetAttribute("endParticle", endParticle);
				particleElem->SetAttribute("type", static_cast<int>(type));  // ParticleType 저장
				gameObjElem->InsertEndChild(particleElem);
			}

			doc.SaveFile(pathStr.c_str());
			break;
		}
	}
}
