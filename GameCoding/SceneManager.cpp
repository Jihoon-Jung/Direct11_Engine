#include "pch.h"
#include "SceneManager.h"
#include "GameObject.h"
#include "MoveObject.h"
#include "RenderPass.h"

void SceneManager::Init()
{
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
	LoadTestScene2();
}
void SceneManager::LoadTestScene2()
{
	wstring sceneName = L"test_scene";
	_activeScene = make_shared<Scene>();

	// XML 파일 로드
	tinyxml2::XMLDocument doc;
	string pathStr = "Resource/Scene/" + Utils::ToString(sceneName) + ".xml";
	doc.LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = doc.FirstChildElement("Scene");
	if (!root)
		return;

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
			particleSystem->SetTransform(gameObj->transform());
			gameObj->AddComponent(particleSystem);
		}
		// Script 컴포넌트 처리
		if (auto scriptElem = gameObjElem->FirstChildElement("Script"))
		{
			string type = scriptElem->Attribute("type");
			if (type == "MoveObject")
			{
				auto moveObject = make_shared<MoveObject>();
				gameObj->AddComponent(moveObject);
			}
			// 다른 스크립트 타입들도 여기에 추가
		}

		_activeScene->AddGameObject(gameObj);
	}
}
void SceneManager::LoadTestScene()
{
	_activeScene = make_shared<Scene>();

	//{
	//	shared_ptr<GameObject> camera = make_shared<GameObject>();
	//	shared_ptr<Transform> camera_transform = make_shared<Transform>();
	//	shared_ptr<MoveObject> moveObject = make_shared<MoveObject>();
	//	shared_ptr<Camera> cameraComponent = make_shared<Camera>();

	//	camera_transform->SetLocalPosition(GP.centerPos + Vec3(-2.0f, 2.f, -10.0f));
	//	camera->AddComponent(camera_transform);

	//	cameraComponent->SetProjectionType(ProjectionType::Perspective);
	//	camera->AddComponent(cameraComponent);

	//	camera->AddComponent(moveObject);

	//	camera->SetName(L"MainCamera");

	//	_activeScene->AddGameObject(camera);
	//}

	//{
	//	shared_ptr<GameObject> uiCamera = make_shared<GameObject>();
	//	shared_ptr<Transform> uiCamera_transform = make_shared<Transform>();
	//	shared_ptr<Camera> uiCameraComponent = make_shared<Camera>();

	//	uiCamera_transform->SetLocalPosition(Vec3(0, 0, -3));
	//	uiCamera->AddComponent(uiCamera_transform);

	//	uiCameraComponent->SetProjectionType(ProjectionType::Orthographic);
	//	uiCamera->AddComponent(uiCameraComponent);

	//	uiCamera->SetName(L"UICamera");

	//	_activeScene->AddGameObject(uiCamera);
	//}

	//{
	//	shared_ptr<GameObject> light = make_shared<GameObject>();
	//	shared_ptr<Transform> light_transform = make_shared<Transform>();
	//	shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

	//	light_transform->SetPosition(Vec3(-28.0f, 37.0f, 112.0f));
	//	light_transform->SetScale(Vec3(10.0f, 10.0f, 10.0f));
	//	light->AddComponent(light_transform);
	//	light->AddComponent(make_shared<Light>());

	//	
	//	meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"LightMaterial"));
	//	meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Sphere"));
	//	meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	//	meshRenderer->AddRenderPass();
	//	meshRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
	//	meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
	//	meshRenderer->GetRenderPasses()[0]->SetTransform(light_transform);
	//	meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	//	light->AddComponent(meshRenderer);

	//	light->SetObjectType(GameObjectType::NormalObject);
	//	light->SetName(L"MainLight");

	//	_activeScene->AddGameObject(light);
	//}


	//{
	//	shared_ptr<GameObject> env_sphere = make_shared<GameObject>();
	//	shared_ptr<Transform> env_sphere_transform = make_shared<Transform>();
	//	shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
	//	shared_ptr<SphereCollider> sphereCollider = make_shared<SphereCollider>();

	//	env_sphere_transform->SetPosition(Vec3(4.0f, 1.f, 122.f));
	//	env_sphere->AddComponent(env_sphere_transform);

	//	meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"DefaultMaterial"));
	//	meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Sphere"));
	//	meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);

	//	meshRenderer->AddRenderPass();
	//	meshRenderer->GetRenderPasses()[0]->SetPass(Pass::ENVIRONMENTMAP_RENDER);
	//	meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
	//	meshRenderer->GetRenderPasses()[0]->SetTransform(env_sphere_transform);
	//	meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	//	meshRenderer->SetUseEnvironmentMap(true);
	//	env_sphere->AddComponent(meshRenderer);

	//	sphereCollider->SetRadius(0.5f);
	//	env_sphere->AddComponent(sphereCollider);

	//	env_sphere->SetObjectType(GameObjectType::NormalObject);
	//	env_sphere->SetName(L"Env_Sphere");

	//	_activeScene->AddGameObject(env_sphere);
	//}


	//{
	//	shared_ptr<GameObject> sphere = make_shared<GameObject>();
	//	shared_ptr<Transform> sphere_transform = make_shared<Transform>();
	//	shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
	//	shared_ptr<SphereCollider> sphereCollider = make_shared<SphereCollider>();

	//	sphere_transform->SetPosition(Vec3(-6.5f, 5.6f, 122.f));
	//	sphere->AddComponent(sphere_transform);

	//	meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"DefaultMaterial"));
	//	meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Sphere"));
	//	meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	//	meshRenderer->AddRenderPass();
	//	meshRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
	//	meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
	//	meshRenderer->GetRenderPasses()[0]->SetTransform(sphere_transform);
	//	meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	//	sphere->AddComponent(meshRenderer);

	//	sphereCollider->SetRadius(1.0f);
	//	sphere->AddComponent(sphereCollider);

	//	sphere->SetObjectType(GameObjectType::NormalObject);
	//	sphere->SetName(L"Sphere");

	//	_activeScene->AddGameObject(sphere);
	//}


	//{
	//	shared_ptr<GameObject> skyBox = make_shared<GameObject>();
	//	shared_ptr<Transform> skyBox_transform = make_shared<Transform>();
	//	shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

	//	skyBox_transform->SetPosition(Vec3(0.f, 0.f, 0.f));
	//	skyBox->AddComponent(skyBox_transform);

	//	meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"SkyBoxMaterial"));
	//	meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Sphere"));
	//	meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, true);
	//	meshRenderer->AddRenderPass();
	//	meshRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
	//	meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
	//	meshRenderer->GetRenderPasses()[0]->SetTransform(skyBox_transform);
	//	meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	//	skyBox->AddComponent(meshRenderer);

	//	skyBox->SetObjectType(GameObjectType::NormalObject);
	//	skyBox->SetName(L"skyBox");

	//	_activeScene->AddGameObject(skyBox);
	//}


	//{
	//	shared_ptr<GameObject> cube = make_shared<GameObject>();
	//	shared_ptr<Transform> cube_transform = make_shared<Transform>();
	//	shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();

	//	cube_transform->SetPosition(Vec3(-6.5f, 0.6f, 122.f));
	//	cube->AddComponent(cube_transform);

	//	shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
	//	meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"DefaultMaterial"));
	//	meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Cube"));
	//	meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	//	meshRenderer->AddRenderPass();
	//	meshRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
	//	meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
	//	meshRenderer->GetRenderPasses()[0]->SetTransform(cube_transform);
	//	meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	//	cube->AddComponent(meshRenderer);

	//	boxCollider->SetScale(Vec3(1.0f, 1.0f, 1.0f));
	//	cube->AddComponent(boxCollider);

	//	cube->SetObjectType(GameObjectType::NormalObject);
	//	cube->SetName(L"cube");

	//	_activeScene->AddGameObject(cube);
	//}
	//

	//{
	//	shared_ptr<GameObject> tessellation_cylinder = make_shared<GameObject>();
	//	shared_ptr<Transform> tessellation_cylinder_transform = make_shared<Transform>();
	//	shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
	//	shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();

	//	tessellation_cylinder_transform->SetPosition(Vec3(-7.0f, 1.5f, 120.0f));
	//	tessellation_cylinder->AddComponent(tessellation_cylinder_transform);

	//	meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"Tessellation_Material"));
	//	meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Cylinder"));
	//	meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	//	meshRenderer->AddRenderPass();
	//	meshRenderer->GetRenderPasses()[0]->SetPass(Pass::TESSELLATION_RENDER);
	//	meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
	//	meshRenderer->GetRenderPasses()[0]->SetTransform(tessellation_cylinder_transform);
	//	meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	//	tessellation_cylinder->AddComponent(meshRenderer);

	//	boxCollider->SetScale(Vec3(1.0f, 1.0f, 1.0f));
	//	tessellation_cylinder->AddComponent(boxCollider);

	//	tessellation_cylinder->SetObjectType(GameObjectType::NormalObject);
	//	tessellation_cylinder->SetName(L"tessellation_cube");

	//	_activeScene->AddGameObject(tessellation_cylinder);
	//}



	//{
	//	shared_ptr<GameObject> stencil_cube1 = make_shared<GameObject>();
	//	shared_ptr<Transform> stencil_cube1_transform = make_shared<Transform>();
	//	shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

	//	stencil_cube1_transform->SetPosition(Vec3(-10.5f, 0.f, 120.f));
	//	stencil_cube1_transform->SetLocalScale(Vec3(1.0f));
	//	stencil_cube1->AddComponent(stencil_cube1_transform);

	//	meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"DefaultMaterial"));
	//	meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Cube"));
	//	meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	//	meshRenderer->AddRenderPass();
	//	meshRenderer->GetRenderPasses()[0]->SetPass(Pass::OUTLINE_RENDER);
	//	meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
	//	meshRenderer->GetRenderPasses()[0]->SetTransform(stencil_cube1_transform);
	//	meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::CUSTOM1);
	//	stencil_cube1->AddComponent(meshRenderer);

	//	stencil_cube1->SetObjectType(GameObjectType::NormalObject);
	//	stencil_cube1->SetName(L"stencil_cube1");

	//	_activeScene->AddGameObject(stencil_cube1);
	//}


	//{
	//	shared_ptr<GameObject> stencil_cube2 = make_shared<GameObject>();
	//	shared_ptr<Transform> stencil_cube2_transform = make_shared<Transform>();
	//	shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

	//	stencil_cube2_transform->SetPosition(Vec3(-10.5f, 0.f, 120.f));
	//	stencil_cube2_transform->SetScale(Vec3(1.1f));
	//	stencil_cube2->AddComponent(stencil_cube2_transform);

	//	meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"LightMaterial"));
	//	meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Cube"));
	//	meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	//	meshRenderer->AddRenderPass();
	//	meshRenderer->GetRenderPasses()[0]->SetPass(Pass::OUTLINE_RENDER);
	//	meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
	//	meshRenderer->GetRenderPasses()[0]->SetTransform(stencil_cube2_transform);
	//	meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::CUSTOM2);
	//	stencil_cube2->AddComponent(meshRenderer);

	//	stencil_cube2->SetObjectType(GameObjectType::NormalObject);
	//	stencil_cube2->SetName(L"stencil_cube2");

	//	_activeScene->AddGameObject(stencil_cube2);
	//}


	//{
	//	shared_ptr<GameObject> terrain = make_shared<GameObject>();
	//	shared_ptr<Transform> terrain_tranform = make_shared<Transform>();
	//	shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

	//	terrain_tranform->SetPosition(Vec3(0.0f, 0.0f, 0.0f));
	//	terrain->AddComponent(terrain_tranform);

	//	meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"TerrainMaterial"));
	//	meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Terrain"));
	//	meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	//	meshRenderer->AddRenderPass();
	//	meshRenderer->GetRenderPasses()[0]->SetPass(Pass::TERRAIN_RENDER);
	//	meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
	//	meshRenderer->GetRenderPasses()[0]->SetTransform(terrain_tranform);
	//	meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	//	terrain->AddComponent(meshRenderer);

	//	terrain->SetObjectType(GameObjectType::NormalObject);
	//	terrain->SetName(L"Terrain_obj");

	//	_activeScene->AddGameObject(terrain);
	//}


	//{
	//	shared_ptr<GameObject> quad_ui = make_shared<GameObject>();
	//	shared_ptr<Transform> quad_ui_transform = make_shared<Transform>();
	//	shared_ptr<UIImage> uiImageComponent = make_shared<UIImage>();

	//	uiImageComponent->SetTransformAndRect(Vec2(900, 200), Vec2(240, 180));
	//	quad_ui->AddComponent(uiImageComponent);

	//	quad_ui_transform->SetPosition(uiImageComponent->GetNDCPosition());
	//	quad_ui_transform->SetLocalScale(uiImageComponent->GetSize());
	//	quad_ui->AddComponent(quad_ui_transform);

	//	shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
	//	meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"Debug_UI_Material"));
	//	meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Quad"));
	//	meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	//	meshRenderer->AddRenderPass();
	//	meshRenderer->GetRenderPasses()[0]->SetPass(Pass::DEBUG_2D_RENDER);
	//	meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
	//	meshRenderer->GetRenderPasses()[0]->SetTransform(quad_ui_transform);
	//	meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	//	quad_ui->AddComponent(meshRenderer);

	//	quad_ui->SetObjectType(GameObjectType::UIObject);
	//	quad_ui->SetName(L"UI_Quad");

	//	_activeScene->AddGameObject(quad_ui);
	//}


	//{
	//	shared_ptr<GameObject> uiButton = make_shared<GameObject>();
	//	shared_ptr<Transform> uiButton_transform = make_shared<Transform>();
	//	shared_ptr<Button> buttonComponent = make_shared<Button>();

	//	buttonComponent->AddOnClickedEvent([this, uiButton]() {_activeScene->RemoveGameObject(uiButton); });
	//	buttonComponent->SetTransformAndRect(Vec2(100, 100), Vec2(100, 100));
	//	uiButton->AddComponent(buttonComponent);

	//	uiButton_transform->SetPosition(buttonComponent->GetNDCPosition());
	//	uiButton_transform->SetLocalScale(buttonComponent->GetSize());
	//	uiButton->AddComponent(uiButton_transform);

	//	shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
	//	meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"Debug_UI_Material"));
	//	meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Quad"));
	//	meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	//	meshRenderer->AddRenderPass();
	//	meshRenderer->GetRenderPasses()[0]->SetPass(Pass::DEBUG_2D_RENDER);
	//	meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
	//	meshRenderer->GetRenderPasses()[0]->SetTransform(uiButton_transform);
	//	meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	//	uiButton->AddComponent(meshRenderer);

	//	uiButton->SetObjectType(GameObjectType::UIObject);
	//	uiButton->SetName(L"UI_Button");

	//	//_activeScene->AddGameObject(uiButton);
	//}


	//{
	//	shared_ptr<GameObject> house = make_shared<GameObject>();
	//	shared_ptr<Transform> house_transfrom = make_shared<Transform>();
	//	shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
	//	shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();

	//	house_transfrom->SetPosition(Vec3(0.f, 0.f, 130.f));
	//	house_transfrom->SetLocalScale(Vec3(0.005f));
	//	house->AddComponent(house_transfrom);

	//	meshRenderer->SetModel(RESOURCE.GetResource<Model>(L"HouseModel"));
	//	meshRenderer->SetMaterial(meshRenderer->GetModel()->GetMaterials()[0]);
	//	meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	//	meshRenderer->AddRenderPass();
	//	meshRenderer->GetRenderPasses()[0]->SetPass(Pass::STATIC_MESH_RENDER);
	//	meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
	//	meshRenderer->GetRenderPasses()[0]->SetTransform(house_transfrom);
	//	meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	//	house->AddComponent(meshRenderer);

	//	boxCollider->SetScale(Vec3(1.0f, 1.0f, 1.0f));
	//	house->AddComponent(boxCollider);

	//	house->SetObjectType(GameObjectType::NormalObject);
	//	house->SetName(L"house");

	//	_activeScene->AddGameObject(house);
	//}


	//{
	//	shared_ptr<GameObject> Kachujin = make_shared<GameObject>();
	//	shared_ptr<Transform> kachujin_transform = make_shared<Transform>();
	//	shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
	//	shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();

	//	kachujin_transform->SetPosition(GP.centerPos - Vec3(2.0f, 0.0f, 0.0f));
	//	kachujin_transform->SetLocalScale(Vec3(0.01f));
	//	Kachujin->AddComponent(kachujin_transform);

	//	meshRenderer->SetModel(RESOURCE.GetResource<Model>(L"Kachujin"));
	//	meshRenderer->SetMaterial(meshRenderer->GetModel()->GetMaterials()[0]);
	//	meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	//	meshRenderer->AddRenderPass();
	//	meshRenderer->GetRenderPasses()[0]->SetPass(Pass::ANIMATED_MESH_RENDER);
	//	meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
	//	meshRenderer->GetRenderPasses()[0]->SetTransform(kachujin_transform);
	//	meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	//	Kachujin->AddComponent(meshRenderer);


	//	boxCollider->SetScale(Vec3(1.0f, 1.0f, 1.0f));
	//	Kachujin->AddComponent(boxCollider);

	//	Kachujin->SetObjectType(GameObjectType::NormalObject);
	//	Kachujin->SetName(L"Kachujin_OBJ");

	//	_activeScene->AddGameObject(Kachujin);
	//}


	//{
	//	shared_ptr<GameObject> Dreyar = make_shared<GameObject>();
	//	shared_ptr<Transform> Dreyar_transform = make_shared<Transform>();
	//	shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
	//	shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();

	//	Dreyar_transform->SetPosition(GP.centerPos);
	//	Dreyar_transform->SetLocalScale(Vec3(0.001f, 0.001f, 0.001f));
	//	Dreyar->AddComponent(Dreyar_transform);

	//	meshRenderer->SetModel(RESOURCE.GetResource<Model>(L"Dreyar"));
	//	meshRenderer->SetMaterial(meshRenderer->GetModel()->GetMaterials()[0]);
	//	meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	//	meshRenderer->AddRenderPass();
	//	meshRenderer->GetRenderPasses()[0]->SetPass(Pass::ANIMATED_MESH_RENDER);
	//	meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
	//	meshRenderer->GetRenderPasses()[0]->SetTransform(Dreyar_transform);
	//	meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	//	Dreyar->AddComponent(meshRenderer);

	//	boxCollider->SetScale(Vec3(1.0f, 1.0f, 1.0f));
	//	Dreyar->AddComponent(boxCollider);

	//	Dreyar->SetObjectType(GameObjectType::NormalObject);
	//	Dreyar->SetName(L"Dreyar_OBJ");

	//	_activeScene->AddGameObject(Dreyar);
	//}


	//{
	//	shared_ptr<GameObject> particleSystem = make_shared<GameObject>();
	//	shared_ptr<Transform> particle_transform = make_shared<Transform>();
	//	shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
	//	shared_ptr<ParticleSystem> particleComponent = make_shared<ParticleSystem>();

	//	particle_transform->SetPosition(GP.centerPos - Vec3(2.0f, 0.0f, 0.0f));
	//	particle_transform->SetLocalScale(Vec3(1.0f));
	//	particleSystem->AddComponent(particle_transform);

	//	meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	//	meshRenderer->AddRenderPass();
	//	meshRenderer->GetRenderPasses()[0]->SetPass(Pass::PARTICLE_RENDER);
	//	meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
	//	meshRenderer->GetRenderPasses()[0]->SetTransform(particle_transform);
	//	meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::CUSTOM3);
	//	particleSystem->AddComponent(meshRenderer);

	//	particleComponent->SetTransform(particle_transform);
	//	particleSystem->AddComponent(particleComponent);

	//	particleSystem->SetObjectType(GameObjectType::NormalObject);
	//	particleSystem->SetName(L"FireParticle");

	//	_activeScene->AddGameObject(particleSystem);
	//}
	//{
	//	shared_ptr<GameObject> skyBox = make_shared<GameObject>();
	//	shared_ptr<Transform> skyBox_transform = make_shared<Transform>();
	//	shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();

	//	skyBox_transform->SetPosition(Vec3(0.f, 0.f, 0.f));
	//	skyBox->AddComponent(skyBox_transform);

	//	meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"SkyBoxMaterial"));
	//	meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Sphere"));
	//	meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, true);
	//	meshRenderer->AddRenderPass();
	//	meshRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
	//	meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
	//	meshRenderer->GetRenderPasses()[0]->SetTransform(skyBox_transform);
	//	meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	//	skyBox->AddComponent(meshRenderer);

	//	skyBox->SetObjectType(GameObjectType::NormalObject);
	//	skyBox->SetName(L"skyBox");

	//	_activeScene->AddGameObject(skyBox);
	//}

	//SaveSceneToXML(L"Resource/Scene/test_scene.xml");


	 // MainCamera
	SaveAndLoadGameObjectToXML(L"test_scene", L"MainCamera",
		GP.centerPos + Vec3(-2.0f, 2.f, -10.0f));
	auto camera = make_shared<Camera>();
	camera->SetProjectionType(ProjectionType::Perspective);
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"MainCamera", camera);
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"MainCamera", make_shared<MoveObject>());

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
	SaveAndLoadGameObjectToXML(L"test_scene", L"Sphere",
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
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"Sphere", sphereRenderer,
		L"DefaultMaterial", L"Sphere");
	sphereCollider = make_shared<SphereCollider>();
	sphereCollider->SetRadius(1.0f);
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"Sphere", sphereCollider);

	// Cube
	SaveAndLoadGameObjectToXML(L"test_scene", L"cube",
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
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"cube", cubeRenderer,
		L"DefaultMaterial", L"Cube");
	auto boxCollider = make_shared<BoxCollider>();
	boxCollider->SetScale(Vec3(1.0f, 1.0f, 1.0f));
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"cube", boxCollider);

	// Tessellation Cylinder
	SaveAndLoadGameObjectToXML(L"test_scene", L"tessellation_cube",
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
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"tessellation_cube", tessellationRenderer,
		L"Tessellation_Material", L"Cylinder");
	boxCollider = make_shared<BoxCollider>();
	boxCollider->SetScale(Vec3(1.0f, 1.0f, 1.0f));
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"tessellation_cube", boxCollider);

	// Stencil Cube 1
	SaveAndLoadGameObjectToXML(L"test_scene", L"stencil_cube1",
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
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"stencil_cube1", stencilRenderer1,
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

	// UI Quad
	SaveAndLoadGameObjectToXML(L"test_scene", L"UI_Quad", Vec3::Zero, Vec3::Zero, Vec3::One,
		nullptr);
	auto uiImage = make_shared<UIImage>();
	uiImage->SetTransformAndRect(Vec2(900, 200), Vec2(240, 180));
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"UI_Quad", uiImage);
	auto uiRenderer = make_shared<MeshRenderer>();
	uiRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Quad"));
	uiRenderer->SetModel(nullptr);
	uiRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"Debug_UI_Material"));
	uiRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	uiRenderer->AddRenderPass();
	uiRenderer->GetRenderPasses()[0]->SetPass(Pass::DEBUG_2D_RENDER);
	uiRenderer->GetRenderPasses()[0]->SetMeshRenderer(uiRenderer);
	uiRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"UI_Quad")->transform());
	uiRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"UI_Quad", uiRenderer,
		L"Debug_UI_Material", L"Quad");

	// UI Button
	SaveAndLoadGameObjectToXML(L"test_scene", L"UI_Button", Vec3::Zero, Vec3::Zero, Vec3::One,
		nullptr);
	auto button = make_shared<Button>();
	button->AddOnClickedEvent([this]() {
		_activeScene->RemoveGameObject(_activeScene->Find(L"UI_Button"));
		});
	button->SetTransformAndRect(Vec2(100, 100), Vec2(100, 100));
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"UI_Button", button);
	auto buttonRenderer = make_shared<MeshRenderer>();
	buttonRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Quad"));
	buttonRenderer->SetModel(nullptr);
	buttonRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"Debug_UI_Material"));
	buttonRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	buttonRenderer->AddRenderPass();
	buttonRenderer->GetRenderPasses()[0]->SetPass(Pass::DEBUG_2D_RENDER);
	buttonRenderer->GetRenderPasses()[0]->SetMeshRenderer(buttonRenderer);
	buttonRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"UI_Button")->transform());
	buttonRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"UI_Button", buttonRenderer,
		L"Debug_UI_Material", L"Quad");

	// House
	SaveAndLoadGameObjectToXML(L"test_scene", L"house",
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
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"house", houseRenderer,
		L"", L"", L"HouseModel");
	boxCollider = make_shared<BoxCollider>();
	boxCollider->SetScale(Vec3(1.0f, 1.0f, 1.0f));
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"house", boxCollider);

	// Kachujin
	SaveAndLoadGameObjectToXML(L"test_scene", L"Kachujin_OBJ",
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
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"Kachujin_OBJ", kachujinRenderer,
		L"", L"", L"Kachujin");
	boxCollider = make_shared<BoxCollider>();
	boxCollider->SetScale(Vec3(1.0f, 1.0f, 1.0f));
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"Kachujin_OBJ", boxCollider);

	// Dreyar
	SaveAndLoadGameObjectToXML(L"test_scene", L"Dreyar_OBJ",
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
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"Dreyar_OBJ", dreyarRenderer,
		L"", L"", L"Dreyar");
	boxCollider = make_shared<BoxCollider>();
	boxCollider->SetScale(Vec3(1.0f, 1.0f, 1.0f));
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"Dreyar_OBJ", boxCollider);

	// Particle System
	SaveAndLoadGameObjectToXML(L"test_scene", L"FireParticle",
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
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"FireParticle", particleRenderer, L"ParticleMaterial");
	auto particleSystem = make_shared<ParticleSystem>();
	particleSystem->SetTransform(_activeScene->Find(L"FireParticle")->transform());
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"FireParticle", particleSystem);

	// SkyBox
	SaveAndLoadGameObjectToXML(L"test_scene", L"skyBox", Vec3::Zero);
	auto skyBoxRenderer = make_shared<MeshRenderer>();
	skyBoxRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Sphere"));
	skyBoxRenderer->SetModel(nullptr);
	skyBoxRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"SkyBoxMaterial"));
	skyBoxRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, true);
	skyBoxRenderer->AddRenderPass();
	skyBoxRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
	skyBoxRenderer->GetRenderPasses()[0]->SetMeshRenderer(skyBoxRenderer);
	skyBoxRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"skyBox")->transform());
	skyBoxRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"skyBox", skyBoxRenderer,
		L"SkyBoxMaterial", L"Sphere");
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
	if (auto gameObject = _activeScene->Find(name))
	{
		gameObject->AddComponent(component);
	}

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
		uiElem->SetAttribute("posX", pos.x);
		uiElem->SetAttribute("posY", pos.y);
		uiElem->SetAttribute("sizeX", size.x);
		uiElem->SetAttribute("sizeY", size.y);
		gameObj->InsertEndChild(uiElem);
	}
	else if (auto moveObject = dynamic_pointer_cast<MoveObject>(component))
	{
		tinyxml2::XMLElement* scriptElem = doc.NewElement("Script");
		scriptElem->SetAttribute("type", "MoveObject");
		scriptElem->SetAttribute("speed", moveObject->GetSpeed());
		gameObj->InsertEndChild(scriptElem);
	}
	else if (auto particleSystem = dynamic_pointer_cast<ParticleSystem>(component))
	{
		tinyxml2::XMLElement* particleElem = doc.NewElement("ParticleSystem");
		gameObj->InsertEndChild(particleElem);
	}
	// 다른 MonoBehaviour 스크립트들도 여기에 추가...
	doc.SaveFile(pathStr.c_str());
}

void SceneManager::UpdateGameObjectTransformInXML(const wstring& sceneName, const wstring& objectName, const Vec3& position, const Vec3& rotation, const Vec3& scale)
{
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
				break;
			}
		}
	}
}


void SceneManager::SaveGameObjectToXML(const wstring& path, const wstring& name, const Vec3* position, const Vec3* rotation, const Vec3* scale, const shared_ptr<GameObject>& parent)
{
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

void SceneManager::CreateCubeToScene(const wstring& sceneName)
{
	SaveAndLoadGameObjectToXML(sceneName, L"cube1",
		Vec3(0.0f, 0.0f, 0.0f));
	auto cubeRenderer = make_shared<MeshRenderer>();
	cubeRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Cube"));
	cubeRenderer->SetModel(nullptr);
	cubeRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"SolidWhiteMaterial"));
	cubeRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	cubeRenderer->AddRenderPass();
	cubeRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
	cubeRenderer->GetRenderPasses()[0]->SetMeshRenderer(cubeRenderer);
	cubeRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"cube1")->transform());
	cubeRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"cube1", cubeRenderer,
		L"SolidWhiteMaterial", L"Cube");
	auto boxCollider = make_shared<BoxCollider>();
	boxCollider->SetScale(Vec3(1.0f, 1.0f, 1.0f));
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"cube1", boxCollider);
}

void SceneManager::CreateSphereToScene(const wstring& sceneName)
{
	SaveAndLoadGameObjectToXML(sceneName, L"sphere1",
		Vec3(0.0f, 0.0f, 0.0f));
	auto cubeRenderer = make_shared<MeshRenderer>();
	cubeRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Sphere"));
	cubeRenderer->SetModel(nullptr);
	cubeRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"SolidWhiteMaterial"));
	cubeRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	cubeRenderer->AddRenderPass();
	cubeRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
	cubeRenderer->GetRenderPasses()[0]->SetMeshRenderer(cubeRenderer);
	cubeRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"sphere1")->transform());
	cubeRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"sphere1", cubeRenderer,
		L"SolidWhiteMaterial", L"Sphere");
	auto sphereCollider = make_shared<SphereCollider>();
	sphereCollider->SetScale(Vec3(1.0f, 1.0f, 1.0f));
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"sphere1", sphereCollider);
}

void SceneManager::CreateCylinderToScene(const wstring& sceneName)
{
	SaveAndLoadGameObjectToXML(sceneName, L"cylinder1",
		Vec3(0.0f, 0.0f, 0.0f));
	auto cubeRenderer = make_shared<MeshRenderer>();
	cubeRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Cylinder"));
	cubeRenderer->SetModel(nullptr);
	cubeRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"SolidWhiteMaterial"));
	cubeRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
	cubeRenderer->AddRenderPass();
	cubeRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
	cubeRenderer->GetRenderPasses()[0]->SetMeshRenderer(cubeRenderer);
	cubeRenderer->GetRenderPasses()[0]->SetTransform(_activeScene->Find(L"cylinder1")->transform());
	cubeRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"cylinder1", cubeRenderer,
		L"SolidWhiteMaterial", L"Cylinder");
	auto boxCollider = make_shared<BoxCollider>();
	boxCollider->SetScale(Vec3(0.866f, 3.0f, 1.0f));
	AddComponentToGameObjectAndSaveToXML(L"test_scene", L"cylinder1", boxCollider);
}
