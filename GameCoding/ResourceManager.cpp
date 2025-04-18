#include "pch.h"
#include "ResourceManager.h"
#include "tinyxml2.h"
#include "Utils.h"

void ResourceManager::Init()
{
	SaveResource();
	AddResource();
}

void ResourceManager::SaveResource()
{
	// mesh
	WriteMeshToXML(L"Sphere", L"Sphere", L"Resource/Mesh/Sphere.xml");
	WriteMeshToXML(L"Cube", L"Cube", L"Resource/Mesh/Cube.xml");
	WriteMeshToXML(L"Grid", L"Grid", L"Resource/Mesh/Grid.xml");
	WriteMeshToXML(L"Terrain", L"Terrain", L"Resource/Mesh/Terrain.xml");
	WriteMeshToXML(L"Quad", L"Quad", L"Resource/Mesh/Quad.xml");
	WriteMeshToXML(L"Cylinder", L"Cylinder", L"Resource/Mesh/Cylinder.xml");

	// texture
	WriteTextureToXML(L"bricks.jpg", L"Bricks", L"Resource/Texture/Bricks.xml");
	WriteTextureToXML(L"Leather.jpg", L"Leather", L"Resource/Texture/Leather.xml");
	WriteTextureToXML(L"yellow.jpg", L"Yellow", L"Resource/Texture/Yellow.xml");
	WriteTextureToXML(L"Grass.png", L"Grass", L"Resource/Texture/Grass.xml");
	WriteTextureToXML(L"panda.jpg", L"Panda", L"Resource/Texture/Panda.xml");
	WriteTextureToXML(L"tree.png", L"Tree", L"Resource/Texture/Tree.xml");
	WriteTextureToXML(L"start.png", L"startButton", L"Resource/Texture/startButton.xml");
	WriteTextureToXML(L"pause.png", L"pauseButton", L"Resource/Texture/pauseButton.xml");
	WriteTextureToXML(L"stop.png", L"stopButton", L"Resource/Texture/stopButton.xml");
	WriteTextureToXML(L"bricks_nmap.png", L"NormalMap", L"Resource/Texture/NormalMap.xml");
	WriteTextureToXML(L"Material_Icon.png", L"Material_Icon", L"Resource/Texture/Material_Icon.xml");
	WriteTextureToXML(L"Mesh_Icon.png", L"Mesh_Icon", L"Resource/Texture/Mesh_Icon.xml");
	WriteTextureToXML(L"Model_Icon.png", L"Model_Icon", L"Resource/Texture/Model_Icon.xml");
	WriteTextureToXML(L"Shader_Icon.png", L"Shader_Icon", L"Resource/Texture/Shader_Icon.xml");
	WriteTextureToXML(L"Solid_white.png", L"Solid_white", L"Resource/Texture/Solid_white.xml");
	WriteTextureToXML(L"DefaultNormal.png", L"DefaultNormal", L"Resource/Texture/DefaultNormal.xml");
	WriteTextureToXML(L"Cpp_Icon.png", L"CPP_Icon", L"Resource/Texture/CPP_Icon.xml");
	// shader
	vector<ShaderType> types = { ShaderType::VERTEX_SHADER, ShaderType::PIXEL_SHADER };
	vector<InputLayoutType> layouts = { InputLayoutType::VertexTextureNormalTangentBlendData, InputLayoutType::VertexTextureNormalTangentBlendData };
	map<wstring, uint32> slots = {
		{L"CameraBuffer", 0},
		{L"TransformBuffer", 1},
		{L"LightMaterial", 2},
		{L"LightDesc", 3},
		{L"LightAndCameraPos", 4},
		{L"LightSpaceTransform", 5},
		{L"CheckInstancingObject", 6},
		{L"texture0", 0},
		{L"normalMap", 1},
		{L"specularMap", 2},
		{L"diffuseMap", 3},
		{L"shadowMap", 4},
	};

	WriteShaderToXML(L"Shader/Default.hlsl", L"Default_Shader", types, layouts, slots, L"Resource/Shader/DefaultShader.xml");

	types = {
	ShaderType::VERTEX_SHADER,
	ShaderType::HULL_SHADER,
	ShaderType::DOMAIN_SHADER,
	ShaderType::PIXEL_SHADER
	};

	layouts = {
		InputLayoutType::Terrain,
		InputLayoutType::Terrain,
		InputLayoutType::Terrain,
		InputLayoutType::Terrain
	};

	slots = {
		{L"CameraBuffer", 0},
		{L"LightMaterial", 1},
		{L"LightDesc", 2},
		{L"LightAndCameraPos", 3},
		{L"TerrainBuffer", 4},
		{L"LightSpaceTransform", 5},
		{L"TransformBuffer", 6},
		{L"gLayerMapArray", 0},
		{L"gBlendMap", 1},
		{L"gHeightMap", 2},
		{L"shadowMap", 3}
	};

	WriteShaderToXML(L"Shader/Terrain.hlsl", L"Terrain_Shader", types, layouts, slots, L"Resource/Shader/TerrainShader.xml");

	types = { ShaderType::VERTEX_SHADER, ShaderType::PIXEL_SHADER };
	layouts = { InputLayoutType::VertexTextureNormalTangentBlendData, InputLayoutType::VertexTextureNormalTangentBlendData };
	slots = {
		{L"CameraBuffer", 0},
		{L"TransformBuffer", 1},
		{L"texture0", 0}
	};
	WriteShaderToXML(L"Shader/Debug_UI_Shader.hlsl", L"Debug_UI_Shader", types, layouts, slots, L"Resource/Shader/DebugUIShader.xml");

	types = { ShaderType::VERTEX_SHADER, ShaderType::GEOMETRY_SHADER_WITH_STREAMOUTPUT };
	layouts = { InputLayoutType::VertexParticle, InputLayoutType::VertexParticle };
	slots = {
		{L"ParticleBuffer", 0},
		{L"gRandomTex", 0}
	};
	WriteShaderToXML(L"Shader/InitParticleSystem.hlsl", L"InitParticle_Shader", types, layouts, slots, L"Resource/Shader/InitParticleShader.xml");

	types = { ShaderType::VERTEX_SHADER, ShaderType::GEOMETRY_SHADER_WITH_STREAMOUTPUT };
	layouts = { InputLayoutType::VertexParticle, InputLayoutType::VertexParticle };
	slots = {
		{L"ParticleBuffer", 0},
		{L"gRandomTex", 0}
	};
	WriteShaderToXML(L"Shader/InitParticleSystem_Bomb.hlsl", L"InitParticleBomb_Shader", types, layouts, slots, L"Resource/Shader/InitParticleBombShader.xml");

	types = { ShaderType::VERTEX_SHADER, ShaderType::GEOMETRY_SHADER, ShaderType::PIXEL_SHADER };
	layouts = { InputLayoutType::VertexParticle, InputLayoutType::VertexParticle, InputLayoutType::VertexParticle };
	slots = {
		{L"ParticleBuffer", 0},
		{L"TransformBuffer", 1},
		{L"gTexArray", 0}
	};
	WriteShaderToXML(L"Shader/RenderParticleSystem.hlsl", L"RenderParticle_Shader", types, layouts, slots, L"Resource/Shader/RenderParticleShader.xml");

	types = {
		ShaderType::VERTEX_SHADER,
		ShaderType::PIXEL_SHADER,
		ShaderType::HULL_SHADER,
		ShaderType::DOMAIN_SHADER
	};
	layouts = {
		InputLayoutType::VertexTextureNormalTangentBlendData,
		InputLayoutType::VertexTextureNormalTangentBlendData,
		InputLayoutType::VertexTextureNormalTangentBlendData,
		InputLayoutType::VertexTextureNormalTangentBlendData
	};
	slots = {
		{L"CameraBuffer", 0},
		{L"TransformBuffer", 1},
		{L"LightMaterial", 2},
		{L"LightDesc", 3},
		{L"LightAndCameraPos", 4},
		{L"LightSpaceTransform", 5},
		{L"texture0", 0},
		{L"normalMap", 1},
		{L"specularMap", 2},
		{L"diffuseMap", 3},
		{L"shadowMap", 4}
	};
	WriteShaderToXML(L"Shader/Tesselation.hlsl", L"Tesselation_Shader", types, layouts, slots, L"Resource/Shader/TessellationShader.xml");


	types = { ShaderType::VERTEX_SHADER, ShaderType::PIXEL_SHADER };
	layouts = { InputLayoutType::VertexTextureNormalTangentBlendData, InputLayoutType::VertexTextureNormalTangentBlendData };
	slots = {
		{L"CameraBuffer", 0},
		{L"TransformBuffer", 1},
		{L"LightMaterial", 2},
		{L"LightDesc", 3},
		{L"LightAndCameraPos", 4},
		{L"LightSpaceTransform", 5},
		{L"texture0", 0},
		{L"normalMap", 1},
		{L"specularMap", 2},
		{L"diffuseMap", 3},
		{L"shadowMap", 4}
	};
	WriteShaderToXML(L"Shader/EnvironmentMap.hlsl", L"EnvironmentMap_Shader", types, layouts, slots, L"Resource/Shader/EnvironmentMapShader.xml");

	types = { ShaderType::VERTEX_SHADER, ShaderType::PIXEL_SHADER };
	layouts = { InputLayoutType::VertexTextureNormalTangentBlendData, InputLayoutType::VertexTextureNormalTangentBlendData };
	slots = {
		{L"texture0", 0}
	};
	WriteShaderToXML(L"Shader/Quad.hlsl", L"Quad_Shader", types, layouts, slots, L"Resource/Shader/QuadShader.xml");

	types = { ShaderType::VERTEX_SHADER, ShaderType::PIXEL_SHADER, ShaderType::GEOMETRY_SHADER };
	layouts = {
		InputLayoutType::VertexBillboard_Geometry,
		InputLayoutType::VertexBillboard_Geometry,
		InputLayoutType::VertexBillboard_Geometry
	};
	slots = {
		{L"CameraBuffer", 0},
		{L"TransformBuffer", 1},
		{L"CameraPos", 2},
		{L"texture0", 0}
	};
	WriteShaderToXML(L"Shader/Billboard.hlsl", L"Billboard_Shader", types, layouts, slots, L"Resource/Shader/BillboardShader.xml");

	types = { ShaderType::COMPUTE_SHADER };
	layouts = { InputLayoutType::VertexTextureNormalTangentBlendData };
	slots.clear();
	WriteShaderToXML(L"Shader/ComputeShader_AdjustTexture.hlsl", L"AdjustTexture_Shader", types, layouts, slots, L"Resource/Shader/AdjustTextureShader.xml");

	types = { ShaderType::COMPUTE_SHADER };
	layouts = { InputLayoutType::VertexTextureNormalTangentBlendData };
	WriteShaderToXML(L"Shader/ComputeShader_GaussianBlurHorizontal.hlsl", L"Gaussian_Horizontal", types, layouts, slots, L"Resource/Shader/GaussianHorizontalShader.xml");

	WriteShaderToXML(L"Shader/ComputeShader_GaussianBlurVertical.hlsl", L"Gaussian_Vertical", types, layouts, slots, L"Resource/Shader/GaussianVerticalShader.xml");

	types = { ShaderType::VERTEX_SHADER, ShaderType::PIXEL_SHADER };
	layouts = { InputLayoutType::VertexTextureNormalTangentBlendData, InputLayoutType::VertexTextureNormalTangentBlendData };
	slots = {
		{L"CameraBuffer", 0},
		{L"texture0", 0}
	};
	WriteShaderToXML(L"Shader/SkyBox.hlsl", L"SkyBox_Shader", types, layouts, slots, L"Resource/Shader/SkyboxShader.xml");

	types = { ShaderType::VERTEX_SHADER, ShaderType::PIXEL_SHADER };
	layouts = { InputLayoutType::VertexTextureNormalTangentBlendData, InputLayoutType::VertexTextureNormalTangentBlendData };
	slots = {
		{L"CameraBuffer", 0},
		{L"TransformBuffer", 1},
		{L"CheckInstancingObject", 2},
		{L"texture0", 0}
	};
	WriteShaderToXML(L"Shader/SimpleShader.hlsl", L"Simple_Render_Shader", types, layouts, slots, L"Resource/Shader/SimpleShader.xml");

	types = { ShaderType::VERTEX_SHADER, ShaderType::PIXEL_SHADER };
	layouts = { InputLayoutType::VertexTextureNormalTangentBlendData, InputLayoutType::VertexTextureNormalTangentBlendData };
	slots = {
		{L"CameraBuffer", 0},
		{L"TransformBuffer", 1},
		{L"LightMaterial", 2},
		{L"LightDesc", 3},
		{L"LightAndCameraPos", 4},
		{L"BoneBuffer", 5},
		{L"BonIndex", 6},
		{L"LightSpaceTransform", 7},
		{L"CheckInstancingObject", 8},
		{L"texture0", 0},
		{L"normalMap", 1},
		{L"specularMap", 2},
		{L"diffuseMap", 3},
		{L"shadowMap", 4}
	};
	WriteShaderToXML(L"Shader/StaticMesh.hlsl", L"StaticMesh_Shader", types, layouts, slots, L"Resource/Shader/StaticMeshShader.xml");

	types = { ShaderType::VERTEX_SHADER, ShaderType::PIXEL_SHADER };
	layouts = { InputLayoutType::VertexTextureNormalTangentBlendData, InputLayoutType::VertexTextureNormalTangentBlendData };
	slots = {
		{L"CameraBuffer", 0},
		{L"TransformBuffer", 1},
		{L"LightMaterial", 2},
		{L"LightDesc", 3},
		{L"LightAndCameraPos", 4},
		{L"BlendBuffer", 5},
		{L"LightSpaceTransform", 6},
		{L"CheckInstancingObject", 7},
		{L"normalMap", 0},
		{L"specularMap", 1},
		{L"diffuseMap", 2},
		{L"TransformMap", 3},
		{L"shadowMap", 4}
	};
	WriteShaderToXML(L"Shader/AnimatedMesh.hlsl", L"AnimatedMesh_Shader", types, layouts, slots, L"Resource/Shader/AnimatedMeshShader.xml");


	// material
	MaterialDesc matDesc;
	matDesc.ambient = Vec4(0.95f, 0.95f, 0.95f, 1.0f);
	matDesc.diffuse = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	matDesc.specular = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
	WriteMaterialToXML(L"Bricks", L"NormalMap", matDesc, L"Default_Shader", L"DefaultMaterial", L"Resource/Material/DefaultMaterial.xml");

	matDesc.ambient = Vec4(0.95f, 0.95f, 0.95f, 1.0f);
	matDesc.diffuse = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	matDesc.specular = Vec4(0.0f, 0.0f, 0.0f, 1.0f);
	WriteMaterialToXML(L"Bricks", L"NormalMap", matDesc, L"Tesselation_Shader", L"Tessellation_Material", L"Resource/Material/TessellationMaterial.xml");

	matDesc.ambient = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	matDesc.diffuse = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	matDesc.specular = Vec4(0.0f, 0.0f, 0.0f, 64.0f);
	WriteMaterialToXML(L"", L"", matDesc, L"Terrain_Shader", L"TerrainMaterial", L"Resource/Material/TerrainMaterial.xml");

	WriteMaterialToXML(L"Yellow", L"", MaterialDesc(), L"Simple_Render_Shader", L"SimpleMaterial", L"Resource/Material/SimpleMaterial.xml");

	WriteMaterialToXML(L"Solid_white", L"DefaultNormal", MaterialDesc(), L"Default_Shader", L"SolidWhiteMaterial", L"Resource/Material/SolidWhiteMaterial.xml");

	WriteMaterialToXML(L"Fire_Particle", L"", MaterialDesc(), L"RenderParticle_Shader", L"ParticleMaterial", L"Resource/Material/ParticleMaterial.xml");

	WriteMaterialToXML(L"Yellow", L"", MaterialDesc(), L"Default_Shader", L"GridMaterial", L"Resource/Material/GridMaterial.xml");

	WriteMaterialToXML(L"Leather", L"", MaterialDesc(), L"Billboard_Shader", L"Billboard_Material", L"Resource/Material/BillboardMaterial.xml");

	WriteMaterialToXML(L"Leather", L"", MaterialDesc(), L"Debug_UI_Shader", L"Debug_UI_Material", L"Resource/Material/DebugUIMaterial.xml");


	// model
	WriteModelToXML(L"Tower/Tower", L"StaticMesh_Shader", L"Tower/Tower", L"TowerModel",
		vector<wstring>(), L"Resource/Model/TowerModel.xml");

	WriteModelToXML(L"House/House", L"StaticMesh_Shader", L"House/House", L"HouseModel",
		vector<wstring>(), L"Resource/Model/HouseModel.xml");

	WriteModelToXML(L"Cottage/Cottage", L"StaticMesh_Shader", L"Cottage/Cottage", L"CottageModel",
		vector<wstring>(), L"Resource/Model/Cottage.xml");

	WriteModelToXML(L"SMG/SMG", L"StaticMesh_Shader", L"SMG/SMG", L"SMGModel",
		vector<wstring>(), L"Resource/Model/SMG.xml");

	vector<wstring> kachujinAnims = { L"Kachujin/Run", L"Kachujin/Slash", L"Kachujin/Dismissing" };
	WriteModelToXML(L"Kachujin/Kachujin", L"AnimatedMesh_Shader", L"Kachujin/Kachujin", L"Kachujin",
		kachujinAnims, L"Resource/Model/KachujinModel.xml");

	vector<wstring> dreyarAnims = { L"Dreyar/Kick", L"Dreyar/Fall", L"Dreyar/Dance" };
	WriteModelToXML(L"Dreyar/Dreyar", L"AnimatedMesh_Shader", L"Dreyar/Dreyar", L"Dreyar",
		dreyarAnims, L"Resource/Model/DreyarModel.xml");

	vector<wstring> mariaAnims = { L"Maria/Idle", L"Maria/Maria_Kick", L"Maria/Maria_Backhand" };
	WriteModelToXML(L"Maria/Maria", L"AnimatedMesh_Shader", L"Maria/Maria", L"Maria",
		mariaAnims, L"Resource/Model/MariaModel.xml");

	vector<wstring> soldierAnims = { L"Soldier/Run", L"Soldier/Fire", L"Soldier/Strafing"};
	WriteModelToXML(L"Soldier/Soldier", L"AnimatedMesh_Shader", L"Soldier/Soldier", L"Soldier",
		soldierAnims, L"Resource/Model/SoldierModel.xml");

	vector<wstring> elyAnims = { L"Ely/Run", L"Ely/Fire", L"Ely/Strafing" };
	WriteModelToXML(L"Ely/Ely", L"AnimatedMesh_Shader", L"Ely/Ely", L"Ely",
		elyAnims, L"Resource/Model/ElyAnimsModel.xml");

	SaveScripts();
}

void ResourceManager::AddResource()
{
	_resources->clear();

	// just for particle system
	{
		shared_ptr<Texture> fireParticleTexture = make_shared<Texture>();

		shared_ptr<Texture> randomTexture = make_shared<Texture>();

		vector<wstring> textureVector;
		textureVector.push_back(L"flare0.dds");
		fireParticleTexture->CreateTexture2DArraySRV(textureVector);
		fireParticleTexture->SetName(L"Fire_Particle");
		RESOURCE.AddResource(fireParticleTexture->GetName(), fireParticleTexture);
		
		randomTexture->CreateRandomTexture1DSRV();
		randomTexture->SetName(L"Random_Texture");
		RESOURCE.AddResource(randomTexture->GetName(), randomTexture);
	}
	

	// Resource 폴더의 모든 리소스 로드
	filesystem::path resourcePath = "Resource";

	// Mesh 로드
	LoadResourcesByType(resourcePath / "Mesh", ".xml", [this](const wstring& path) {
		LoadMeshData(path);
		});

	// Texture 로드
	LoadResourcesByType(resourcePath / "Texture", ".xml", [this](const wstring& path) {
		LoadTextureData(path);
		});

	// Shader 로드
	LoadResourcesByType(resourcePath / "Shader", ".xml", [this](const wstring& path) {
		LoadShaderData(path);
		});

	// Material 로드
	LoadResourcesByType(resourcePath / "Material", ".xml", [this](const wstring& path) {
		LoadMaterialData(path);
		});

	// Model 로드
	LoadResourcesByType(resourcePath / "Model", ".xml", [this](const wstring& path) {
		LoadModelData(path);
		});

}
void ResourceManager::LoadResourcesByType(const filesystem::path& folderPath,
	const string& extension, function<void(const wstring&)> loadFunc)
{
	if (!filesystem::exists(folderPath))
		return;

	// 해당 폴더의 모든 파일을 순회
	for (const auto& entry : filesystem::directory_iterator(folderPath))
	{
		if (!entry.is_regular_file())
			continue;

		// 확장자 체크
		if (entry.path().extension() != extension)
			continue;

		// 리소스 로드 함수 호출
		loadFunc(entry.path().wstring());
	}
}

void ResourceManager::WriteScriptToXML(const string& className, const string& displayName, const wstring& finalPath)
{
	auto path = filesystem::path(finalPath);
	filesystem::create_directory(path.parent_path());

	shared_ptr<tinyxml2::XMLDocument> document = make_shared<tinyxml2::XMLDocument>();
	tinyxml2::XMLDeclaration* decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	tinyxml2::XMLElement* root = document->NewElement("Script");
	document->LinkEndChild(root);

	// 클래스 이름 저장
	tinyxml2::XMLElement* classNameElement = document->NewElement("ClassName");
	classNameElement->SetText(className.c_str());
	root->LinkEndChild(classNameElement);

	// 표시 이름 저장
	tinyxml2::XMLElement* displayNameElement = document->NewElement("DisplayName");
	displayNameElement->SetText(displayName.c_str());
	root->LinkEndChild(displayNameElement);

	// cpp 파일 내용 저장
	string cppContent = LoadScriptContent(className, false);
	tinyxml2::XMLElement* cppElement = document->NewElement("CppContent");
	cppElement->SetText(cppContent.c_str());
	root->LinkEndChild(cppElement);

	// 헤더 파일 내용 저장
	string headerContent = LoadScriptContent(className, true);
	tinyxml2::XMLElement* headerElement = document->NewElement("HeaderContent");
	headerElement->SetText(headerContent.c_str());
	root->LinkEndChild(headerElement);

	document->SaveFile(Utils::ToString(finalPath).c_str());
}

void ResourceManager::LoadScriptData(const wstring& path)
{
	shared_ptr<tinyxml2::XMLDocument> document = make_shared<tinyxml2::XMLDocument>();
	string pathStr = Utils::ToString(path);
	document->LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = document->FirstChildElement("Script");
	if (root == nullptr)
		return;

	tinyxml2::XMLElement* contentElement = root->FirstChildElement("Content");
	if (contentElement == nullptr)
		return;

	//_scriptContent = contentElement->GetText();  // GUIManager에서 사용할 수 있도록 저장
}

void ResourceManager::SaveScripts()
{
	const auto& scripts = CF.GetRegisteredScripts();

	for (const auto& [className, info] : scripts)
	{
		wstring finalPath = L"Resource/Script/" + Utils::ToWString(className) + L".xml";
		WriteScriptToXML(className, info.displayName, finalPath);
	}
}

string ResourceManager::LoadScriptContent(const string& className, bool isHeader)
{
	filesystem::path filePath = filesystem::current_path() /
		(className + (isHeader ? ".h" : ".cpp"));

	if (!filesystem::exists(filePath))
		return "";

	std::ifstream file(filePath);
	if (!file.is_open())
		return "";

	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}



void ResourceManager::WriteMeshToXML(const wstring& meshName, const wstring& meshType, const wstring& finalPath)
{
	auto path = filesystem::path(finalPath);
	filesystem::create_directory(path.parent_path());

	shared_ptr<tinyxml2::XMLDocument> document = make_shared<tinyxml2::XMLDocument>();

	// XML 선언 추가
	tinyxml2::XMLDeclaration* decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	// 루트 요소 생성
	tinyxml2::XMLElement* root = document->NewElement("Mesh");
	document->LinkEndChild(root);

	// 메시 이름 저장
	tinyxml2::XMLElement* nameElement = document->NewElement("Name");
	nameElement->SetText(Utils::ToString(meshName).c_str());
	root->LinkEndChild(nameElement);

	// 메시 타입 저장
	tinyxml2::XMLElement* typeElement = document->NewElement("Type");
	typeElement->SetText(Utils::ToString(meshType).c_str());
	root->LinkEndChild(typeElement);

	document->SaveFile(Utils::ToString(finalPath).c_str());
}

void ResourceManager::WriteTextureToXML(const wstring& imagePath, const wstring& textureNamae, const wstring& finalPath)
{
	auto path = filesystem::path(finalPath);
	filesystem::create_directory(path.parent_path());

	shared_ptr<tinyxml2::XMLDocument> document = make_shared<tinyxml2::XMLDocument>();

	// XML 선언 추가
	tinyxml2::XMLDeclaration* decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	// 루트 요소 생성
	tinyxml2::XMLElement* root = document->NewElement("Texture");
	document->LinkEndChild(root);

	// 메시 이름 저장
	tinyxml2::XMLElement* fileElement = document->NewElement("File");
	fileElement->SetText(Utils::ToString(imagePath).c_str());
	root->LinkEndChild(fileElement);

	// 메시 타입 저장
	tinyxml2::XMLElement* nameElement = document->NewElement("Name");
	nameElement->SetText(Utils::ToString(textureNamae).c_str());
	root->LinkEndChild(nameElement);

	document->SaveFile(Utils::ToString(finalPath).c_str());
}

void ResourceManager::WriteShaderToXML(const wstring& shaderPath, const wstring& shaderName, const vector<ShaderType>& shaderTypes, const vector<InputLayoutType>& inputLayouts, const map<wstring, uint32>& slots, const wstring& finalPath)
{
	auto path = filesystem::path(finalPath);
	filesystem::create_directory(path.parent_path());

	shared_ptr<tinyxml2::XMLDocument> document = make_shared<tinyxml2::XMLDocument>();

	// XML 선언 추가
	tinyxml2::XMLDeclaration* decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	// 루트 요소 생성
	tinyxml2::XMLElement* root = document->NewElement("Shader");
	document->LinkEndChild(root);

	// 셰이더 이름 저장
	tinyxml2::XMLElement* nameElement = document->NewElement("Name");
	nameElement->SetText(Utils::ToString(shaderName).c_str());
	root->LinkEndChild(nameElement);

	// 셰이더 파일 경로 저장
	tinyxml2::XMLElement* pathElement = document->NewElement("Path");
	pathElement->SetText(Utils::ToString(shaderPath).c_str());
	root->LinkEndChild(pathElement);

	// ShaderTypes 저장
	tinyxml2::XMLElement* typesElement = document->NewElement("ShaderTypes");
	root->LinkEndChild(typesElement);

	for (const auto& type : shaderTypes)
	{
		tinyxml2::XMLElement* typeElement = document->NewElement("Type");
		typeElement->SetText(static_cast<int>(type));
		typesElement->LinkEndChild(typeElement);
	}

	// InputLayouts 저장
	tinyxml2::XMLElement* layoutsElement = document->NewElement("InputLayouts");
	root->LinkEndChild(layoutsElement);

	for (const auto& layout : inputLayouts)
	{
		tinyxml2::XMLElement* layoutElement = document->NewElement("Layout");
		layoutElement->SetText(static_cast<int>(layout));
		layoutsElement->LinkEndChild(layoutElement);
	}

	// Slots 저장
	tinyxml2::XMLElement* slotsElement = document->NewElement("Slots");
	root->LinkEndChild(slotsElement);

	for (const auto& slot : slots)
	{
		tinyxml2::XMLElement* slotElement = document->NewElement("Slot");
		slotElement->SetAttribute("name", Utils::ToString(slot.first).c_str());
		slotElement->SetAttribute("index", slot.second);
		slotsElement->LinkEndChild(slotElement);
	}

	document->SaveFile(Utils::ToString(finalPath).c_str());
}

void ResourceManager::WriteMaterialToXML(const wstring& textureName, const wstring& normalMapName,
	const MaterialDesc& matDesc, const wstring& shaderName, const wstring& materialName, const wstring& finalPath)
{
	auto path = filesystem::path(finalPath);
	filesystem::create_directory(path.parent_path());

	shared_ptr<tinyxml2::XMLDocument> document = make_shared<tinyxml2::XMLDocument>();
	tinyxml2::XMLDeclaration* decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	tinyxml2::XMLElement* root = document->NewElement("Material");
	document->LinkEndChild(root);

	// Material 이름 저장
	tinyxml2::XMLElement* nameElement = document->NewElement("Name");
	nameElement->SetText(Utils::ToString(materialName).c_str());
	root->LinkEndChild(nameElement);

	// Texture 이름 저장 (있을 경우만)
	if (!textureName.empty())
	{
		tinyxml2::XMLElement* textureElement = document->NewElement("Texture");
		textureElement->SetText(Utils::ToString(textureName).c_str());
		root->LinkEndChild(textureElement);
	}

	// NormalMap 이름 저장 (있을 경우만)
	if (!normalMapName.empty())
	{
		tinyxml2::XMLElement* normalMapElement = document->NewElement("NormalMap");
		normalMapElement->SetText(Utils::ToString(normalMapName).c_str());
		root->LinkEndChild(normalMapElement);
	}

	// Shader 이름 저장
	tinyxml2::XMLElement* shaderElement = document->NewElement("Shader");
	shaderElement->SetText(Utils::ToString(shaderName).c_str());
	root->LinkEndChild(shaderElement);

	// MaterialDesc 저장
	tinyxml2::XMLElement* descElement = document->NewElement("MaterialDesc");
	root->LinkEndChild(descElement);

	// Ambient
	tinyxml2::XMLElement* ambientElement = document->NewElement("Ambient");
	ambientElement->SetAttribute("r", matDesc.ambient.x);
	ambientElement->SetAttribute("g", matDesc.ambient.y);
	ambientElement->SetAttribute("b", matDesc.ambient.z);
	ambientElement->SetAttribute("a", matDesc.ambient.w);
	descElement->LinkEndChild(ambientElement);

	// Diffuse
	tinyxml2::XMLElement* diffuseElement = document->NewElement("Diffuse");
	diffuseElement->SetAttribute("r", matDesc.diffuse.x);
	diffuseElement->SetAttribute("g", matDesc.diffuse.y);
	diffuseElement->SetAttribute("b", matDesc.diffuse.z);
	diffuseElement->SetAttribute("a", matDesc.diffuse.w);
	descElement->LinkEndChild(diffuseElement);

	// Specular
	tinyxml2::XMLElement* specularElement = document->NewElement("Specular");
	specularElement->SetAttribute("r", matDesc.specular.x);
	specularElement->SetAttribute("g", matDesc.specular.y);
	specularElement->SetAttribute("b", matDesc.specular.z);
	specularElement->SetAttribute("a", matDesc.specular.w);
	descElement->LinkEndChild(specularElement);

	document->SaveFile(Utils::ToString(finalPath).c_str());
}

void ResourceManager::WriteModelToXML(const wstring& modelPath, const wstring& shaderName, const wstring& materialPath, const wstring& modelName, const vector<wstring>& animPaths, const wstring& finalPath)
{
	auto path = filesystem::path(finalPath);
	filesystem::create_directory(path.parent_path());

	shared_ptr<tinyxml2::XMLDocument> document = make_shared<tinyxml2::XMLDocument>();
	tinyxml2::XMLDeclaration* decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	tinyxml2::XMLElement* root = document->NewElement("Model");
	document->LinkEndChild(root);

	// Model 이름 저장
	tinyxml2::XMLElement* nameElement = document->NewElement("Name");
	nameElement->SetText(Utils::ToString(modelName).c_str());
	root->LinkEndChild(nameElement);

	// Model 경로 저장
	tinyxml2::XMLElement* pathElement = document->NewElement("Path");
	pathElement->SetText(Utils::ToString(modelPath).c_str());
	root->LinkEndChild(pathElement);

	// Shader 이름 저장
	tinyxml2::XMLElement* shaderElement = document->NewElement("Shader");
	shaderElement->SetText(Utils::ToString(shaderName).c_str());
	root->LinkEndChild(shaderElement);

	// Material 경로 저장
	tinyxml2::XMLElement* materialElement = document->NewElement("Material");
	materialElement->SetText(Utils::ToString(materialPath).c_str());
	root->LinkEndChild(materialElement);

	// Animation 경로들 저장
	if (!animPaths.empty())
	{
		tinyxml2::XMLElement* animationsElement = document->NewElement("Animations");
		root->LinkEndChild(animationsElement);

		for (const auto& animPath : animPaths)
		{
			tinyxml2::XMLElement* animElement = document->NewElement("Animation");
			animElement->SetText(Utils::ToString(animPath).c_str());
			animationsElement->LinkEndChild(animElement);
		}
	}

	document->SaveFile(Utils::ToString(finalPath).c_str());
}

void ResourceManager::LoadMeshData(wstring path)
{
	shared_ptr<tinyxml2::XMLDocument> document = make_shared<tinyxml2::XMLDocument>();
	string pathStr = Utils::ToString(path);
	document->LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = document->FirstChildElement("Mesh");  // "Mesh" 루트 요소를 찾음
	if (root == nullptr)
	{
		// XML 파일 구조가 잘못됐을 경우 처리
		return;
	}

	tinyxml2::XMLElement* nameElement = root->FirstChildElement("Name");
	tinyxml2::XMLElement* typeElement = root->FirstChildElement("Type");

	if (nameElement == nullptr || typeElement == nullptr)
	{
		// 필수 요소가 없을 경우 처리
		return;
	}

	string name = nameElement->GetText();
	string type = typeElement->GetText();

	shared_ptr<Mesh> mesh = make_shared<Mesh>();

	// 메시 타입에 따라 적절한 생성 함수 호출
	if (type == "Sphere")
		mesh->CreateSphere_NormalTangent();
	else if (type == "Cube")
		mesh->CreateCube_NormalTangent();
	else if (type == "Grid")
		mesh->CreateGrid_NormalTangent(100, 100);
	else if (type == "Terrain")
		mesh->CreateTerrain();
	else if (type == "Quad")
		mesh->CreateQuad_NormalTangent();
	else if (type == "Cylinder")
		mesh->CreateCylinder_NormalTangent();

	mesh->SetName(Utils::ToWString(name));
	AddResource(mesh->GetName(), mesh);
}

void ResourceManager::LoadTextureData(wstring path)
{
	shared_ptr<tinyxml2::XMLDocument> document = make_shared<tinyxml2::XMLDocument>();
	string pathStr = Utils::ToString(path);
	document->LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = document->FirstChildElement("Texture");  // "Mesh" 루트 요소를 찾음
	if (root == nullptr)
	{
		// XML 파일 구조가 잘못됐을 경우 처리
		abort();
		return;
	}

	tinyxml2::XMLElement* fileElement = root->FirstChildElement("File");
	tinyxml2::XMLElement* nameElement = root->FirstChildElement("Name");

	if (fileElement == nullptr || nameElement == nullptr)
	{
		// 필수 요소가 없을 경우 처리
		return;
	}

	string file = fileElement->GetText();
	string name = nameElement->GetText();
	

	shared_ptr<Texture> texture = make_shared<Texture>();
	texture->CreateTexture(Utils::ToWString(file));
	texture->SetName(Utils::ToWString(name));
	RESOURCE.AddResource(texture->GetName(), texture);

}

void ResourceManager::LoadShaderData(wstring path)
{
	shared_ptr<tinyxml2::XMLDocument> document = make_shared<tinyxml2::XMLDocument>();
	string pathStr = Utils::ToString(path);
	document->LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = document->FirstChildElement("Shader");
	if (root == nullptr)
		return;

	// 기본 정보 읽기
	string name = root->FirstChildElement("Name")->GetText();
	string shaderPath = root->FirstChildElement("Path")->GetText();

	shared_ptr<Shader> shader = make_shared<Shader>();

	// ShaderTypes 읽기
	tinyxml2::XMLElement* typesElement = root->FirstChildElement("ShaderTypes");
	tinyxml2::XMLElement* typeElement = typesElement->FirstChildElement("Type");

	// InputLayouts 읽기
	tinyxml2::XMLElement* layoutsElement = root->FirstChildElement("InputLayouts");
	tinyxml2::XMLElement* layoutElement = layoutsElement->FirstChildElement("Layout");

	// ShaderTypes와 InputLayouts 매칭하여 Shader 생성
	while (typeElement && layoutElement)
	{
		ShaderType type = static_cast<ShaderType>(atoi(typeElement->GetText()));
		InputLayoutType layout = static_cast<InputLayoutType>(atoi(layoutElement->GetText()));

		shader->CreateShader(type, Utils::ToWString(shaderPath), layout);

		typeElement = typeElement->NextSiblingElement("Type");
		layoutElement = layoutElement->NextSiblingElement("Layout");
	}

	// Slots 읽기
	tinyxml2::XMLElement* slotsElement = root->FirstChildElement("Slots");
	tinyxml2::XMLElement* slotElement = slotsElement->FirstChildElement("Slot");

	while (slotElement)
	{
		const char* slotName = slotElement->Attribute("name");
		int slotIndex = 0;
		slotElement->QueryIntAttribute("index", &slotIndex);

		shader->GetShaderSlot()->SetSlot(Utils::ToWString(slotName), slotIndex);

		slotElement = slotElement->NextSiblingElement("Slot");
	}

	shader->SetName(Utils::ToWString(name));
	AddResource(shader->GetName(), shader);
}

void ResourceManager::LoadMaterialData(const wstring& path)
{
	shared_ptr<tinyxml2::XMLDocument> document = make_shared<tinyxml2::XMLDocument>();
	string pathStr = Utils::ToString(path);
	document->LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = document->FirstChildElement("Material");
	if (root == nullptr)
		return;

	// 기본 정보 읽기
	string name = root->FirstChildElement("Name")->GetText();
	string shaderName = root->FirstChildElement("Shader")->GetText();

	// Material 생성 및 설정
	shared_ptr<Material> material = make_shared<Material>();

	// Texture 설정 (있을 경우만)
	tinyxml2::XMLElement* textureElement = root->FirstChildElement("Texture");
	if (textureElement)
	{
		string textureName = textureElement->GetText();
		material->SetTexture(RESOURCE.GetResource<Texture>(Utils::ToWString(textureName)));
	}

	// NormalMap 설정 (있을 경우만)
	tinyxml2::XMLElement* normalMapElement = root->FirstChildElement("NormalMap");
	if (normalMapElement)
	{
		string normalMapName = normalMapElement->GetText();
		material->SetNormalMap(RESOURCE.GetResource<Texture>(Utils::ToWString(normalMapName)));
	}

	// MaterialDesc 읽기
	MaterialDesc matDesc;
	tinyxml2::XMLElement* descElement = root->FirstChildElement("MaterialDesc");
	if (descElement)
	{
		tinyxml2::XMLElement* ambientElement = descElement->FirstChildElement("Ambient");
		matDesc.ambient.x = ambientElement->FloatAttribute("r");
		matDesc.ambient.y = ambientElement->FloatAttribute("g");
		matDesc.ambient.z = ambientElement->FloatAttribute("b");
		matDesc.ambient.w = ambientElement->FloatAttribute("a");

		tinyxml2::XMLElement* diffuseElement = descElement->FirstChildElement("Diffuse");
		matDesc.diffuse.x = diffuseElement->FloatAttribute("r");
		matDesc.diffuse.y = diffuseElement->FloatAttribute("g");
		matDesc.diffuse.z = diffuseElement->FloatAttribute("b");
		matDesc.diffuse.w = diffuseElement->FloatAttribute("a");

		tinyxml2::XMLElement* specularElement = descElement->FirstChildElement("Specular");
		matDesc.specular.x = specularElement->FloatAttribute("r");
		matDesc.specular.y = specularElement->FloatAttribute("g");
		matDesc.specular.z = specularElement->FloatAttribute("b");
		matDesc.specular.w = specularElement->FloatAttribute("a");
	}
	else
	{
		matDesc.ambient = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		matDesc.diffuse = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
		matDesc.specular = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	material->SetMaterialDesc(matDesc);
	material->SetShader(RESOURCE.GetResource<Shader>(Utils::ToWString(shaderName)));
	material->SetName(Utils::ToWString(name));

	RESOURCE.AddResource(material->GetName(), material);
}

void ResourceManager::LoadModelData(const wstring& path)
{
	shared_ptr<tinyxml2::XMLDocument> document = make_shared<tinyxml2::XMLDocument>();
	string pathStr = Utils::ToString(path);
	document->LoadFile(pathStr.c_str());

	tinyxml2::XMLElement* root = document->FirstChildElement("Model");
	if (root == nullptr)
		return;

	// 기본 정보 읽기
	string name = root->FirstChildElement("Name")->GetText();
	string modelPath = root->FirstChildElement("Path")->GetText();
	string shaderName = root->FirstChildElement("Shader")->GetText();
	string materialPath = root->FirstChildElement("Material")->GetText();

	// Model 생성 및 설정
	shared_ptr<Model> model = make_shared<Model>();
	model->ReadModel(Utils::ToWString(modelPath));
	model->SetShaderForMaterial(RESOURCE.GetResource<Shader>(Utils::ToWString(shaderName)));
	model->ReadMaterial(Utils::ToWString(materialPath));

	// Animation 읽기 (있는 경우)
	tinyxml2::XMLElement* animationsElement = root->FirstChildElement("Animations");
	if (animationsElement)
	{
		tinyxml2::XMLElement* animElement = animationsElement->FirstChildElement("Animation");
		while (animElement)
		{
			string animPath = animElement->GetText();
			model->ReadAnimation(Utils::ToWString(animPath));
			animElement = animElement->NextSiblingElement("Animation");
		}
		model->CreateTexture();
	}

	model->SetName(Utils::ToWString(name));
	RESOURCE.AddResource(model->GetName(), model);
}

