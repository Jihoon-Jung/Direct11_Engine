#include "pch.h"
#include "ResourceManager.h"

void ResourceManager::Init()
{
	AddResource();
}

void ResourceManager::AddResource()
{
	shared_ptr<Mesh> sphere_mesh = make_shared<Mesh>();
	shared_ptr<Mesh> cube_mesh = make_shared<Mesh>();
	shared_ptr<Mesh> grid_mesh = make_shared<Mesh>();
	shared_ptr<Mesh> quard_mesh = make_shared<Mesh>();

	shared_ptr<Model> tower_model = make_shared<Model>();
	shared_ptr<Model> kachujin_Anim = make_shared<Model>();

	shared_ptr<Texture> texture = make_shared<Texture>();
	shared_ptr<Texture> normalMap = make_shared<Texture>();
	shared_ptr<Texture> lightTexture = make_shared<Texture>();
	shared_ptr<Texture> skyBoxTexture = make_shared<Texture>();
	shared_ptr<Texture> treeTexture = make_shared<Texture>();

	shared_ptr<Texture> grassTexture = make_shared<Texture>();

	shared_ptr<Shader> ligthRenderShader = make_shared<Shader>();

	shared_ptr<Shader> defaultShader = make_shared<Shader>();

	shared_ptr<Shader> billBoardShader = make_shared<Shader>();

	shared_ptr<Shader> skyboxShader = make_shared<Shader>();

	shared_ptr<Shader> staticMesh_shader = make_shared<Shader>();

	shared_ptr<Shader> animatedMesh_shader = make_shared<Shader>();

	shared_ptr<Shader> adjustTexture_shader = make_shared<Shader>();

	shared_ptr<Material> material = make_shared<Material>();

	shared_ptr<Material> billboardMaterial = make_shared<Material>();

	shared_ptr<Material> lightMaterial = make_shared<Material>();

	shared_ptr<Material> skyBoxMaterial = make_shared<Material>();


	sphere_mesh->CreateSphere_NormalTangent();
	sphere_mesh->SetName(L"TestMesh");
	RESOURCE.AddResource(sphere_mesh->GetName(), sphere_mesh);
	
	cube_mesh->CreateCube_NormalTangent();
	cube_mesh->SetName(L"Cube");
	RESOURCE.AddResource(cube_mesh->GetName(), cube_mesh);

	grid_mesh->CreateGrid_NormalTangent(10, 10);
	grid_mesh->SetName(L"Grid");
	RESOURCE.AddResource(grid_mesh->GetName(), grid_mesh);

	quard_mesh->CreateQuard_NormalTangent();
	quard_mesh->SetName(L"Quard");
	RESOURCE.AddResource(quard_mesh->GetName(), quard_mesh);

	texture = make_shared<Texture>();
	texture->CreateTexture(L"Leather.jpg");
	texture->SetName(L"Leather");
	RESOURCE.AddResource(texture->GetName(), texture);

	grassTexture = make_shared<Texture>();
	grassTexture->CreateTexture(L"Golem.png");
	grassTexture->SetName(L"Grass");
	RESOURCE.AddResource(grassTexture->GetName(), grassTexture);

	skyBoxTexture = make_shared<Texture>();
	skyBoxTexture->CreateTexture(L"4k_Sky.jpg");
	skyBoxTexture->SetName(L"skyBox");
	RESOURCE.AddResource(skyBoxTexture->GetName(), skyBoxTexture);

	treeTexture = make_shared<Texture>();
	treeTexture->CreateTexture(L"tree.png");
	treeTexture->SetName(L"tree");
	RESOURCE.AddResource(treeTexture->GetName(), treeTexture);

	normalMap = make_shared<Texture>();
	normalMap->CreateTexture(L"Leather_Normal.jpg");
	normalMap->SetName(L"NormalMap");
	RESOURCE.AddResource(normalMap->GetName(), normalMap);

	lightTexture = make_shared<Texture>();
	lightTexture->CreateTexture(L"yellow.jpg");
	lightTexture->SetName(L"lightTexture");
	RESOURCE.AddResource(lightTexture->GetName(), lightTexture);

	defaultShader = make_shared<Shader>();
	defaultShader->CreateShader(ShaderType::VERTEX_SHADER, L"Default.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	defaultShader->CreateShader(ShaderType::PIXEL_SHADER, L"Default.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	defaultShader->SetName(L"Default_Shader");
	{
		defaultShader->GetShaderSlot()->SetSlot(L"CameraBuffer", 0);
		defaultShader->GetShaderSlot()->SetSlot(L"TransformBuffer", 1);
		defaultShader->GetShaderSlot()->SetSlot(L"LightMaterial", 2);
		defaultShader->GetShaderSlot()->SetSlot(L"LightDesc", 3);
		defaultShader->GetShaderSlot()->SetSlot(L"LightAndCameraPos", 4);
		defaultShader->GetShaderSlot()->SetSlot(L"texture0", 0);
		defaultShader->GetShaderSlot()->SetSlot(L"normalMap", 1);
		defaultShader->GetShaderSlot()->SetSlot(L"specularMap", 2);
		defaultShader->GetShaderSlot()->SetSlot(L"diffuseMap", 3);
	}
	RESOURCE.AddResource(defaultShader->GetName(), defaultShader);

	billBoardShader->CreateShader(ShaderType::VERTEX_SHADER, L"Billboard.hlsl", InputLayoutType::VertexBillboard_Geometry);
	billBoardShader->CreateShader(ShaderType::PIXEL_SHADER, L"Billboard.hlsl", InputLayoutType::VertexBillboard_Geometry);
	billBoardShader->CreateShader(ShaderType::GEOMETRY_SHADER, L"Billboard.hlsl", InputLayoutType::VertexBillboard_Geometry);
	billBoardShader->SetName(L"Billboard_Shader");
	{
		billBoardShader->GetShaderSlot()->SetSlot(L"CameraBuffer",0);
		billBoardShader->GetShaderSlot()->SetSlot(L"TransformBuffer", 1);
		billBoardShader->GetShaderSlot()->SetSlot(L"CameraPos", 2);
		billBoardShader->GetShaderSlot()->SetSlot(L"texture0", 0);
	}
	RESOURCE.AddResource(billBoardShader->GetName(), billBoardShader);

	adjustTexture_shader = make_shared<Shader>();
	adjustTexture_shader->CreateShader(ShaderType::COMPUTE_SHADER, L"ComputeShader_AdjustTexture.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	adjustTexture_shader->SetName(L"AdjustTexture_Shader");
	RESOURCE.AddResource(adjustTexture_shader->GetName(), adjustTexture_shader);

	skyboxShader = make_shared<Shader>();
	skyboxShader->CreateShader(ShaderType::VERTEX_SHADER, L"SkyBox.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	skyboxShader->CreateShader(ShaderType::PIXEL_SHADER, L"SkyBox.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	skyboxShader->SetName(L"SkyBox_Shader");
	{
		skyboxShader->GetShaderSlot()->SetSlot(L"CameraBuffer", 0);
		skyboxShader->GetShaderSlot()->SetSlot(L"texture0", 0);
	}
	RESOURCE.AddResource(skyboxShader->GetName(), skyboxShader);


	ligthRenderShader = make_shared<Shader>();
	ligthRenderShader->CreateShader(ShaderType::VERTEX_SHADER, L"Light.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	ligthRenderShader->SetName(L"Light_Render_Shader");
	ligthRenderShader->CreateShader(ShaderType::PIXEL_SHADER, L"Light.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	{
		ligthRenderShader->GetShaderSlot()->SetSlot(L"CameraBuffer", 0);
		ligthRenderShader->GetShaderSlot()->SetSlot(L"TransformBuffer", 1);
		ligthRenderShader->GetShaderSlot()->SetSlot(L"texture0", 0);
	}
	RESOURCE.AddResource(ligthRenderShader->GetName(), ligthRenderShader);


	staticMesh_shader = make_shared<Shader>();
	staticMesh_shader->CreateShader(ShaderType::VERTEX_SHADER, L"StaticMesh.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	staticMesh_shader->CreateShader(ShaderType::PIXEL_SHADER, L"StaticMesh.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	staticMesh_shader->SetName(L"StaticMesh_Shader");
	{
		staticMesh_shader->GetShaderSlot()->SetSlot(L"CameraBuffer", 0);
		staticMesh_shader->GetShaderSlot()->SetSlot(L"TransformBuffer", 1);
		staticMesh_shader->GetShaderSlot()->SetSlot(L"LightMaterial", 2);
		staticMesh_shader->GetShaderSlot()->SetSlot(L"LightDesc", 3);
		staticMesh_shader->GetShaderSlot()->SetSlot(L"LightAndCameraPos", 4);
		staticMesh_shader->GetShaderSlot()->SetSlot(L"BoneBuffer", 5);
		staticMesh_shader->GetShaderSlot()->SetSlot(L"BonIndex", 6);
		staticMesh_shader->GetShaderSlot()->SetSlot(L"texture0", 0);
		staticMesh_shader->GetShaderSlot()->SetSlot(L"normalMap", 1);
		staticMesh_shader->GetShaderSlot()->SetSlot(L"specularMap", 2);
		staticMesh_shader->GetShaderSlot()->SetSlot(L"diffuseMap", 3);
	}
	RESOURCE.AddResource(staticMesh_shader->GetName(), staticMesh_shader);


	animatedMesh_shader = make_shared<Shader>();
	animatedMesh_shader->CreateShader(ShaderType::VERTEX_SHADER, L"AnimatedMesh.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	animatedMesh_shader->CreateShader(ShaderType::PIXEL_SHADER, L"AnimatedMesh.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	animatedMesh_shader->SetName(L"AnimatedMesh_Shader");
	{
		animatedMesh_shader->GetShaderSlot()->SetSlot(L"CameraBuffer", 0);
		animatedMesh_shader->GetShaderSlot()->SetSlot(L"TransformBuffer", 1);
		animatedMesh_shader->GetShaderSlot()->SetSlot(L"LightMaterial", 2);
		animatedMesh_shader->GetShaderSlot()->SetSlot(L"LightDesc", 3);
		animatedMesh_shader->GetShaderSlot()->SetSlot(L"LightAndCameraPos", 4);
		animatedMesh_shader->GetShaderSlot()->SetSlot(L"BlendBuffer", 5);
		animatedMesh_shader->GetShaderSlot()->SetSlot(L"normalMap", 0);
		animatedMesh_shader->GetShaderSlot()->SetSlot(L"specularMap", 1);
		animatedMesh_shader->GetShaderSlot()->SetSlot(L"diffuseMap", 2);
		animatedMesh_shader->GetShaderSlot()->SetSlot(L"TransformMap", 3);
	}
	RESOURCE.AddResource(animatedMesh_shader->GetName(), animatedMesh_shader);


	material = make_shared<Material>();
	material->SetTexture(RESOURCE.GetResource<Texture>(L"Leather"));
	material->SetNormalMap(RESOURCE.GetResource<Texture>(L"NormalMap"));
	material->SetShader(RESOURCE.GetResource<Shader>(L"Default_Shader"));
	material->SetName(L"DefaultMaterial");
	RESOURCE.AddResource(material->GetName(), material);

	billboardMaterial->SetTexture(RESOURCE.GetResource<Texture>(L"Leather"));
	billboardMaterial->SetShader(RESOURCE.GetResource<Shader>(L"Billboard_Shader"));
	billboardMaterial->SetName(L"Billboard_Material");
	RESOURCE.AddResource(billboardMaterial->GetName(), billboardMaterial);

	skyBoxMaterial = make_shared<Material>();
	skyBoxMaterial->SetTexture(RESOURCE.GetResource<Texture>(L"skyBox"));
	skyBoxMaterial->SetShader(RESOURCE.GetResource<Shader>(L"SkyBox_Shader"));
	skyBoxMaterial->SetName(L"SkyBoxMaterial");
	RESOURCE.AddResource(skyBoxMaterial->GetName(), skyBoxMaterial);



	shared_ptr<Material> tmp = RESOURCE.GetResource<Material>(L"DefaultMaterial");
	lightMaterial = make_shared<Material>();
	lightMaterial->SetTexture(RESOURCE.GetResource<Texture>(L"lightTexture"));
	lightMaterial->SetShader(RESOURCE.GetResource<Shader>(L"Light_Render_Shader"));
	lightMaterial->SetName(L"LightMaterial");
	RESOURCE.AddResource(lightMaterial->GetName(), lightMaterial);

	tower_model->ReadModel(L"Tower/Tower");
	tower_model->SetShaderForMaterial(RESOURCE.GetResource<Shader>(L"StaticMesh_Shader"));
	tower_model->ReadMaterial(L"Tower/Tower");
	tower_model->SetName(L"TowerModel");
	RESOURCE.AddResource(tower_model->GetName(), tower_model);

	kachujin_Anim->ReadModel(L"Kachujin/Kachujin");
	kachujin_Anim->SetShaderForMaterial(RESOURCE.GetResource<Shader>(L"AnimatedMesh_Shader"));
	kachujin_Anim->ReadMaterial(L"Kachujin/Kachujin");
	kachujin_Anim->ReadAnimation(L"Kachujin/Idle");
	kachujin_Anim->ReadAnimation(L"Kachujin/Run");
	kachujin_Anim->ReadAnimation(L"Kachujin/Slash");
	kachujin_Anim->CreateTexture();
	kachujin_Anim->SetName(L"Kachujin");
	RESOURCE.AddResource(kachujin_Anim->GetName(), kachujin_Anim);
}
