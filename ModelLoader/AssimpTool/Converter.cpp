#include "pch.h"
#include "Converter.h"
#include <filesystem>
#include "Utils.h"
#include "tinyxml2.h"
#include "FileUtils.h"
#include <regex>
#include <unordered_set>
Converter::Converter()
{
	_importer = make_shared<Assimp::Importer>();

}

Converter::~Converter()
{

}

void Converter::init(HWND hwnd)
{
	_hwnd = hwnd;
	CreateDeviceAndSwapChain();
}

void Converter::ReadAssetFile(wstring file)
{
	wstring fileStr = _assetPath + file;

	auto p = std::filesystem::path(fileStr);
	assert(std::filesystem::exists(p));

	_scene = _importer->ReadFile(
		Utils::ToString(fileStr),
		aiProcess_ConvertToLeftHanded |
		aiProcess_Triangulate |
		aiProcess_GenUVCoords |
		aiProcess_GenNormals |
		aiProcess_CalcTangentSpace
	);

	assert(_scene != nullptr);
}

void Converter::ExportModelData(wstring savePath)
{
	wstring finalPath = _modelPath + savePath + L".mesh";
	ReadModelData(_scene->mRootNode, -1, -1);
	ReadSkinData();

	// Normalize bone names to ensure compatibility with animation data
	NormalizeBoneNames();

	// Check and log bone hierarchy consistency
	CheckAndFixBoneHierarchy();

	// Normalize skin weights to ensure proper animation
	NormalizeSkinWeights();

	// Save mesh data to CSV for debugging purposes
	SaveMeshDataToCSV();

	WriteModelFile(finalPath);
}

void Converter::ExportMaterialData(wstring savePath)
{
	wstring finalPath = _texturePath + savePath + L".xml";
	ReadMaterialData();
	WriteMaterialData(finalPath);
}

void Converter::ExportAnimationData(wstring savePath, uint32 index)
{
	wstring finalPath = _modelPath + savePath + L".clip";
	assert(index < _scene->mNumAnimations);
	shared_ptr<asAnimation> animation = ReadAnimationData(_scene->mAnimations[index]);

	// Normalize bone names to ensure compatibility with mesh data
	NormalizeAnimationBoneNames(animation);

	//// Save animation data to CSV for debugging purposes
	//SaveAnimationDataToCSV(animation);

	// Save animation keyframe data to a log file for further debugging
	string logFileName = "../AnimationKeyframeLog.txt";
	FILE* logFile;
	::fopen_s(&logFile, logFileName.c_str(), "w");
	for (shared_ptr<asKeyframe> keyframe : animation->keyframes)
	{
		::fprintf(logFile, "Bone: %s\n", keyframe->boneName.c_str());
		for (const auto& transform : keyframe->transforms)
		{
			::fprintf(logFile, "Time: %f\n", transform.time);
			::fprintf(logFile, "Translation: [%f, %f, %f]\n", transform.translation.x, transform.translation.y, transform.translation.z);
			::fprintf(logFile, "Rotation: [%f, %f, %f, %f]\n", transform.rotation.x, transform.rotation.y, transform.rotation.z, transform.rotation.w);
			::fprintf(logFile, "Scale: [%f, %f, %f]\n", transform.scale.x, transform.scale.y, transform.scale.z);
			::fprintf(logFile, "\n");
		}
	}
	::fclose(logFile);

	WriteAnimationData(animation, finalPath);
}

void Converter::NormalizeBoneNames()
{
	// List of possible prefixes and suffixes to remove
	std::vector<std::string> prefixes = { "mixamorig:", "Bone_", "Root_", "Bip01_", "Dummy_", "_joint" };
	std::vector<std::string> suffixes = { "_End", "$AssimpFbx$_PreRotation", "$AssimpFbx$_Translation", "$AssimpFbx$_Scaling" };

	auto removePrefixesAndSuffixes = [&](std::string& name) {
		// Repeatedly remove known prefixes
		bool modified = true;
		while (modified)
		{
			modified = false;
			for (const auto& prefix : prefixes)
			{
				if (name.find(prefix) == 0)
				{
					name.erase(0, prefix.size());
					modified = true;
					break;
				}
			}
		}

		// Repeatedly remove known suffixes
		modified = true;
		while (modified)
		{
			modified = false;
			for (const auto& suffix : suffixes)
			{
				if (name.size() >= suffix.size() && name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0)
				{
					name.erase(name.size() - suffix.size());
					modified = true;
					break;
				}
			}
		}
	};

	// Normalize bone names in the _bones vector
	for (shared_ptr<asBone>& bone : _bones)
	{
		removePrefixesAndSuffixes(bone->name);

		// Remove any remaining non-alphanumeric characters
		bone->name.erase(std::remove_if(bone->name.begin(), bone->name.end(), [](char c) { return !std::isalnum(c) && c != '_'; }), bone->name.end());
	}

	// Normalize bone names in the _meshes vector
	for (shared_ptr<asMesh>& mesh : _meshes)
	{
		removePrefixesAndSuffixes(mesh->name);

		// Remove any remaining non-alphanumeric characters
		mesh->name.erase(std::remove_if(mesh->name.begin(), mesh->name.end(), [](char c) { return !std::isalnum(c) && c != '_'; }), mesh->name.end());

		// Update bone index based on normalized name
		mesh->boneIndex = GetBoneIndex(mesh->name);
	}
}
void Converter::NormalizeAnimationBoneNames(shared_ptr<asAnimation> animation)
{
	// List of possible prefixes and suffixes to remove
	std::vector<std::string> prefixes = { "mixamorig:", "Bone_", "Root_", "Bip01_", "Dummy_", "_joint" };
	std::vector<std::string> suffixes = { "_End", "$AssimpFbx$_PreRotation", "$AssimpFbx$_Translation", "$AssimpFbx$_Scaling" };

	auto removePrefixesAndSuffixes = [&](std::string& name) {
		// Repeatedly remove known prefixes
		bool modified = true;
		while (modified)
		{
			modified = false;
			for (const auto& prefix : prefixes)
			{
				if (name.find(prefix) == 0)
				{
					name.erase(0, prefix.size());
					modified = true;
					break;
				}
			}
		}

		// Repeatedly remove known suffixes
		modified = true;
		while (modified)
		{
			modified = false;
			for (const auto& suffix : suffixes)
			{
				if (name.size() >= suffix.size() && name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0)
				{
					name.erase(name.size() - suffix.size());
					modified = true;
					break;
				}
			}
		}
	};

	// Normalize bone names in the animation keyframes
	for (shared_ptr<asKeyframe> keyframe : animation->keyframes)
	{
		removePrefixesAndSuffixes(keyframe->boneName);

		// Remove any remaining non-alphanumeric characters
		keyframe->boneName.erase(std::remove_if(keyframe->boneName.begin(), keyframe->boneName.end(), [](char c) { return !std::isalnum(c) && c != '_'; }), keyframe->boneName.end());
	}
}


void Converter::SaveMeshDataToCSV()
{
	//// CSV file path for mesh data
	//string csvFileName = "../NormalizedMeshData.csv";
	//FILE* csvFile;
	//::fopen_s(&csvFile, csvFileName.c_str(), "w");

	//// Header for CSV
	//::fprintf(csvFile, "BoneIndex,BoneName,ParentIndex\n");
	//for (shared_ptr<asBone>& bone : _bones)
	//{
	//	::fprintf(csvFile, "%d,%s,%d\n", bone->index, bone->name.c_str(), bone->parent);
	//}

	//::fprintf(csvFile, "\n");
	//for (shared_ptr<asMesh>& mesh : _meshes)
	//{
	//	::fprintf(csvFile, "MeshName,%s\n", mesh->name.c_str());
	//	::fprintf(csvFile, "VertexIndex,PositionX,PositionY,PositionZ,BlendIndex0,BlendIndex1,BlendIndex2,BlendIndex3,BlendWeight0,BlendWeight1,BlendWeight2,BlendWeight3\n");

	//	for (UINT i = 0; i < mesh->vertices.size(); i++)
	//	{
	//		const VertexTextureNormalTangentBlendData& vertex = mesh->vertices[i];
	//		Vec3 p = vertex.position;
	//		Vec4 indices = vertex.blendIndices;
	//		Vec4 weights = vertex.blendWeights;

	//		::fprintf(csvFile, "%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
	//			i, p.x, p.y, p.z,
	//			indices.x, indices.y, indices.z, indices.w,
	//			weights.x, weights.y, weights.z, weights.w);
	//	}

	//	::fprintf(csvFile, "\n");
	//}

	//::fclose(csvFile);
	// CSV file path for mesh data
	string csvFileName = "../SimplifiedNormalizedMeshData.csv";
	FILE* csvFile;
	::fopen_s(&csvFile, csvFileName.c_str(), "w");

	// Header for CSV
	::fprintf(csvFile, "BoneIndex,BoneName\n");
	for (shared_ptr<asBone>& bone : _bones)
	{
		::fprintf(csvFile, "%d,%s\n", bone->index, bone->name.c_str());
	}

	::fclose(csvFile);
}

void Converter::SaveAnimationDataToCSV(shared_ptr<asAnimation> animation)
{
	//// CSV file path for animation data
	//string csvFileName = "../NormalizedAnimationData.csv";
	//FILE* csvFile;
	//::fopen_s(&csvFile, csvFileName.c_str(), "w");

	//// Header for CSV
	//::fprintf(csvFile, "Frame,BoneName,Time,TranslationX,TranslationY,TranslationZ,RotationX,RotationY,RotationZ,RotationW,ScaleX,ScaleY,ScaleZ\n");

	//for (shared_ptr<asKeyframe> keyframe : animation->keyframes)
	//{
	//	string boneName = keyframe->boneName;
	//	uint32 transformCount = keyframe->transforms.size();

	//	for (uint32 i = 0; i < transformCount; ++i)
	//	{
	//		asKeyframeData& frameData = keyframe->transforms[i];
	//		float time = frameData.time;
	//		Vec3& translation = frameData.translation;
	//		Quaternion& rotation = frameData.rotation;
	//		Vec3& scale = frameData.scale;

	//		::fprintf(csvFile, "%d,%s,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f\n",
	//			i, boneName.c_str(), time,
	//			translation.x, translation.y, translation.z,
	//			rotation.x, rotation.y, rotation.z, rotation.w,
	//			scale.x, scale.y, scale.z);
	//	}
	//}

	//::fclose(csvFile);
	// CSV file path for animation data
	string csvFileName = "../SimplifiedNormalizedAnimationData.csv";
	FILE* csvFile;
	::fopen_s(&csvFile, csvFileName.c_str(), "w");

	// Header for CSV
	::fprintf(csvFile, "BoneName\n");

	for (shared_ptr<asKeyframe> keyframe : animation->keyframes)
	{
		string boneName = keyframe->boneName;
		::fprintf(csvFile, "%s\n", boneName.c_str());
	}

	::fclose(csvFile);
}

void Converter::CheckAndFixBoneHierarchy()
{
	// Verify bone hierarchy consistency by logging discrepancies and saving to a text file
	string logFileName = "../BoneHierarchyCheckLog.txt";
	FILE* logFile;
	::fopen_s(&logFile, logFileName.c_str(), "w");

	for (shared_ptr<asBone>& bone : _bones)
	{
		if (bone->parent >= 0 && bone->parent < _bones.size())
		{
			auto parentBone = _bones[bone->parent];
			if (parentBone == nullptr || parentBone->index != bone->parent)
			{
				// Log any discrepancies found
				::fprintf(logFile, "Discrepancy found in bone hierarchy for bone: %s with parent index: %d\n", bone->name.c_str(), bone->parent);
			}
		}
		else if (bone->parent >= 0)
		{
			// Log if the parent index is out of bounds
			::fprintf(logFile, "Invalid parent index for bone: %s, parent index: %d is out of bounds\n", bone->name.c_str(), bone->parent);
		}
		else
		{
			// Log the bone's transform matrix for debugging purposes
			::fprintf(logFile, "Bone: %s, Index: %d, Parent Index: %d\n", bone->name.c_str(), bone->index, bone->parent);
			::fprintf(logFile, "Transform Matrix:\n");
			for (int i = 0; i < 4; ++i)
			{
				::fprintf(logFile, "[%f, %f, %f, %f]\n", bone->transform.m[i][0], bone->transform.m[i][1], bone->transform.m[i][2], bone->transform.m[i][3]);
			}
			::fprintf(logFile, "\n");
		}
	}

	::fclose(logFile);
}

void Converter::NormalizeSkinWeights()
{
	// Ensure skin weights are properly normalized
	string logFileName = "../SkinWeightNormalizationLog.txt";
	FILE* logFile;
	::fopen_s(&logFile, logFileName.c_str(), "w");

	for (shared_ptr<asMesh>& mesh : _meshes)
	{
		for (auto& vertex : mesh->vertices)
		{
			float totalWeight = vertex.blendWeights.x + vertex.blendWeights.y + vertex.blendWeights.z + vertex.blendWeights.w;
			if (totalWeight > 0.0f)
			{
				vertex.blendWeights.x /= totalWeight;
				vertex.blendWeights.y /= totalWeight;
				vertex.blendWeights.z /= totalWeight;
				vertex.blendWeights.w /= totalWeight;
			}
			else
			{
				::fprintf(logFile, "Vertex at position (%f, %f, %f) has zero total weight.\n", vertex.position.x, vertex.position.y, vertex.position.z);
			}
			// Log the final normalized weights for each vertex
			::fprintf(logFile, "Vertex at position (%f, %f, %f) - Weights: [%f, %f, %f, %f]\n", vertex.position.x, vertex.position.y, vertex.position.z, vertex.blendWeights.x, vertex.blendWeights.y, vertex.blendWeights.z, vertex.blendWeights.w);
		}
	}

	::fclose(logFile);
}


void Converter::CreateDeviceAndSwapChain()
{
	DXGI_SWAP_CHAIN_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	{
		int32 _width = 800;
		int32 _height = 600;
		desc.BufferDesc.Width = _width;
		desc.BufferDesc.Height = _height;
		desc.BufferDesc.RefreshRate.Numerator = 60;
		desc.BufferDesc.RefreshRate.Denominator = 1;
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = 1;
		desc.OutputWindow = _hwnd;
		desc.Windowed = true;
		desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	}

	HRESULT hr = ::D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&desc,
		_swapChain.GetAddressOf(),
		_device.GetAddressOf(),
		nullptr,
		_deviceContext.GetAddressOf()
	);

	CHECK(hr);
}

void Converter::ReadModelData(aiNode* node, int32 index, int32 parent)
{
	shared_ptr<asBone> bone = make_shared<asBone>();
	bone->index = index;
	bone->parent = parent;
	bone->name = node->mName.C_Str();

	// Relative Transform
	Matrix transform(node->mTransformation[0]);
	bone->transform = transform.Transpose(); // 직속 부모를 기준으로한 위치 

	// 2) Root (Local)
	Matrix matParent = Matrix::Identity;
	if (parent >= 0)
		matParent = _bones[parent]->transform;

	// Local (Root) Transform
	bone->transform = bone->transform * matParent; // 이 과정이 반복되면 사실상 bone의 transform은 직속 부모를 기준으로 한게 아닌 root기준의 위치가 됨

	_bones.push_back(bone);

	// Mesh
	ReadMeshData(node, index);

	// 재귀 함수
	for (uint32 i = 0; i < node->mNumChildren; i++)
		ReadModelData(node->mChildren[i], _bones.size(), index);
}

void Converter::ReadMeshData(aiNode* node, int32 bone)
{
	if (node->mNumMeshes < 1) // node가 항상 뼈와 관련된 정보만 들어있는게 아니기 때문에 mesh 정보가 있는경우만 체크
		return;

	shared_ptr<asMesh> mesh = make_shared<asMesh>();
	mesh->name = node->mName.C_Str();
	mesh->boneIndex = bone;

	for (uint32 i = 0; i < node->mNumMeshes; i++)
	{
		uint32 index = node->mMeshes[i]; // scene이 가지고 있는 Meshes의 인덱스 번호
		const aiMesh* srcMesh = _scene->mMeshes[index];

		// Material Name
		const aiMaterial* material = _scene->mMaterials[srcMesh->mMaterialIndex];
		mesh->materialName = material->GetName().C_Str();

		const uint32 startVertex = mesh->vertices.size();

		for (uint32 v = 0; v < srcMesh->mNumVertices; v++)
		{
			// Vertex
			VertexTextureNormalTangentBlendData vertex;
			::memcpy(&vertex.position, &srcMesh->mVertices[v], sizeof(Vec3));

			// UV
			if (srcMesh->HasTextureCoords(0))
				::memcpy(&vertex.uv, &srcMesh->mTextureCoords[0][v], sizeof(Vec2));

			// Normal
			if (srcMesh->HasNormals())
				::memcpy(&vertex.normal, &srcMesh->mNormals[v], sizeof(Vec3));

			mesh->vertices.push_back(vertex);
		}

		// Index
		for (uint32 f = 0; f < srcMesh->mNumFaces; f++)
		{
			aiFace& face = srcMesh->mFaces[f];

			for (uint32 k = 0; k < face.mNumIndices; k++)
				mesh->indices.push_back(face.mIndices[k] + startVertex);
		}
	}

	_meshes.push_back(mesh);
}

void Converter::ReadSkinData()
{
	for (uint32 i = 0; i < _scene->mNumMeshes; i++)
	{
		aiMesh* srcMesh = _scene->mMeshes[i]; // 원본 mesh
		if (srcMesh->HasBones() == false)
			continue;

		shared_ptr<asMesh> mesh = _meshes[i]; // 커스텀된 mesh

		vector<asBoneWeights> tempVertexBoneWeights;
		tempVertexBoneWeights.resize(mesh->vertices.size());

		// Bone을 순회하면서 연관된 VertexId, Weight를 구해서 기록한다.
		for (uint32 b = 0; b < srcMesh->mNumBones; b++)
		{
			aiBone* srcMeshBone = srcMesh->mBones[b];
			uint32 boneIndex = GetBoneIndex(srcMeshBone->mName.C_Str()); // 우리가 파싱한 뼈의 인덱스 추출

			for (uint32 w = 0; w < srcMeshBone->mNumWeights; w++)
			{
				uint32 index = srcMeshBone->mWeights[w].mVertexId;
				float weight = srcMeshBone->mWeights[w].mWeight;
				tempVertexBoneWeights[index].AddWeights(boneIndex, weight);
			}
		}

		// 최종 결과 계산
		for (uint32 v = 0; v < tempVertexBoneWeights.size(); v++)
		{
			tempVertexBoneWeights[v].Normalize();

			asBlendWeight blendWeight = tempVertexBoneWeights[v].GetBlendWeights();
			mesh->vertices[v].blendIndices = blendWeight.indices;
			mesh->vertices[v].blendWeights = blendWeight.weights;
		}
	}
}

void Converter::WriteModelFile(wstring finalPath)
{
	auto path = filesystem::path(finalPath);

	// 폴더가 없으면 만든다.
	filesystem::create_directory(path.parent_path());

	shared_ptr<FileUtils> file = make_shared<FileUtils>();
	file->Open(finalPath, FileMode::Write);

	// Bone Data
	file->Write<uint32>(_bones.size());
	for (shared_ptr<asBone>& bone : _bones)
	{
		file->Write<int32>(bone->index);
		file->Write<string>(bone->name);
		file->Write<int32>(bone->parent);
		file->Write<Matrix>(bone->transform);
	}

	// Mesh Data
	file->Write<uint32>(_meshes.size());
	for (shared_ptr<asMesh>& meshData : _meshes)
	{
		file->Write<string>(meshData->name);
		file->Write<int32>(meshData->boneIndex);
		file->Write<string>(meshData->materialName);

		// Vertex Data
		file->Write<uint32>(meshData->vertices.size());
		file->Write(&meshData->vertices[0], sizeof(VertexTextureNormalTangentBlendData) * meshData->vertices.size());

		// Index Data
		file->Write<uint32>(meshData->indices.size());
		file->Write(&meshData->indices[0], sizeof(uint32) * meshData->indices.size());
	}
}

void Converter::ReadMaterialData()
{
	for (uint32 i = 0; i < _scene->mNumMaterials; i++)
	{
		aiMaterial* srcMaterial = _scene->mMaterials[i];

		shared_ptr<asMaterial> material = make_shared<asMaterial>();
		material->name = srcMaterial->GetName().C_Str();

		aiColor3D color;
		// Ambient
		srcMaterial->Get(AI_MATKEY_COLOR_AMBIENT, color);
		material->ambient = Color(color.r, color.g, color.b, 1.f);

		// Diffuse
		srcMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		material->diffuse = Color(color.r, color.g, color.b, 1.f);

		// Specular
		srcMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color);
		material->specular = Color(color.r, color.g, color.b, 1.f);
		srcMaterial->Get(AI_MATKEY_SHININESS, material->specular.w);

		// Emissive
		srcMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, color);
		material->emissive = Color(color.r, color.g, color.b, 1.0f);

		aiString file;

		// Diffuse Texture
		srcMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &file);
		material->diffuseFile = file.C_Str();

		// Specular Texture
		srcMaterial->GetTexture(aiTextureType_SPECULAR, 0, &file);
		material->specularFile = file.C_Str();

		// Normal Texture
		srcMaterial->GetTexture(aiTextureType_NORMALS, 0, &file);
		material->normalFile = file.C_Str();

		_materials.push_back(material);
	}
}

void Converter::WriteMaterialData(wstring finalPath)
{
	auto path = filesystem::path(finalPath);

	// 폴더가 없으면 만든다.
	filesystem::create_directory(path.parent_path());

	string folder = path.parent_path().string();

	shared_ptr<tinyxml2::XMLDocument> document = make_shared<tinyxml2::XMLDocument>();

	tinyxml2::XMLDeclaration* decl = document->NewDeclaration();
	document->LinkEndChild(decl);

	tinyxml2::XMLElement* root = document->NewElement("Materials");
	document->LinkEndChild(root);

	for (shared_ptr<asMaterial> material : _materials)
	{
		tinyxml2::XMLElement* node = document->NewElement("Material");
		root->LinkEndChild(node);

		tinyxml2::XMLElement* element = nullptr;

		element = document->NewElement("Name");
		element->SetText(material->name.c_str());
		node->LinkEndChild(element);

		element = document->NewElement("DiffuseFile");
		element->SetText(WriteTexture(folder, material->diffuseFile).c_str());
		node->LinkEndChild(element);

		element = document->NewElement("SpecularFile");
		element->SetText(WriteTexture(folder, material->specularFile).c_str());
		node->LinkEndChild(element);

		element = document->NewElement("NormalFile");
		element->SetText(WriteTexture(folder, material->normalFile).c_str());
		node->LinkEndChild(element);

		element = document->NewElement("Ambient");
		element->SetAttribute("R", material->ambient.x);
		element->SetAttribute("G", material->ambient.y);
		element->SetAttribute("B", material->ambient.z);
		element->SetAttribute("A", material->ambient.w);
		node->LinkEndChild(element);

		element = document->NewElement("Diffuse");
		element->SetAttribute("R", material->diffuse.x);
		element->SetAttribute("G", material->diffuse.y);
		element->SetAttribute("B", material->diffuse.z);
		element->SetAttribute("A", material->diffuse.w);
		node->LinkEndChild(element);

		element = document->NewElement("Specular");
		element->SetAttribute("R", material->specular.x);
		element->SetAttribute("G", material->specular.y);
		element->SetAttribute("B", material->specular.z);
		element->SetAttribute("A", material->specular.w);
		node->LinkEndChild(element);

		element = document->NewElement("Emissive");
		element->SetAttribute("R", material->emissive.x);
		element->SetAttribute("G", material->emissive.y);
		element->SetAttribute("B", material->emissive.z);
		element->SetAttribute("A", material->emissive.w);
		node->LinkEndChild(element);
	}

	document->SaveFile(Utils::ToString(finalPath).c_str());
}

std::string Converter::WriteTexture(string saveFolder, string file)
{
	string fileName = filesystem::path(file).filename().string();
	string folderName = filesystem::path(saveFolder).filename().string();

	const aiTexture* srcTexture = _scene->GetEmbeddedTexture(file.c_str()); // fbx파일 안에 텍스처가 있는경우
	if (srcTexture)
	{
		string pathStr = (filesystem::path(saveFolder) / fileName).string();

		if (srcTexture->mHeight == 0)
		{
			shared_ptr<FileUtils> file = make_shared<FileUtils>();
			file->Open(Utils::ToWString(pathStr), FileMode::Write);
			file->Write(srcTexture->pcData, srcTexture->mWidth);
		}
		else
		{
			D3D11_TEXTURE2D_DESC desc;
			ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
			desc.Width = srcTexture->mWidth;
			desc.Height = srcTexture->mHeight;
			desc.MipLevels = 1;
			desc.ArraySize = 1;
			desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			desc.SampleDesc.Count = 1;
			desc.SampleDesc.Quality = 0;
			desc.Usage = D3D11_USAGE_IMMUTABLE;

			D3D11_SUBRESOURCE_DATA subResource = { 0 };
			subResource.pSysMem = srcTexture->pcData;

			ComPtr<ID3D11Texture2D> texture;
			HRESULT hr = _device->CreateTexture2D(&desc, &subResource, texture.GetAddressOf());
			CHECK(hr);

			DirectX::ScratchImage img;
			::CaptureTexture(_device.Get(), _deviceContext.Get(), texture.Get(), img);

			// Save To File
			hr = DirectX::SaveToDDSFile(*img.GetImages(), DirectX::DDS_FLAGS_NONE, Utils::ToWString(fileName).c_str());
			CHECK(hr);
		}
	}
	else
	{
		string originStr = (filesystem::path(_assetPath) / folderName / file).string();
		Utils::Replace(originStr, "\\", "/"); // \\를 /로 바꿈

		string pathStr = (filesystem::path(saveFolder) / fileName).string();
		Utils::Replace(pathStr, "\\", "/");

		::CopyFileA(originStr.c_str(), pathStr.c_str(), false);//originStr에 있는 파일을 pathStr에 복사해라
	}

	return fileName;
}

shared_ptr<asAnimation> Converter::ReadAnimationData(aiAnimation* srcAnimation)
{
	shared_ptr<asAnimation> animation = make_shared<asAnimation>();
	animation->name = srcAnimation->mName.C_Str();
	animation->frameRate = (float)srcAnimation->mTicksPerSecond;
	animation->frameCount = (uint32)srcAnimation->mDuration + 1;

	map<string, shared_ptr<asAnimationNode>> cacheAnimNodes;

	for (uint32 i = 0; i < srcAnimation->mNumChannels; i++)
	{
		aiNodeAnim* srcNode = srcAnimation->mChannels[i];

		// 애니메이션 노드 데이터 파싱( SRT 추출 )
		shared_ptr<asAnimationNode> node = ParseAnimationNode(animation, srcNode);

		// 현재 찾은 노드 중에 제일 긴 시간으로 애니메이션 시간 갱신
		animation->duration = max(animation->duration, node->keyframe.back().time);

		cacheAnimNodes[srcNode->mNodeName.C_Str()] = node;
	}

	ReadKeyframeData(animation, _scene->mRootNode, cacheAnimNodes);

	return animation;
}

shared_ptr<asAnimationNode> Converter::ParseAnimationNode(shared_ptr<asAnimation> animation, aiNodeAnim* srcNode)
{
	std::shared_ptr<asAnimationNode> node = make_shared<asAnimationNode>();
	node->name = srcNode->mNodeName;

	uint32 keyCount = max(max(srcNode->mNumPositionKeys, srcNode->mNumScalingKeys), srcNode->mNumRotationKeys);

	for (uint32 k = 0; k < keyCount; k++)
	{
		asKeyframeData frameData;

		bool found = false;
		uint32 t = node->keyframe.size();

		// Position
		if (::fabsf((float)srcNode->mPositionKeys[k].mTime - (float)t) <= 0.0001f)
		{
			aiVectorKey key = srcNode->mPositionKeys[k];
			frameData.time = (float)key.mTime;
			::memcpy_s(&frameData.translation, sizeof(Vec3), &key.mValue, sizeof(aiVector3D));

			found = true;
		}

		// Rotation
		if (::fabsf((float)srcNode->mRotationKeys[k].mTime - (float)t) <= 0.0001f)
		{
			aiQuatKey key = srcNode->mRotationKeys[k];
			frameData.time = (float)key.mTime;

			frameData.rotation.x = key.mValue.x;
			frameData.rotation.y = key.mValue.y;
			frameData.rotation.z = key.mValue.z;
			frameData.rotation.w = key.mValue.w;

			found = true;
		}

		// Scale
		if (::fabsf((float)srcNode->mScalingKeys[k].mTime - (float)t) <= 0.0001f)
		{
			aiVectorKey key = srcNode->mScalingKeys[k];
			frameData.time = (float)key.mTime;
			::memcpy_s(&frameData.scale, sizeof(Vec3), &key.mValue, sizeof(aiVector3D));

			found = true;
		}

		if (found == true)
			node->keyframe.push_back(frameData);
	}

	// Keyframe 늘려주기
	if (node->keyframe.size() < animation->frameCount)
	{
		uint32 count = animation->frameCount - node->keyframe.size();
		asKeyframeData keyFrame = node->keyframe.back();

		for (uint32 n = 0; n < count; n++)
			node->keyframe.push_back(keyFrame);
	}

	return node;
}

void Converter::ReadKeyframeData(shared_ptr<asAnimation> animation, aiNode* srcNode, map<string, shared_ptr<asAnimationNode>>& cache)
{
	shared_ptr<asKeyframe> keyframe = make_shared<asKeyframe>();
	keyframe->boneName = srcNode->mName.C_Str();

	shared_ptr<asAnimationNode> findNode = cache[srcNode->mName.C_Str()];

	for (uint32 i = 0; i < animation->frameCount; i++)
	{
		asKeyframeData frameData;

		if (findNode == nullptr)
		{
			Matrix transform(srcNode->mTransformation[0]);
			transform = transform.Transpose();
			frameData.time = (float)i;
			transform.Decompose(OUT frameData.scale, OUT frameData.rotation, OUT frameData.translation);
		}
		else
		{
			frameData = findNode->keyframe[i];
		}

		keyframe->transforms.push_back(frameData);
	}

	// 애니메이션 키프레임 채우기
	animation->keyframes.push_back(keyframe);

	for (uint32 i = 0; i < srcNode->mNumChildren; i++)
		ReadKeyframeData(animation, srcNode->mChildren[i], cache);
}

void Converter::WriteAnimationData(shared_ptr<asAnimation> animation, wstring finalPath)
{
	auto path = filesystem::path(finalPath);

	// 폴더가 없으면 만든다.
	filesystem::create_directory(path.parent_path());

	shared_ptr<FileUtils> file = make_shared<FileUtils>();
	file->Open(finalPath, FileMode::Write);

	file->Write<string>(animation->name);
	file->Write<float>(animation->duration);
	file->Write<float>(animation->frameRate);
	file->Write<uint32>(animation->frameCount);

	file->Write<uint32>(animation->keyframes.size());

	for (shared_ptr<asKeyframe> keyframe : animation->keyframes)
	{
		file->Write<string>(keyframe->boneName);

		file->Write<uint32>(keyframe->transforms.size());
		file->Write(&keyframe->transforms[0], sizeof(asKeyframeData) * keyframe->transforms.size());
	}
}

uint32 Converter::GetBoneIndex(const string& name)
{
	for (shared_ptr<asBone>& bone : _bones)
	{
		if (bone->name == name)
			return bone->index;
	}
	assert(false);
	return 0;
}
