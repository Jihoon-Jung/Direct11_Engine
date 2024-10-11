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
	LoadTestScene();
}
 
void SceneManager::LoadTestScene()
{
	_activeScene = make_shared<Scene>();
	
	shared_ptr<GameObject> camera = make_shared<GameObject>();
	shared_ptr<Transform> camera_transform = make_shared<Transform>();
	camera_transform->SetLocalPosition(Vec3(0, 2.f, 100));  
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
	

	shared_ptr<GameObject> env_sphere = make_shared<GameObject>();
	env_sphere->SetObjectType(GameObjectType::NormalObject);
	shared_ptr<Transform> env_sphere_transform = make_shared<Transform>();

	shared_ptr<GameObject> sphere = make_shared<GameObject>();
	sphere->SetObjectType(GameObjectType::NormalObject);
	shared_ptr<Transform> sphere_transform = make_shared<Transform>();

	shared_ptr<GameObject> skyBox = make_shared<GameObject>();
	skyBox->SetObjectType(GameObjectType::NormalObject);
	shared_ptr<Transform> skyBox_transform = make_shared<Transform>();

	shared_ptr<GameObject> cube = make_shared<GameObject>();
	cube->SetObjectType(GameObjectType::NormalObject);
	shared_ptr<Transform> cube_transform = make_shared<Transform>();

	shared_ptr<GameObject> tessellation_cylinder = make_shared<GameObject>();
	tessellation_cylinder->SetObjectType(GameObjectType::NormalObject);
	shared_ptr<Transform> tessellation_cylinder_transform = make_shared<Transform>();

	shared_ptr<GameObject> testPlane = make_shared<GameObject>();
	testPlane->SetObjectType(GameObjectType::NormalObject);
	shared_ptr<Transform> testPlane_transform = make_shared<Transform>();

	shared_ptr<GameObject> stencil_cube1 = make_shared<GameObject>();
	stencil_cube1->SetObjectType(GameObjectType::NormalObject);
	shared_ptr<Transform> stencil_cube1_transform = make_shared<Transform>();

	shared_ptr<GameObject> stencil_cube2 = make_shared<GameObject>();
	stencil_cube2->SetObjectType(GameObjectType::NormalObject);
	shared_ptr<Transform> stencil_cube2_transform = make_shared<Transform>();

	shared_ptr<GameObject> grid = make_shared<GameObject>();
	grid->SetObjectType(GameObjectType::NormalObject);
	shared_ptr<Transform> grid_transform = make_shared<Transform>();

	shared_ptr<GameObject> terrain = make_shared<GameObject>();
	terrain->SetObjectType(GameObjectType::NormalObject);
	shared_ptr<Transform> terrain_tranform = make_shared<Transform>();

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

	shared_ptr<GameObject> Ely = make_shared<GameObject>();
	Ely->SetObjectType(GameObjectType::NormalObject);
	shared_ptr<Transform> Ely_transform = make_shared<Transform>();

	shared_ptr<GameObject> particleSystem = make_shared<GameObject>();
	particleSystem->SetObjectType(GameObjectType::NormalObject);
	
	{
		env_sphere_transform->SetPosition(Vec3(4.0f, 1.f, 122.f));
		env_sphere->AddComponent(env_sphere_transform);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"DefaultMaterial"));
		meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"TestMesh"));
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		meshRenderer->AddRenderPass();
		meshRenderer->GetRenderPasses()[0]->SetPass(Pass::ENVIRONMENTMAP_RENDER);
		meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
		meshRenderer->GetRenderPasses()[0]->SetTransform(env_sphere_transform);
		meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
		meshRenderer->SetUseEnvironmentMap(true);
		env_sphere->AddComponent(meshRenderer);
		shared_ptr<SphereCollider> sphereCollider = make_shared<SphereCollider>();
		sphereCollider->SetRadius(0.5f);
		env_sphere->AddComponent(sphereCollider);
		env_sphere->SetName(L"Env_Sphere");
	}
	{
		sphere_transform->SetPosition(Vec3(-6.5f, 5.6f, 122.f));
		sphere->AddComponent(sphere_transform);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"DefaultMaterial"));
		meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"TestMesh"));
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		meshRenderer->AddRenderPass();
		meshRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
		meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
		meshRenderer->GetRenderPasses()[0]->SetTransform(sphere_transform);
		meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
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
		meshRenderer->AddRenderPass();
		meshRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
		meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
		meshRenderer->GetRenderPasses()[0]->SetTransform(skyBox_transform);
		meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
		
		skyBox->AddComponent(meshRenderer);
		skyBox->SetName(L"skyBox");
	}
	{
		cube_transform->SetPosition(Vec3(-6.5f, 0.6f, 122.f));
		cube->AddComponent(cube_transform);
		//cube->SetParent(sphere);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"DefaultMaterial"));
		//shared_ptr<Shader> computeShader = RESOURCE.GetResource<Shader>(L"AdjustTexture_Shader");
		//shared_ptr<Texture> inputTexture = meshRenderer->GetMaterial()->GetTexture();
		//ComPtr<ID3D11ShaderResourceView> result = meshRenderer->GetMaterial()->AdjustTexture(computeShader, inputTexture);
		//meshRenderer->GetMaterial()->GetTexture()->SetShaderResourceView(result);
		meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Cube"));
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		meshRenderer->AddRenderPass();
		meshRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
		meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
		meshRenderer->GetRenderPasses()[0]->SetTransform(cube_transform);
		meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
		//meshRenderer->SetUseEnvironmentMap(true);
		/*meshRenderer->AddRenderPass();
		meshRenderer->GetRenderPasses()[1]->SetPass(Pass::QUAD_RENDER);
		meshRenderer->GetRenderPasses()[1]->SetMesh(RESOURCE.GetResource<Mesh>(L"Quard"));
		meshRenderer->GetRenderPasses()[1]->SetShader(RESOURCE.GetResource<Shader>(L"Quad_Shader"));*/


		cube->AddComponent(meshRenderer);
		shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
		cube->AddComponent(boxCollider);
		cube->SetName(L"cube");
	}
	{
		tessellation_cylinder_transform->SetPosition(Vec3(-7.0f, 1.5f, 120.0f));
		tessellation_cylinder->AddComponent(tessellation_cylinder_transform);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"Tessellation_Material"));
		meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Cylinder"));
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		meshRenderer->AddRenderPass();
		meshRenderer->GetRenderPasses()[0]->SetPass(Pass::TESSELLATION_RENDER);
		meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
		meshRenderer->GetRenderPasses()[0]->SetTransform(tessellation_cylinder_transform);
		meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
		tessellation_cylinder->AddComponent(meshRenderer);

		shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
		tessellation_cylinder->AddComponent(boxCollider);
		tessellation_cylinder->SetName(L"tessellation_cube");
	}
	{
		testPlane_transform->SetPosition(Vec3(7.0f, 0.f, 0.f));
		testPlane_transform->SetRotation(Vec3(0.f, 90.f, 0.f));
		testPlane->AddComponent(testPlane_transform);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"DefaultMaterial"));
		meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Quard"));
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		meshRenderer->AddRenderPass();
		meshRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
		meshRenderer->GetRenderPasses()[0]->SetRasterizerStates(meshRenderer->GetRasterzerStates());
		meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
		meshRenderer->GetRenderPasses()[0]->SetTransform(testPlane_transform);
		meshRenderer->GetRenderPasses()[0]->SetMesh(meshRenderer->GetMesh());
		meshRenderer->GetRenderPasses()[0]->SetTexture(meshRenderer->GetMaterial()->GetTexture());
		meshRenderer->GetRenderPasses()[0]->SetNormalMap(meshRenderer->GetMaterial()->GetNormalMap());
		meshRenderer->GetRenderPasses()[0]->SetSpecularMap(meshRenderer->GetMaterial()->GetSpecularMap());
		meshRenderer->GetRenderPasses()[0]->SetDiffuseMap(meshRenderer->GetMaterial()->GetDiffuseMap());
		meshRenderer->GetRenderPasses()[0]->SetShader(meshRenderer->GetMaterial()->GetShader());

		testPlane->AddComponent(meshRenderer);
		testPlane->SetName(L"testPlane");
	}
	

	{
		stencil_cube1_transform->SetPosition(Vec3(-10.5f, 0.f, 120.f));
		stencil_cube1_transform->SetLocalScale(Vec3(1.0f));
		stencil_cube1->AddComponent(stencil_cube1_transform);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"DefaultMaterial"));
		meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Cube"));
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		meshRenderer->AddRenderPass();
		meshRenderer->GetRenderPasses()[0]->SetPass(Pass::OUTLINE_RENDER);
		meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
		meshRenderer->GetRenderPasses()[0]->SetTransform(stencil_cube1_transform);
		meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::CUSTOM1);
		stencil_cube1->AddComponent(meshRenderer);
		stencil_cube1->SetName(L"stencil_cube1");
	}
	{
		stencil_cube2_transform->SetPosition(Vec3(-10.5f, 0.f, 120.f));
		stencil_cube2_transform->SetScale(Vec3(1.1f));
		stencil_cube2->AddComponent(stencil_cube2_transform);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"LightMaterial"));
		meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Cube"));
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		meshRenderer->AddRenderPass();
		meshRenderer->GetRenderPasses()[0]->SetPass(Pass::OUTLINE_RENDER);
		meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
		meshRenderer->GetRenderPasses()[0]->SetTransform(stencil_cube2_transform);
		meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::CUSTOM2);
		stencil_cube2->AddComponent(meshRenderer);
		stencil_cube2->SetName(L"stencil_cube2");
	}

	{
		grid_transform->SetPosition(Vec3(5.0f, 0.0f, 0.0f));
		grid->AddComponent(grid_transform);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"DefaultMaterial"));
		// change color by compute shader
		/*shared_ptr<Shader> computeShader = RESOURCE.GetResource<Shader>(L"AdjustTexture_Shader");
		shared_ptr<Texture> inputTexture = meshRenderer->GetMaterial()->GetTexture();
		ComPtr<ID3D11ShaderResourceView> result = meshRenderer->GetMaterial()->AdjustTexture(computeShader, inputTexture);
		meshRenderer->GetMaterial()->GetTexture()->SetShaderResourceView(result);*/
		meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Grid"));
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		
		meshRenderer->AddRenderPass();
		meshRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
		meshRenderer->GetRenderPasses()[0]->SetRasterizerStates(meshRenderer->GetRasterzerStates());
		meshRenderer->GetRenderPasses()[0]->SetMesh(meshRenderer->GetMesh());
		meshRenderer->GetRenderPasses()[0]->SetTexture(meshRenderer->GetMaterial()->GetTexture());
		meshRenderer->GetRenderPasses()[0]->SetNormalMap(meshRenderer->GetMaterial()->GetNormalMap());
		meshRenderer->GetRenderPasses()[0]->SetSpecularMap(meshRenderer->GetMaterial()->GetSpecularMap());
		meshRenderer->GetRenderPasses()[0]->SetDiffuseMap(meshRenderer->GetMaterial()->GetDiffuseMap());
		meshRenderer->GetRenderPasses()[0]->SetShader(meshRenderer->GetMaterial()->GetShader());


		grid->AddComponent(meshRenderer);
		//shared_ptr<Terrain> terrain = make_shared<Terrain>();
		//grid->AddComponent(terrain);
		//grid->GetComponent<Terrain>()->GetGridMesh(RESOURCE.GetResource<Mesh>(L"Grid"));
		grid->SetName(L"grid");
	}

	{
		terrain_tranform->SetPosition(Vec3(0.0f, 0.0f, 0.0f));
		terrain->AddComponent(terrain_tranform);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"TerrainMaterial"));
		meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Terrain"));
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);

		meshRenderer->AddRenderPass();
		meshRenderer->GetRenderPasses()[0]->SetPass(Pass::TERRAIN_RENDER);
		meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
		meshRenderer->GetRenderPasses()[0]->SetTransform(terrain_tranform);
		meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
		terrain->AddComponent(meshRenderer);
		terrain->SetName(L"Terrain_obj");
		/*shared_ptr<Terrain> terrainComponent = make_shared<Terrain>();
		terrain->AddComponent(terrainComponent);
		terrain->GetComponent<Terrain>()->GetGridMesh(RESOURCE.GetResource<Mesh>(L"Terrain"));*/

	}

	{
		quard_transform->SetPosition(Vec3(0.0f, 0.f, 0.f));
		quard->AddComponent(quard_transform);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"Billboard_Material"));
		meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"Quard"));
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		meshRenderer->AddRenderPass();
		meshRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
		meshRenderer->GetRenderPasses()[0]->SetRasterizerStates(meshRenderer->GetRasterzerStates());
		meshRenderer->GetRenderPasses()[0]->SetMesh(meshRenderer->GetMesh());
		meshRenderer->GetRenderPasses()[0]->SetTexture(meshRenderer->GetMaterial()->GetTexture());
		meshRenderer->GetRenderPasses()[0]->SetNormalMap(meshRenderer->GetMaterial()->GetNormalMap());
		meshRenderer->GetRenderPasses()[0]->SetSpecularMap(meshRenderer->GetMaterial()->GetSpecularMap());
		meshRenderer->GetRenderPasses()[0]->SetDiffuseMap(meshRenderer->GetMaterial()->GetDiffuseMap());
		meshRenderer->GetRenderPasses()[0]->SetShader(meshRenderer->GetMaterial()->GetShader());
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
		meshRenderer->AddRenderPass();
		meshRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
		meshRenderer->GetRenderPasses()[0]->SetRasterizerStates(meshRenderer->GetRasterzerStates());
		meshRenderer->GetRenderPasses()[0]->SetMesh(meshRenderer->GetMesh());
		meshRenderer->GetRenderPasses()[0]->SetTexture(meshRenderer->GetMaterial()->GetTexture());
		meshRenderer->GetRenderPasses()[0]->SetNormalMap(meshRenderer->GetMaterial()->GetNormalMap());
		meshRenderer->GetRenderPasses()[0]->SetSpecularMap(meshRenderer->GetMaterial()->GetSpecularMap());
		meshRenderer->GetRenderPasses()[0]->SetDiffuseMap(meshRenderer->GetMaterial()->GetDiffuseMap());
		meshRenderer->GetRenderPasses()[0]->SetShader(meshRenderer->GetMaterial()->GetShader());
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
		meshRenderer->AddRenderPass();
		meshRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
		meshRenderer->GetRenderPasses()[0]->SetRasterizerStates(meshRenderer->GetRasterzerStates());
		meshRenderer->GetRenderPasses()[0]->SetMesh(meshRenderer->GetMesh());
		meshRenderer->GetRenderPasses()[0]->SetTexture(meshRenderer->GetMaterial()->GetTexture());
		meshRenderer->GetRenderPasses()[0]->SetNormalMap(meshRenderer->GetMaterial()->GetNormalMap());
		meshRenderer->GetRenderPasses()[0]->SetSpecularMap(meshRenderer->GetMaterial()->GetSpecularMap());
		meshRenderer->GetRenderPasses()[0]->SetDiffuseMap(meshRenderer->GetMaterial()->GetDiffuseMap());
		meshRenderer->GetRenderPasses()[0]->SetShader(meshRenderer->GetMaterial()->GetShader());
		uiButton->AddComponent(meshRenderer);
		uiButton->SetName(L"UI_Button");
	}
	{
		tower_transform->SetPosition(Vec3(0.f, 0.f, 130.f));
		tower_transform->SetLocalScale(Vec3(0.01f));
		tower->AddComponent(tower_transform);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetModel(RESOURCE.GetResource<Model>(L"TowerModel"));
		meshRenderer->SetMaterial(meshRenderer->GetModel()->GetMaterials()[0]);
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		meshRenderer->AddRenderPass();
		meshRenderer->GetRenderPasses()[0]->SetPass(Pass::STATIC_MESH_RENDER);
		meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
		meshRenderer->GetRenderPasses()[0]->SetTransform(tower_transform);
		meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
		tower->AddComponent(meshRenderer);
		shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
		tower->AddComponent(boxCollider);
		tower->SetName(L"tower");
	}
	{
		float angle = 50.0f; // 각도 단위는 도(degrees)
		float r = 18.0277557f; // Bounding Sphere의 반지름을 250으로 설정함. (너무 크면 shadow map에 문제가 생겨서 임의로 작게 만듬)
		float x, y, z;

		// 각도를 라디안으로 변환
		float angleRad = angle * XM_PI / 180.0f;

		// 변 x와 y를 계산
		x = r * cos(angleRad);
		y = r * sin(angleRad);
		z = 0.0f;
		light_transform->SetPosition(Vec3(x, y, z));//SetPosition(Vec3(-500.0f, 400.0f, 420.0f));
		light_transform->SetScale(Vec3(10.0f, 10.0f, 10.0f));
		light->AddComponent(light_transform);
		light->AddComponent(make_shared<Light>());
		/*shared_ptr<MoveObject> moveObject = make_shared<MoveObject>();
		light->AddComponent(moveObject);*/
		/*shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"LightMaterial"));
		meshRenderer->SetMesh(RESOURCE.GetResource<Mesh>(L"TestMesh"));
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		meshRenderer->AddRenderPass();
		meshRenderer->GetRenderPasses()[0]->SetPass(Pass::DEFAULT_RENDER);
		meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
		meshRenderer->GetRenderPasses()[0]->SetTransform(light_transform);
		meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
		light->AddComponent(meshRenderer);*/
		light->SetName(L"MainLight");
	}
	{
		kachujin_transform->SetPosition(Vec3(3.0f, 0.0f, 117.0f));
		kachujin_transform->SetLocalScale(Vec3(0.01f));
		Kachujin->AddComponent(kachujin_transform);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetModel(RESOURCE.GetResource<Model>(L"Kachujin"));
		meshRenderer->SetMaterial(meshRenderer->GetModel()->GetMaterials()[0]);
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		meshRenderer->AddRenderPass();
		meshRenderer->GetRenderPasses()[0]->SetPass(Pass::ANIMATED_MESH_RENDER);
		meshRenderer->GetRenderPasses()[0]->SetMeshRenderer(meshRenderer);
		meshRenderer->GetRenderPasses()[0]->SetTransform(kachujin_transform);
		meshRenderer->GetRenderPasses()[0]->SetDepthStencilStateType(DSState::NORMAL);
		Kachujin->AddComponent(meshRenderer);
		shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
		Kachujin->AddComponent(boxCollider);
		Kachujin->SetName(L"Kachujin_OBJ");
	}
	{
		Ely_transform->SetPosition(Vec3(-9.0f, 5.0f, 0.0f));
		Ely_transform->SetLocalScale(Vec3(0.1f));
		Ely->AddComponent(Ely_transform);
		shared_ptr<MeshRenderer> meshRenderer = make_shared<MeshRenderer>();
		meshRenderer->SetModel(RESOURCE.GetResource<Model>(L"Ely"));
		meshRenderer->SetMaterial(meshRenderer->GetModel()->GetMaterials()[0]);
		meshRenderer->SetRasterzierState(D3D11_FILL_SOLID, D3D11_CULL_BACK, false);
		Ely->AddComponent(meshRenderer);
		shared_ptr<BoxCollider> boxCollider = make_shared<BoxCollider>();
		Ely->AddComponent(boxCollider);
		Ely->SetName(L"Ely_OBJ");
	}
	{
		shared_ptr<ParticleSystem> particleComponent = make_shared<ParticleSystem>();
		particleComponent->Init(RESOURCE.GetResource<Texture>(L"Fire_Particle")->GetShaderResourceView(), RESOURCE.GetResource<Texture>(L"Random_Texture")->GetShaderResourceView(), 500);
		particleSystem->AddComponent(particleComponent);
		particleSystem->SetName(L"FireParticle");
	}
	int parentCount = cube->transform()->ParentCount();
	int childCount = env_sphere->transform()->ChildCount();

	_activeScene->AddGameObject(camera);
	_activeScene->AddGameObject(uiCamera);
	_activeScene->AddGameObject(light);
	_activeScene->AddGameObject(skyBox);
	_activeScene->AddGameObject(env_sphere);
	_activeScene->AddGameObject(sphere);
	_activeScene->AddGameObject(cube);
	_activeScene->AddGameObject(tessellation_cylinder);
	//_activeScene->AddGameObject(grid);
	_activeScene->AddGameObject(tower);
	//_activeScene->AddGameObject(quard_ui);
	//_activeScene->AddGameObject(uiButton);
	_activeScene->AddGameObject(stencil_cube1);
	_activeScene->AddGameObject(stencil_cube2);
	//_activeScene->AddGameObject(quard);
	_activeScene->AddGameObject(Kachujin);
	_activeScene->AddGameObject(terrain);
	_activeScene->AddGameObject(particleSystem);
	//_activeScene->AddGameObject(Ch4);
	//_activeScene->AddGameObject(anna);
	//_activeScene->AddGameObject(house);
	//_activeScene->AddGameObject(testPlane);
}
