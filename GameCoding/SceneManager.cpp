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
	camera->AddComponent(make_shared<Camera>());
	camera->SetName(L"MainCamera");

	shared_ptr<GameObject> light = make_shared<GameObject>();
	shared_ptr<Transform> light_transform = make_shared<Transform>();
	

	shared_ptr<GameObject> sphere = make_shared<GameObject>();
	shared_ptr<Transform> sphere_transform = make_shared<Transform>();
	shared_ptr<GameObject> skyBox = make_shared<GameObject>();
	shared_ptr<Transform> skyBox_transform = make_shared<Transform>();
	shared_ptr<GameObject> cube = make_shared<GameObject>();
	shared_ptr<Transform> cube_transform = make_shared<Transform>();
	shared_ptr<GameObject> tower = make_shared<GameObject>();
	shared_ptr<Transform> tower_transform = make_shared<Transform>();
	shared_ptr<GameObject> Kachujin = make_shared<GameObject>();
	shared_ptr<Transform> kachujin_transform = make_shared<Transform>();

	{
		sphere_transform->SetPosition(Vec3(0.f, 0.f, 0.f));
		sphere->AddComponent(sphere_transform);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"DefaultMaterial"));
		meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"TestMesh"));
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		sphere->AddComponent(meshRenderer);
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
		meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Cube"));
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		cube->AddComponent(meshRenderer);
		cube->SetName(L"cube");
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
		Kachujin->SetName(L"Kachujin_OBJ");
	}
	int parentCount = cube->transform()->ParentCount();
	int childCount = sphere->transform()->ChildCount();

	_activeScene->AddGameObject(camera);
	_activeScene->AddGameObject(light);
	_activeScene->AddGameObject(skyBox);
	_activeScene->AddGameObject(sphere);
	_activeScene->AddGameObject(cube);
	_activeScene->AddGameObject(tower);
	_activeScene->AddGameObject(Kachujin);
}
