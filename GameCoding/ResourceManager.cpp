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
	shared_ptr<Mesh> quad_mesh = make_shared<Mesh>();
	shared_ptr<Mesh> cylinder_mesh = make_shared<Mesh>();
	shared_ptr<Mesh> terrain_mesh = make_shared<Mesh>();

	shared_ptr<Model> tower_model = make_shared<Model>();
	shared_ptr<Model> house_model = make_shared<Model>();
	shared_ptr<Model> kachujin_Anim = make_shared<Model>();
	shared_ptr<Model> Dreyar_Anim = make_shared<Model>();

	shared_ptr<Texture> texture = make_shared<Texture>();
	shared_ptr<Texture> normalMap = make_shared<Texture>();
	shared_ptr<Texture> lightTexture = make_shared<Texture>();

	shared_ptr<Texture> gridTexture = make_shared<Texture>();

	shared_ptr<Texture> bricks_texture = make_shared<Texture>();

	shared_ptr<Texture> treeTexture = make_shared<Texture>();

	shared_ptr<Texture> grassTexture = make_shared<Texture>();

	shared_ptr<Texture> pandaTexture = make_shared<Texture>();

	shared_ptr<Texture> fireParticleTexture = make_shared<Texture>();

	shared_ptr<Texture> randomTexture = make_shared<Texture>();

	shared_ptr<Shader> ligthRenderShader = make_shared<Shader>();

	shared_ptr<Shader> defaultShader = make_shared<Shader>();
	
	shared_ptr<Shader> environmentMapShader = make_shared<Shader>();

	shared_ptr<Shader> quadShader = make_shared<Shader>();

	shared_ptr<Shader> billBoardShader = make_shared<Shader>();

	shared_ptr<Shader> skyboxShader = make_shared<Shader>();

	shared_ptr<Shader> staticMesh_shader = make_shared<Shader>();

	shared_ptr<Shader> animatedMesh_shader = make_shared<Shader>();

	shared_ptr<Shader> adjustTexture_shader = make_shared<Shader>();

	shared_ptr<Shader> tessellation_shader = make_shared<Shader>();

	shared_ptr<Shader> gaussianBlur_csShaderHorizontal = make_shared<Shader>();

	shared_ptr<Shader> gaussianBlur_csShaderVertical = make_shared<Shader>();

	shared_ptr<Shader> terrain_shader = make_shared<Shader>();
	
	shared_ptr<Shader> initParticle_shader = make_shared<Shader>();

	shared_ptr<Shader> renderParticle_shader = make_shared<Shader>();

	shared_ptr<Shader> debuge_UI_shader = make_shared<Shader>();

	shared_ptr<Material> material = make_shared<Material>();

	shared_ptr<Material> grid_material = make_shared<Material>();

	shared_ptr<Material> debug_UI_material = make_shared <Material>();

	shared_ptr<Material> tessellation_material = make_shared<Material>();

	shared_ptr<Material> billboardMaterial = make_shared<Material>();

	shared_ptr<Material> lightMaterial = make_shared<Material>();

	shared_ptr<Material> skyBoxMaterial = make_shared<Material>();

	shared_ptr<Material> terrainMaterial = make_shared<Material>();

	sphere_mesh->CreateSphere_NormalTangent();
	sphere_mesh->SetName(L"TestMesh");
	RESOURCE.AddResource(sphere_mesh->GetName(), sphere_mesh);
	
	cube_mesh->CreateCube_NormalTangent();
	cube_mesh->SetName(L"Cube");
	RESOURCE.AddResource(cube_mesh->GetName(), cube_mesh);

	grid_mesh->CreateGrid_NormalTangent(100, 100);
	grid_mesh->SetName(L"Grid");
	RESOURCE.AddResource(grid_mesh->GetName(), grid_mesh);

	TerrainInfo info;
	info.heightMapFilename = L"Terrain/terrain.raw";
	info.layerMapFilename0 = L"Terrain/grass.dds";
	info.layerMapFilename1 = L"Terrain/darkdirt.dds";
	info.layerMapFilename2 = L"Terrain/stone.dds";
	info.layerMapFilename3 = L"Terrain/lightdirt.dds";
	info.layerMapFilename4 = L"Terrain/snow.dds";
	info.blendMapFilename = L"Terrain/blend.dds";
	info.heightScale = 50.0f;
	info.heightmapWidth = 2049;
	info.heightmapHeight = 2049;
	info.cellSpacing = 0.5f;
	terrain_mesh->CreateTerrain(info);
	terrain_mesh->SetName(L"Terrain");
	RESOURCE.AddResource(terrain_mesh->GetName(), terrain_mesh);

	quad_mesh->CreateQuad_NormalTangent();
	quad_mesh->SetName(L"Quad");
	RESOURCE.AddResource(quad_mesh->GetName(), quad_mesh);

	cylinder_mesh->CreateCylinder_NormalTangent();
	cylinder_mesh->SetName(L"Cylinder");
	RESOURCE.AddResource(cylinder_mesh->GetName(), cylinder_mesh);

	bricks_texture = make_shared<Texture>();
	bricks_texture->CreateTexture(L"bricks.jpg");
	bricks_texture->SetName(L"Bricks");
	RESOURCE.AddResource(bricks_texture->GetName(), bricks_texture);

	texture = make_shared<Texture>();
	texture->CreateTexture(L"bricks.jpg");
	texture->SetName(L"Leather");
	RESOURCE.AddResource(texture->GetName(), texture);

	gridTexture = make_shared<Texture>();
	gridTexture->CreateTexture(L"yellow.jpg");
	gridTexture->SetName(L"Grid_Texture");
	RESOURCE.AddResource(gridTexture->GetName(), gridTexture);

	grassTexture = make_shared<Texture>();
	grassTexture->CreateTexture(L"Golem.png");
	grassTexture->SetName(L"Grass");
	RESOURCE.AddResource(grassTexture->GetName(), grassTexture);

	pandaTexture = make_shared<Texture>();
	pandaTexture->CreateTexture(L"website.jpg");
	pandaTexture->SetName(L"Panda");
	RESOURCE.AddResource(pandaTexture->GetName(), pandaTexture);

	fireParticleTexture = make_shared<Texture>();
	vector<wstring> textureVector;
	textureVector.push_back(L"flare0.dds");
	fireParticleTexture->CreateTexture2DArraySRV(textureVector);
	fireParticleTexture->SetName(L"Fire_Particle");
	RESOURCE.AddResource(fireParticleTexture->GetName(), fireParticleTexture);

	randomTexture = make_shared<Texture>();
	randomTexture->CreateRandomTexture1DSRV();
	randomTexture->SetName(L"Random_Texture");
	RESOURCE.AddResource(randomTexture->GetName(), randomTexture);

	treeTexture = make_shared<Texture>();
	treeTexture->CreateTexture(L"tree.png");
	treeTexture->SetName(L"tree");
	RESOURCE.AddResource(treeTexture->GetName(), treeTexture);

	normalMap = make_shared<Texture>();
	normalMap->CreateTexture(L"bricks_nmap.png");
	normalMap->SetName(L"NormalMap");
	RESOURCE.AddResource(normalMap->GetName(), normalMap);

	lightTexture = make_shared<Texture>();
	lightTexture->CreateTexture(L"bricks_nmap.png");
	lightTexture->SetName(L"lightTexture");
	RESOURCE.AddResource(lightTexture->GetName(), lightTexture);

	defaultShader = make_shared<Shader>();
	defaultShader->CreateShader(ShaderType::VERTEX_SHADER, L"Shader/Default.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	defaultShader->CreateShader(ShaderType::PIXEL_SHADER, L"Shader/Default.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	defaultShader->SetName(L"Default_Shader");
	{
		defaultShader->GetShaderSlot()->SetSlot(L"CameraBuffer", 0);
		defaultShader->GetShaderSlot()->SetSlot(L"TransformBuffer", 1);
		defaultShader->GetShaderSlot()->SetSlot(L"LightMaterial", 2);
		defaultShader->GetShaderSlot()->SetSlot(L"LightDesc", 3);
		defaultShader->GetShaderSlot()->SetSlot(L"LightAndCameraPos", 4);
		defaultShader->GetShaderSlot()->SetSlot(L"LightSpaceTransform", 5);
		defaultShader->GetShaderSlot()->SetSlot(L"texture0", 0);
		defaultShader->GetShaderSlot()->SetSlot(L"normalMap", 1);
		defaultShader->GetShaderSlot()->SetSlot(L"specularMap", 2);
		defaultShader->GetShaderSlot()->SetSlot(L"diffuseMap", 3);
		defaultShader->GetShaderSlot()->SetSlot(L"shadowMap", 4);
	}
	RESOURCE.AddResource(defaultShader->GetName(), defaultShader);

	debuge_UI_shader = make_shared<Shader>();
	debuge_UI_shader->CreateShader(ShaderType::VERTEX_SHADER, L"Shader/Debug_UI_Shader.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	debuge_UI_shader->CreateShader(ShaderType::PIXEL_SHADER, L"Shader/Debug_UI_Shader.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	debuge_UI_shader->SetName(L"Debug_UI_Shader");
	{
		debuge_UI_shader->GetShaderSlot()->SetSlot(L"CameraBuffer", 0);
		debuge_UI_shader->GetShaderSlot()->SetSlot(L"TransformBuffer", 1);
		debuge_UI_shader->GetShaderSlot()->SetSlot(L"texture0", 0);
	}
	RESOURCE.AddResource(debuge_UI_shader->GetName(), debuge_UI_shader);

	initParticle_shader = make_shared<Shader>();
	initParticle_shader->CreateShader(ShaderType::VERTEX_SHADER, L"Shader/InitParticleSystem.hlsl", InputLayoutType::VertexParticle);
	initParticle_shader->CreateShader(ShaderType::GEOMETRY_SHADER_WITH_STREAMOUTPUT, L"Shader/InitParticleSystem.hlsl", InputLayoutType::VertexParticle);
	initParticle_shader->SetName(L"InitParticle_Shader");
	{
		initParticle_shader->GetShaderSlot()->SetSlot(L"ParticleBuffer", 0);
		initParticle_shader->GetShaderSlot()->SetSlot(L"gRandomTex", 0);
	}
	RESOURCE.AddResource(initParticle_shader->GetName(), initParticle_shader);

	renderParticle_shader = make_shared<Shader>();
	renderParticle_shader->CreateShader(ShaderType::VERTEX_SHADER, L"Shader/RenderParticleSystem.hlsl", InputLayoutType::VertexParticle);
	renderParticle_shader->CreateShader(ShaderType::GEOMETRY_SHADER, L"Shader/RenderParticleSystem.hlsl", InputLayoutType::VertexParticle);
	renderParticle_shader->CreateShader(ShaderType::PIXEL_SHADER, L"Shader/RenderParticleSystem.hlsl", InputLayoutType::VertexParticle);
	renderParticle_shader->SetName(L"RenderParticle_Shader");
	{
		renderParticle_shader->GetShaderSlot()->SetSlot(L"ParticleBuffer", 0);
		renderParticle_shader->GetShaderSlot()->SetSlot(L"gTexArray", 0);
	}
	RESOURCE.AddResource(renderParticle_shader->GetName(), renderParticle_shader);

	terrain_shader = make_shared<Shader>();
	terrain_shader->CreateShader(ShaderType::VERTEX_SHADER, L"Shader/Terrain.hlsl", InputLayoutType::Terrain);
	terrain_shader->CreateShader(ShaderType::HULL_SHADER, L"Shader/Terrain.hlsl", InputLayoutType::Terrain);
	terrain_shader->CreateShader(ShaderType::DOMAIN_SHADER, L"Shader/Terrain.hlsl", InputLayoutType::Terrain);
	terrain_shader->CreateShader(ShaderType::PIXEL_SHADER, L"Shader/Terrain.hlsl", InputLayoutType::Terrain);
	terrain_shader->SetName(L"Terrain_Shader");
	{
		terrain_shader->GetShaderSlot()->SetSlot(L"CameraBuffer", 0);
		terrain_shader->GetShaderSlot()->SetSlot(L"LightMaterial", 1);
		terrain_shader->GetShaderSlot()->SetSlot(L"LightDesc", 2);
		terrain_shader->GetShaderSlot()->SetSlot(L"LightAndCameraPos", 3);
		terrain_shader->GetShaderSlot()->SetSlot(L"TerrainBuffer", 4);
		terrain_shader->GetShaderSlot()->SetSlot(L"LightSpaceTransform", 5);
		terrain_shader->GetShaderSlot()->SetSlot(L"gLayerMapArray", 0);
		terrain_shader->GetShaderSlot()->SetSlot(L"gBlendMap", 1);
		terrain_shader->GetShaderSlot()->SetSlot(L"gHeightMap", 2);
		terrain_shader->GetShaderSlot()->SetSlot(L"shadowMap", 3);
	}
	RESOURCE.AddResource(terrain_shader->GetName(), terrain_shader);


	tessellation_shader = make_shared<Shader>();
	tessellation_shader->CreateShader(ShaderType::VERTEX_SHADER, L"Shader/Tesselation.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	tessellation_shader->CreateShader(ShaderType::PIXEL_SHADER, L"Shader/Tesselation.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	tessellation_shader->CreateShader(ShaderType::HULL_SHADER, L"Shader/Tesselation.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	tessellation_shader->CreateShader(ShaderType::DOMAIN_SHADER, L"Shader/Tesselation.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	tessellation_shader->SetName(L"Tesselation_Shader");
	{
		tessellation_shader->GetShaderSlot()->SetSlot(L"CameraBuffer", 0);
		tessellation_shader->GetShaderSlot()->SetSlot(L"TransformBuffer", 1);
		tessellation_shader->GetShaderSlot()->SetSlot(L"LightMaterial", 2);
		tessellation_shader->GetShaderSlot()->SetSlot(L"LightDesc", 3);
		tessellation_shader->GetShaderSlot()->SetSlot(L"LightAndCameraPos", 4);
		tessellation_shader->GetShaderSlot()->SetSlot(L"LightSpaceTransform", 5);
		tessellation_shader->GetShaderSlot()->SetSlot(L"texture0", 0);
		tessellation_shader->GetShaderSlot()->SetSlot(L"normalMap", 1);
		tessellation_shader->GetShaderSlot()->SetSlot(L"specularMap", 2);
		tessellation_shader->GetShaderSlot()->SetSlot(L"diffuseMap", 3);
		tessellation_shader->GetShaderSlot()->SetSlot(L"shadowMap", 4);
	}
	RESOURCE.AddResource(tessellation_shader->GetName(), tessellation_shader);


	environmentMapShader = make_shared<Shader>();
	environmentMapShader->CreateShader(ShaderType::VERTEX_SHADER, L"Shader/EnvironmentMap.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	environmentMapShader->CreateShader(ShaderType::PIXEL_SHADER, L"Shader/EnvironmentMap.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	environmentMapShader->SetName(L"EnvironmentMap_Shader");
	{
		environmentMapShader->GetShaderSlot()->SetSlot(L"CameraBuffer", 0);
		environmentMapShader->GetShaderSlot()->SetSlot(L"TransformBuffer", 1);
		environmentMapShader->GetShaderSlot()->SetSlot(L"LightMaterial", 2);
		environmentMapShader->GetShaderSlot()->SetSlot(L"LightDesc", 3);
		environmentMapShader->GetShaderSlot()->SetSlot(L"LightAndCameraPos", 4);
		environmentMapShader->GetShaderSlot()->SetSlot(L"LightSpaceTransform", 5);
		environmentMapShader->GetShaderSlot()->SetSlot(L"texture0", 0);
		environmentMapShader->GetShaderSlot()->SetSlot(L"normalMap", 1);
		environmentMapShader->GetShaderSlot()->SetSlot(L"specularMap", 2);
		environmentMapShader->GetShaderSlot()->SetSlot(L"diffuseMap", 3);
		environmentMapShader->GetShaderSlot()->SetSlot(L"shadowMap", 4);
	}
	RESOURCE.AddResource(environmentMapShader->GetName(), environmentMapShader);

	quadShader = make_shared<Shader>();
	quadShader->CreateShader(ShaderType::VERTEX_SHADER, L"Shader/Quad.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	quadShader->CreateShader(ShaderType::PIXEL_SHADER, L"Shader/Quad.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	quadShader->SetName(L"Quad_Shader");
	{
		quadShader->GetShaderSlot()->SetSlot(L"texture0", 0);
	}
	RESOURCE.AddResource(quadShader->GetName(), quadShader);

	billBoardShader->CreateShader(ShaderType::VERTEX_SHADER, L"Shader/Billboard.hlsl", InputLayoutType::VertexBillboard_Geometry);
	billBoardShader->CreateShader(ShaderType::PIXEL_SHADER, L"Shader/Billboard.hlsl", InputLayoutType::VertexBillboard_Geometry);
	billBoardShader->CreateShader(ShaderType::GEOMETRY_SHADER, L"Shader/Billboard.hlsl", InputLayoutType::VertexBillboard_Geometry);
	billBoardShader->SetName(L"Billboard_Shader");
	{
		billBoardShader->GetShaderSlot()->SetSlot(L"CameraBuffer",0);
		billBoardShader->GetShaderSlot()->SetSlot(L"TransformBuffer", 1);
		billBoardShader->GetShaderSlot()->SetSlot(L"CameraPos", 2);
		billBoardShader->GetShaderSlot()->SetSlot(L"texture0", 0);
	}
	RESOURCE.AddResource(billBoardShader->GetName(), billBoardShader);

	adjustTexture_shader = make_shared<Shader>();
	adjustTexture_shader->CreateShader(ShaderType::COMPUTE_SHADER, L"Shader/ComputeShader_AdjustTexture.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	adjustTexture_shader->SetName(L"AdjustTexture_Shader");
	RESOURCE.AddResource(adjustTexture_shader->GetName(), adjustTexture_shader);

	gaussianBlur_csShaderHorizontal = make_shared<Shader>();
	gaussianBlur_csShaderHorizontal->CreateShader(ShaderType::COMPUTE_SHADER, L"Shader/ComputeShader_GaussianBlurHorizontal.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	gaussianBlur_csShaderHorizontal->SetName(L"Gaussian_Horizontal");
	RESOURCE.AddResource(gaussianBlur_csShaderHorizontal->GetName(), gaussianBlur_csShaderHorizontal);

	gaussianBlur_csShaderVertical = make_shared<Shader>();
	gaussianBlur_csShaderVertical->CreateShader(ShaderType::COMPUTE_SHADER, L"Shader/ComputeShader_GaussianBlurVertical.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	gaussianBlur_csShaderVertical->SetName(L"Gaussian_Vertical");
	RESOURCE.AddResource(gaussianBlur_csShaderVertical->GetName(), gaussianBlur_csShaderVertical);

	skyboxShader = make_shared<Shader>();
	skyboxShader->CreateShader(ShaderType::VERTEX_SHADER, L"Shader/SkyBox.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	skyboxShader->CreateShader(ShaderType::PIXEL_SHADER, L"Shader/SkyBox.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	skyboxShader->SetName(L"SkyBox_Shader");
	{
		skyboxShader->GetShaderSlot()->SetSlot(L"CameraBuffer", 0);
		skyboxShader->GetShaderSlot()->SetSlot(L"texture0", 0);
	}
	RESOURCE.AddResource(skyboxShader->GetName(), skyboxShader);


	ligthRenderShader = make_shared<Shader>();
	ligthRenderShader->CreateShader(ShaderType::VERTEX_SHADER, L"Shader/Light.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	ligthRenderShader->SetName(L"Light_Render_Shader");
	ligthRenderShader->CreateShader(ShaderType::PIXEL_SHADER, L"Shader/Light.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	{
		ligthRenderShader->GetShaderSlot()->SetSlot(L"CameraBuffer", 0);
		ligthRenderShader->GetShaderSlot()->SetSlot(L"TransformBuffer", 1);
		ligthRenderShader->GetShaderSlot()->SetSlot(L"texture0", 0);
	}
	RESOURCE.AddResource(ligthRenderShader->GetName(), ligthRenderShader);


	staticMesh_shader = make_shared<Shader>();
	staticMesh_shader->CreateShader(ShaderType::VERTEX_SHADER, L"Shader/StaticMesh.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	staticMesh_shader->CreateShader(ShaderType::PIXEL_SHADER, L"Shader/StaticMesh.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	staticMesh_shader->SetName(L"StaticMesh_Shader");
	{
		staticMesh_shader->GetShaderSlot()->SetSlot(L"CameraBuffer", 0);
		staticMesh_shader->GetShaderSlot()->SetSlot(L"TransformBuffer", 1);
		staticMesh_shader->GetShaderSlot()->SetSlot(L"LightMaterial", 2);
		staticMesh_shader->GetShaderSlot()->SetSlot(L"LightDesc", 3);
		staticMesh_shader->GetShaderSlot()->SetSlot(L"LightAndCameraPos", 4);
		staticMesh_shader->GetShaderSlot()->SetSlot(L"BoneBuffer", 5);
		staticMesh_shader->GetShaderSlot()->SetSlot(L"BonIndex", 6);
		staticMesh_shader->GetShaderSlot()->SetSlot(L"LightSpaceTransform", 7);
		staticMesh_shader->GetShaderSlot()->SetSlot(L"texture0", 0);
		staticMesh_shader->GetShaderSlot()->SetSlot(L"normalMap", 1);
		staticMesh_shader->GetShaderSlot()->SetSlot(L"specularMap", 2);
		staticMesh_shader->GetShaderSlot()->SetSlot(L"diffuseMap", 3);
		staticMesh_shader->GetShaderSlot()->SetSlot(L"shadowMap", 4);

	}
	RESOURCE.AddResource(staticMesh_shader->GetName(), staticMesh_shader);


	animatedMesh_shader = make_shared<Shader>();
	animatedMesh_shader->CreateShader(ShaderType::VERTEX_SHADER, L"Shader/AnimatedMesh.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	animatedMesh_shader->CreateShader(ShaderType::PIXEL_SHADER, L"Shader/AnimatedMesh.hlsl", InputLayoutType::VertexTextureNormalTangentBlendData);
	animatedMesh_shader->SetName(L"AnimatedMesh_Shader");
	{
		animatedMesh_shader->GetShaderSlot()->SetSlot(L"CameraBuffer", 0);
		animatedMesh_shader->GetShaderSlot()->SetSlot(L"TransformBuffer", 1);
		animatedMesh_shader->GetShaderSlot()->SetSlot(L"LightMaterial", 2);
		animatedMesh_shader->GetShaderSlot()->SetSlot(L"LightDesc", 3);
		animatedMesh_shader->GetShaderSlot()->SetSlot(L"LightAndCameraPos", 4);
		animatedMesh_shader->GetShaderSlot()->SetSlot(L"BlendBuffer", 5);
		animatedMesh_shader->GetShaderSlot()->SetSlot(L"LightSpaceTransform", 6);
		animatedMesh_shader->GetShaderSlot()->SetSlot(L"normalMap", 0);
		animatedMesh_shader->GetShaderSlot()->SetSlot(L"specularMap", 1);
		animatedMesh_shader->GetShaderSlot()->SetSlot(L"diffuseMap", 2);
		animatedMesh_shader->GetShaderSlot()->SetSlot(L"TransformMap", 3);
		animatedMesh_shader->GetShaderSlot()->SetSlot(L"shadowMap", 4);
	}
	RESOURCE.AddResource(animatedMesh_shader->GetName(), animatedMesh_shader);


	material = make_shared<Material>();
	material->SetTexture(RESOURCE.GetResource<Texture>(L"Leather"));
	material->SetNormalMap(RESOURCE.GetResource<Texture>(L"NormalMap"));
	MaterialDesc matDesc;
	matDesc.ambient = Vec4(0.95f, 0.95f, 0.95f, 1.0f);
	matDesc.diffuse = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	matDesc.specular = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
	material->SetMaterialDesc(matDesc);
	material->SetShader(RESOURCE.GetResource<Shader>(L"Default_Shader"));
	material->SetName(L"DefaultMaterial");
	RESOURCE.AddResource(material->GetName(), material);

	grid_material = make_shared<Material>();
	grid_material->SetTexture(RESOURCE.GetResource<Texture>(L"Grid_Texture"));
	matDesc;
	matDesc.ambient = Vec4(0.95f, 0.95f, 0.95f, 1.0f);
	matDesc.diffuse = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	matDesc.specular = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
	grid_material->SetMaterialDesc(matDesc);
	grid_material->SetShader(RESOURCE.GetResource<Shader>(L"Default_Shader"));
	grid_material->SetName(L"GridMaterial");
	RESOURCE.AddResource(grid_material->GetName(), grid_material);

	debug_UI_material = make_shared<Material>();
	debug_UI_material->SetTexture(RESOURCE.GetResource<Texture>(L"Leather"));
	debug_UI_material->SetShader(RESOURCE.GetResource<Shader>(L"Debug_UI_Shader"));
	debug_UI_material->SetName(L"Debug_UI_Material");
	RESOURCE.AddResource(debug_UI_material->GetName(), debug_UI_material);

	tessellation_material = make_shared<Material>();
	tessellation_material->SetTexture(RESOURCE.GetResource<Texture>(L"Bricks"));
	tessellation_material->SetNormalMap(RESOURCE.GetResource<Texture>(L"NormalMap"));
	matDesc.ambient = Vec4(0.95f, 0.95f, 0.95f, 1.0f);
	matDesc.diffuse = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	matDesc.specular = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
	tessellation_material->SetMaterialDesc(matDesc);
	tessellation_material->SetShader(RESOURCE.GetResource<Shader>(L"Tesselation_Shader"));
	tessellation_material->SetName(L"Tessellation_Material");
	RESOURCE.AddResource(tessellation_material->GetName(), tessellation_material);

	billboardMaterial->SetTexture(RESOURCE.GetResource<Texture>(L"Leather"));
	billboardMaterial->SetShader(RESOURCE.GetResource<Shader>(L"Billboard_Shader"));
	billboardMaterial->SetName(L"Billboard_Material");
	RESOURCE.AddResource(billboardMaterial->GetName(), billboardMaterial);


	skyBoxMaterial = make_shared<Material>();
	shared_ptr<Texture> cubeMapTexture = make_shared<Texture>();
	cubeMapTexture->LoadTextureFromDDS(L"grasscube1024.dds");
	skyBoxMaterial->SetTexture(cubeMapTexture);
	skyBoxMaterial->SetShader(RESOURCE.GetResource<Shader>(L"SkyBox_Shader"));
	skyBoxMaterial->SetName(L"SkyBoxMaterial");
	RESOURCE.AddResource(skyBoxMaterial->GetName(), skyBoxMaterial);

	terrainMaterial = make_shared<Material>();
	terrainMaterial->SetShader(RESOURCE.GetResource<Shader>(L"Terrain_Shader"));
	matDesc.ambient = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	matDesc.diffuse = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	matDesc.specular = Vec4(0.0f, 0.0f, 0.0f, 64.0f);
	terrainMaterial->SetMaterialDesc(matDesc);
	terrainMaterial->SetName(L"TerrainMaterial");
	RESOURCE.AddResource(terrainMaterial->GetName(), terrainMaterial);

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

	house_model->ReadModel(L"House/House");
	house_model->SetShaderForMaterial(RESOURCE.GetResource<Shader>(L"StaticMesh_Shader"));
	house_model->ReadMaterial(L"House/House");
	house_model->SetName(L"HouseModel");
	RESOURCE.AddResource(house_model->GetName(), house_model);


	kachujin_Anim->ReadModel(L"Kachujin/Kachujin");
	kachujin_Anim->SetShaderForMaterial(RESOURCE.GetResource<Shader>(L"AnimatedMesh_Shader"));
	kachujin_Anim->ReadMaterial(L"Kachujin/Kachujin");
	kachujin_Anim->ReadAnimation(L"Kachujin/Dismissing");
	kachujin_Anim->ReadAnimation(L"Kachujin/Run");
	kachujin_Anim->ReadAnimation(L"Kachujin/Slash");
	kachujin_Anim->CreateTexture();
	kachujin_Anim->SetName(L"Kachujin");
	RESOURCE.AddResource(kachujin_Anim->GetName(), kachujin_Anim);

	Dreyar_Anim->ReadModel(L"Dreyar/Dreyar");
	Dreyar_Anim->SetShaderForMaterial(RESOURCE.GetResource<Shader>(L"AnimatedMesh_Shader"));
	Dreyar_Anim->ReadMaterial(L"Dreyar/Dreyar");
	Dreyar_Anim->ReadAnimation(L"Dreyar/Kick");
	Dreyar_Anim->ReadAnimation(L"Dreyar/Fall");
	Dreyar_Anim->ReadAnimation(L"Dreyar/Dance");
	Dreyar_Anim->CreateTexture();
	Dreyar_Anim->SetName(L"Dreyar");
	RESOURCE.AddResource(Dreyar_Anim->GetName(), Dreyar_Anim);

}
