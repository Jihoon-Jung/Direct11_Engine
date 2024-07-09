#include "pch.h"
#include "SceneManager.h"
#include "GameObject.h"
#include "MoveObject.h"

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
	LoadTestScene();
}
 
void SceneManager::LoadTestScene()
{
	_activeScene = make_shared<Scene>();
	
	shared_ptr<GameObject> camera = make_shared<GameObject>();
	shared_ptr<Transform> camera_transform = make_shared<Transform>();
	camera_transform->SetLocalPosition(Vec3(0, 0, -3));
	camera->AddComponent(camera_transform);
	shared_ptr<MoveObject> moveObject = make_shared<MoveObject>();
	camera->AddComponent(moveObject);
	shared_ptr<Camera> cameraComponent = make_shared<Camera>();
	cameraComponent->SetProjectionType(ProjectionType::Perspective);
	camera->AddComponent(cameraComponent);
	camera->SetName(L"MainCamera");

	shared_ptr<GameObject> uiCamera = make_shared<GameObject>();
	shared_ptr<Transform> uiCamera_transform = make_shared<Transform>();
	uiCamera_transform->SetLocalPosition(Vec3(0, 0, -3));
	uiCamera->AddComponent(uiCamera_transform);
	shared_ptr<Camera> uiCameraComponent = make_shared<Camera>();
	uiCameraComponent->SetProjectionType(ProjectionType::Orthographic);
	uiCamera->AddComponent(uiCameraComponent);
	uiCamera->SetName(L"UICamera");

	shared_ptr<GameObject> light = make_shared<GameObject>();
	light->SetObjectType(GameObjectType::NormalObject);
	shared_ptr<Transform> light_transform = make_shared<Transform>();
	

	shared_ptr<GameObject> sphere = make_shared<GameObject>();
	sphere->SetObjectType(GameObjectType::NormalObject);
	shared_ptr<Transform> sphere_transform = make_shared<Transform>();

	shared_ptr<GameObject> skyBox = make_shared<GameObject>();
	skyBox->SetObjectType(GameObjectType::NormalObject);
	shared_ptr<Transform> skyBox_transform = make_shared<Transform>();

	shared_ptr<GameObject> cube = make_shared<GameObject>();
	cube->SetObjectType(GameObjectType::NormalObject);
	shared_ptr<Transform> cube_transform = make_shared<Transform>();

	shared_ptr<GameObject> grid = make_shared<GameObject>();
	grid->SetObjectType(GameObjectType::NormalObject);
	shared_ptr<Transform> grid_transform = make_shared<Transform>();

	shared_ptr<GameObject> quard = make_shared<GameObject>();
	quard->SetObjectType(GameObjectType::NormalObject);
	shared_ptr<Transform> quard_transform = make_shared<Transform>();

	shared_ptr<GameObject> quard_ui = make_shared<GameObject>();
	quard_ui->SetObjectType(GameObjectType::UIObject);
	shared_ptr<Transform> quard_ui_transform = make_shared<Transform>();

	shared_ptr<GameObject> uiButton = make_shared<GameObject>();
	uiButton->SetObjectType(GameObjectType::UIObject);
	shared_ptr<Transform> uiButton_transform = make_shared<Transform>();

	shared_ptr<GameObject> tower = make_shared<GameObject>();
	tower->SetObjectType(GameObjectType::NormalObject);
	shared_ptr<Transform> tower_transform = make_shared<Transform>();

	shared_ptr<GameObject> Kachujin = make_shared<GameObject>();
	Kachujin->SetObjectType(GameObjectType::NormalObject);
	shared_ptr<Transform> kachujin_transform = make_shared<Transform>();

	{
		sphere_transform->SetPosition(Vec3(-1.0f, 0.f, 2.f));
		sphere->AddComponent(sphere_transform);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"DefaultMaterial"));
		meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"TestMesh"));
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		sphere->AddComponent(meshRenderer);
		shared_ptr<SphereCollider> sphereCollider = make_shared<SphereCollider>();
		sphereCollider->SetRadius(0.5f);
		sphere->AddComponent(sphereCollider);
		sphere->SetName(L"Sphere");
	}
	{
		skyBox_transform->SetPosition(Vec3(0.f, 0.f, 0.f));
		skyBox->AddComponent(skyBox_transform);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"SkyBoxMaterial"));
		meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"TestMesh"));
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, true);
		skyBox->AddComponent(meshRenderer);
		skyBox->SetName(L"skyBox");
	}
	{
		cube_transform->SetPosition(Vec3(0.5f, 0.f, 2.f));
		cube->AddComponent(cube_transform);
		//cube->SetParent(sphere);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"DefaultMaterial"));
		shared_ptr<Shader> computeShader = RESOURCE.GetResource<Shader>(L"AdjustTexture_Shader");
		shared_ptr<Texture> inputTexture = meshRenderer->GetMaterial()->GetTexture();
		ComPtr<ID3D11ShaderResourceView> result = meshRenderer->GetMaterial()->AdjustTexture(computeShader, inputTexture);
		meshRenderer->GetMaterial()->GetTexture()->SetShaderResourceView(result);
		meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Cube"));
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		cube->AddComponent(meshRenderer);
		shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
		cube->AddComponent(boxCollider);
		cube->SetName(L"cube");
	}
	{
		grid_transform->SetPosition(Vec3(5.0f, 0.0f, 0.0f));
		grid->AddComponent(grid_transform);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"DefaultMaterial"));
		shared_ptr<Shader> computeShader = RESOURCE.GetResource<Shader>(L"AdjustTexture_Shader");
		shared_ptr<Texture> inputTexture = meshRenderer->GetMaterial()->GetTexture();
		ComPtr<ID3D11ShaderResourceView> result = meshRenderer->GetMaterial()->AdjustTexture(computeShader, inputTexture);
		meshRenderer->GetMaterial()->GetTexture()->SetShaderResourceView(result);
		meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Grid"));
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		grid->AddComponent(meshRenderer);
		shared_ptr<Terrain> terrain = make_shared<Terrain>();
		grid->AddComponent(terrain);
		grid->GetComponent<Terrain>()->GetGridMesh(RESOURCE.GetResource<Mesh>(L"Grid"));
		grid->SetName(L"grid");
	}
	{
		quard_transform->SetPosition(Vec3(0.0f, 0.f, 0.f));
		quard->AddComponent(quard_transform);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"Billboard_Material"));
		meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Quard"));
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		quard->AddComponent(meshRenderer);
		quard->SetName(L"Quard_Billboard");
	}
	{
		shared_ptr<UIImage> uiImageComponent = make_shared<UIImage>();
		uiImageComponent->SetTransformAndRect(Vec2(700, 100), Vec2(100, 100));
		quard_ui->AddComponent(uiImageComponent);
		quard_ui_transform->SetPosition(uiImageComponent->GetNDCPosition());
		quard_ui_transform->SetLocalScale(uiImageComponent->GetSize());
		quard_ui->AddComponent(quard_ui_transform);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"DefaultMaterial"));
		meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Quard"));
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		quard_ui->AddComponent(meshRenderer);
		quard_ui->SetName(L"UI_Quard");
	}
	{
		shared_ptr<Button> buttonComponent = make_shared<Button>();
		buttonComponent->AddOnClickedEvent([this, uiButton]() {_activeScene->RemoveGameObject(uiButton); });
		buttonComponent->SetTransformAndRect(Vec2(100, 100), Vec2(100, 100));
		uiButton->AddComponent(buttonComponent);
		uiButton_transform->SetPosition(buttonComponent->GetNDCPosition());
		uiButton_transform->SetLocalScale(buttonComponent->GetSize());
		uiButton->AddComponent(uiButton_transform);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"DefaultMaterial"));
		meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Quard"));
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		uiButton->AddComponent(meshRenderer);
		uiButton->SetName(L"UI_Button");
	}
	{
		tower_transform->SetPosition(Vec3(0.0f, 0.0f, -1.0f));
		tower_transform->SetLocalScale(Vec3(0.01f));
		tower->AddComponent(tower_transform);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetModel(RESOURCE.GetResource<Model>(L"TowerModel"));
		meshRenderer->SetMaterial(meshRenderer->GetModel()->GetMaterials()[0]);
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		tower->AddComponent(meshRenderer);
		shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
		tower->AddComponent(boxCollider);
		tower->SetName(L"tower");
	}

	{
		light_transform->SetPosition(Vec3(-5.f, 0.f, -2.f));
		light->AddComponent(light_transform);
		light->AddComponent(make_shared<Light>());
		/*shared_ptr<MoveObject> moveObject = make_shared<MoveObject>();
		light->AddComponent(moveObject);*/
		/*shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"LightMaterial"));
		meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"TestMesh"));
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		light->AddComponent(meshRenderer);*/
		light->SetName(L"MainLight");
	}
	{
		kachujin_transform->SetPosition(Vec3(0.0f, 0.0f, 0.0f));
		kachujin_transform->SetLocalScale(Vec3(0.01f));
		Kachujin->AddComponent(kachujin_transform);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetModel(RESOURCE.GetResource<Model>(L"Kachujin"));
		meshRenderer->SetMaterial(meshRenderer->GetModel()->GetMaterials()[0]);
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		Kachujin->AddComponent(meshRenderer);
		shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
		Kachujin->AddComponent(boxCollider);
		Kachujin->SetName(L"Kachujin_OBJ");
	}
	int parentCount = cube->transform()->ParentCount();
	int childCount = sphere->transform()->ChildCount();

	_activeScene->AddGameObject(camera);
	_activeScene->AddGameObject(uiCamera);
	_activeScene->AddGameObject(light);
	//_activeScene->AddGameObject(skyBox);
	//_activeScene->AddGameObject(sphere);
	//_activeScene->AddGameObject(cube);
	//_activeScene->AddGameObject(grid);
	//_activeScene->AddGameObject(tower);
	//_activeScene->AddGameObject(quard_ui);
	//_activeScene->AddGameObject(uiButton);
	_activeScene->AddGameObject(quard);
	//_activeScene->AddGameObject(Kachujin);
}
