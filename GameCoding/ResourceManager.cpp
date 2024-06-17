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
	shared_ptr<Model> tower_model = make_shared<Model>();
	shared_ptr<Model> kachujin_Anim = make_shared<Model>();

	shared_ptr<Texture> texture = make_shared<Texture>();
	shared_ptr<Texture> normalMap = make_shared<Texture>();
	shared_ptr<Texture> lightTexture = make_shared<Texture>();
	shared_ptr<Texture> skyBoxTexture = make_shared<Texture>();

	shared_ptr<Shader> ligthVertexShader = make_shared<Shader>();
	shared_ptr<Shader> lightPixelShader = make_shared<Shader>();

	shared_ptr<Shader> vertexShader = make_shared<Shader>();
	shared_ptr<Shader> pixelShader = make_shared<Shader>();

	shared_ptr<Shader> skyboxVertexShader = make_shared<Shader>();
	shared_ptr<Shader> skyboxPixelShader = make_shared<Shader>();

	shared_ptr<Shader> staticMesh_VertexShader = make_shared<Shader>();
	shared_ptr<Shader> staticMesh_PixelShader = make_shared<Shader>();

	shared_ptr<Shader> animatedMesh_VertexShader = make_shared<Shader>();
	shared_ptr<Shader> animatedMesh_PixelShader = make_shared<Shader>();

	shared_ptr<Material> material = make_shared<Material>();
	shared_ptr<Material> lightMaterial = make_shared<Material>();
	shared_ptr<Material> skyBoxMaterial = make_shared<Material>();


	sphere_mesh->CreateSphere_NormalTangent();
	sphere_mesh->SetName(L"TestMesh");
	RESOURCE.AddResource(sphere_mesh->GetName(), sphere_mesh);
	
	cube_mesh->CreateCube_NormalTangent();
	cube_mesh->SetName(L"Cube");
	RESOURCE.AddResource(cube_mesh->GetName(), cube_mesh);

	texture = make_shared<Texture>();
	texture->CreateTexture(L"Leather.jpg");
	texture->SetName(L"panda");
	RESOURCE.AddResource(texture->GetName(), texture);

	skyBoxTexture = make_shared<Texture>();
	skyBoxTexture->CreateTexture(L"4k_Sky.jpg");
	skyBoxTexture->SetName(L"skyBox");
	RESOURCE.AddResource(skyBoxTexture->GetName(), skyBoxTexture);

	normalMap = make_shared<Texture>();
	normalMap->CreateTexture(L"Leather_Normal.jpg");
	normalMap->SetName(L"NormalMap");
	RESOURCE.AddResource(normalMap->GetName(), normalMap);

	lightTexture = make_shared<Texture>();
	lightTexture->CreateTexture(L"yellow.jpg");
	lightTexture->SetName(L"lightTexture");
	RESOURCE.AddResource(lightTexture->GetName(), lightTexture);

	vertexShader = make_shared<Shader>();
	vertexShader->CreateShader(ShaderType::VERTEX_SHADER, L"Default.hlsl");
	vertexShader->SetName(L"Default_VS");
	RESOURCE.AddResource(vertexShader->GetName(), vertexShader);

	pixelShader = make_shared<Shader>();
	pixelShader->CreateShader(ShaderType::PIXEL_SHADER, L"Default.hlsl");
	pixelShader->SetName(L"Default_PS");
	RESOURCE.AddResource(pixelShader->GetName(), pixelShader);

	skyboxVertexShader = make_shared<Shader>();
	skyboxVertexShader->CreateShader(ShaderType::VERTEX_SHADER, L"SkyBox.hlsl");
	skyboxVertexShader->SetName(L"SkyBox_VS");
	RESOURCE.AddResource(skyboxVertexShader->GetName(), skyboxVertexShader);

	skyboxPixelShader = make_shared<Shader>();
	skyboxPixelShader->CreateShader(ShaderType::PIXEL_SHADER, L"SkyBox.hlsl");
	skyboxPixelShader->SetName(L"SkyBox_PS");
	RESOURCE.AddResource(skyboxPixelShader->GetName(), skyboxPixelShader);

	ligthVertexShader = make_shared<Shader>();
	ligthVertexShader->CreateShader(ShaderType::VERTEX_SHADER, L"Light.hlsl");
	ligthVertexShader->SetName(L"Light_VS");
	RESOURCE.AddResource(ligthVertexShader->GetName(), ligthVertexShader);

	lightPixelShader = make_shared<Shader>();
	lightPixelShader->CreateShader(ShaderType::PIXEL_SHADER, L"Light.hlsl");
	lightPixelShader->SetName(L"Light_PS");
	RESOURCE.AddResource(lightPixelShader->GetName(), lightPixelShader);

	staticMesh_VertexShader = make_shared<Shader>();
	staticMesh_VertexShader->CreateShader(ShaderType::VERTEX_SHADER, L"StaticMesh.hlsl");
	staticMesh_VertexShader->SetName(L"StaticMesh_VS");
	RESOURCE.AddResource(staticMesh_VertexShader->GetName(), staticMesh_VertexShader);

	staticMesh_PixelShader = make_shared<Shader>();
	staticMesh_PixelShader->CreateShader(ShaderType::PIXEL_SHADER, L"StaticMesh.hlsl");
	staticMesh_PixelShader->SetName(L"StaticMesh_PS");
	RESOURCE.AddResource(staticMesh_PixelShader->GetName(), staticMesh_PixelShader);


	animatedMesh_VertexShader = make_shared<Shader>();
	animatedMesh_VertexShader->CreateShader(ShaderType::VERTEX_SHADER, L"AnimatedMesh.hlsl");
	animatedMesh_VertexShader->SetName(L"AnimatedMesh_VS");
	RESOURCE.AddResource(animatedMesh_VertexShader->GetName(), animatedMesh_VertexShader);

	animatedMesh_PixelShader = make_shared<Shader>();
	animatedMesh_PixelShader->CreateShader(ShaderType::PIXEL_SHADER, L"AnimatedMesh.hlsl");
	animatedMesh_PixelShader->SetName(L"AnimatedMesh_PS");
	RESOURCE.AddResource(animatedMesh_PixelShader->GetName(), animatedMesh_PixelShader);

	material = make_shared<Material>();
	material->SetTexture(RESOURCE.GetResource<Texture>(L"panda"));
	material->SetNormalMap(RESOURCE.GetResource<Texture>(L"NormalMap"));
	material->SetVertexShader(RESOURCE.GetResource<Shader>(L"Default_VS"));
	material->SetPixelShader(RESOURCE.GetResource<Shader>(L"Default_PS"));
	material->SetName(L"DefaultMaterial");
	RESOURCE.AddResource(material->GetName(), material);

	skyBoxMaterial = make_shared<Material>();
	skyBoxMaterial->SetTexture(RESOURCE.GetResource<Texture>(L"skyBox"));
	skyBoxMaterial->SetVertexShader(RESOURCE.GetResource<Shader>(L"SkyBox_VS"));
	skyBoxMaterial->SetPixelShader(RESOURCE.GetResource<Shader>(L"SkyBox_PS"));
	skyBoxMaterial->SetName(L"SkyBoxMaterial");
	RESOURCE.AddResource(skyBoxMaterial->GetName(), skyBoxMaterial);

	shared_ptr<Material> tmp = RESOURCE.GetResource<Material>(L"DefaultMaterial");
	lightMaterial = make_shared<Material>();
	lightMaterial->SetTexture(RESOURCE.GetResource<Texture>(L"lightTexture"));
	lightMaterial->SetVertexShader(RESOURCE.GetResource<Shader>(L"Light_VS"));
	lightMaterial->SetPixelShader(RESOURCE.GetResource<Shader>(L"Light_PS"));
	lightMaterial->SetName(L"LightMaterial");
	RESOURCE.AddResource(lightMaterial->GetName(), lightMaterial);

	tower_model->ReadModel(L"Tower/Tower");
	tower_model->SetVertexShaderForMaterial(RESOURCE.GetResource<Shader>(L"StaticMesh_VS"));
	tower_model->SetPixelShaderForMaterial(RESOURCE.GetResource<Shader>(L"StaticMesh_PS"));
	tower_model->ReadMaterial(L"Tower/Tower");
	tower_model->SetName(L"TowerModel");
	RESOURCE.AddResource(tower_model->GetName(), tower_model);

	kachujin_Anim->ReadModel(L"Kachujin/Kachujin");
	kachujin_Anim->SetVertexShaderForMaterial(RESOURCE.GetResource<Shader>(L"AnimatedMesh_VS"));
	kachujin_Anim->SetPixelShaderForMaterial(RESOURCE.GetResource<Shader>(L"AnimatedMesh_PS"));
	kachujin_Anim->ReadMaterial(L"Kachujin/Kachujin");
	kachujin_Anim->ReadAnimation(L"Kachujin/Idle");
	kachujin_Anim->ReadAnimation(L"Kachujin/Run");
	kachujin_Anim->ReadAnimation(L"Kachujin/Slash");
	kachujin_Anim->CreateTexture();
	kachujin_Anim->SetName(L"Kachujin");
	RESOURCE.AddResource(kachujin_Anim->GetName(), kachujin_Anim);
}
