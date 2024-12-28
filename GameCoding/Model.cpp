#include "pch.h"
#include "Model.h"
#include "Utils.h"
#include "FileUtils.h"
#include "tinyxml2.h"
#include <filesystem>
#include "Material.h"

#include <windows.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <iomanip>

Model::Model()
	:Super(ResourceType::Mesh)
{

}

Model::~Model()
{

}

void Model::ReadMaterial(wstring filename)
{
	wstring fullPath = _texturePath + filename + L".xml";
	auto parentPath = filesystem::path(fullPath).parent_path();
	
	tinyxml2::XMLDocument* document = new tinyxml2::XMLDocument();
	tinyxml2::XMLError error = document->LoadFile(Utils::ToString(fullPath).c_str());
	assert(error == tinyxml2::XML_SUCCESS);

	tinyxml2::XMLElement* root = document->FirstChildElement();
	tinyxml2::XMLElement* materialNode = root->FirstChildElement();

	while (materialNode)
	{
		shared_ptr<Material> material = make_shared<Material>();

		tinyxml2::XMLElement* node = nullptr;

		node = materialNode->FirstChildElement();
		material->SetName(Utils::ToWString(node->GetText()));

		// Diffuse Texture
		node = node->NextSiblingElement();
		if (node->GetText())
		{
			wstring textureStr = Utils::ToWString(node->GetText());
			if (textureStr.length() > 0)
			{
				shared_ptr<Texture> diffuseTexture = make_shared<Texture>();
				diffuseTexture->CreateTexture((parentPath / textureStr).wstring());
				diffuseTexture->SetName(L"Tower_Diffuse");
				RESOURCE.AddResource(diffuseTexture->GetName(), diffuseTexture);
				/*auto texture = RESOURCES->GetOrAddTexture(textureStr, (parentPath / textureStr).wstring());*/
				material->SetDiffuseMap(diffuseTexture);
			}
		}

		// Specular Texture
		node = node->NextSiblingElement();
		if (node->GetText())
		{
			wstring texture = Utils::ToWString(node->GetText());
			if (texture.length() > 0)
			{
				wstring textureStr = Utils::ToWString(node->GetText());
				if (textureStr.length() > 0)
				{
					shared_ptr<Texture> specularTexture = make_shared<Texture>();
					specularTexture->CreateTexture((parentPath / textureStr).wstring());
					specularTexture->SetName(L"Tower_Specular");
					RESOURCE.AddResource(specularTexture->GetName(), specularTexture);
					//auto texture = RESOURCES->GetOrAddTexture(textureStr, (parentPath / textureStr).wstring());
					material->SetSpecularMap(specularTexture);
				}
			}
		}

		// Normal Texture
		node = node->NextSiblingElement();
		if (node->GetText())
		{
			wstring textureStr = Utils::ToWString(node->GetText());
			if (textureStr.length() > 0)
			{
				shared_ptr<Texture> normalTexture = make_shared<Texture>();
				normalTexture->CreateTexture((parentPath / textureStr).wstring());
				normalTexture->SetName(L"Tower_Normal");
				RESOURCE.AddResource(normalTexture->GetName(), normalTexture);
				material->SetNormalMap(normalTexture);
			}
		}

		MaterialDesc materialDesc;
		// Ambient
		{
			node = node->NextSiblingElement();

			Color color;
			color.x = node->FloatAttribute("R");
			color.y = node->FloatAttribute("G");
			color.z = node->FloatAttribute("B");
			color.w = node->FloatAttribute("A");
			materialDesc.ambient = color;
		}

		// Diffuse
		{
			node = node->NextSiblingElement();

			Color color;
			color.x = node->FloatAttribute("R");
			color.y = node->FloatAttribute("G");
			color.z = node->FloatAttribute("B");
			color.w = node->FloatAttribute("A");
			materialDesc.diffuse = color;
		}

		// Specular
		{
			node = node->NextSiblingElement();

			Color color;
			color.x = node->FloatAttribute("R");
			color.y = node->FloatAttribute("G");
			color.z = node->FloatAttribute("B");
			color.w = node->FloatAttribute("A");
			materialDesc.specular = color;
		}

		// Emissive
		{
			node = node->NextSiblingElement();

			Color color;
			color.x = node->FloatAttribute("R");
			color.y = node->FloatAttribute("G");
			color.z = node->FloatAttribute("B");
			color.w = node->FloatAttribute("A");
			materialDesc.emissive = color;
		}

		material->SetShader(_shader);
		material->SetMaterialDesc(materialDesc);
		_materials.push_back(material);

		
		// Next Material
		materialNode = materialNode->NextSiblingElement();
	}

	BindCacheInfo();
}
float Dot(const Vec3& a, const Vec3& b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
void Model::CalculateTangents(vector<VertexTextureNormalTangentBlendData>& vertices, const vector<unsigned int>& indices)
{
	// 탄젠트 초기화
	for (auto& vertex : vertices)
	{
		vertex.tangent = Vec3::Zero;
	}

	// 삼각형 단위로 탄젠트 계산
	for (size_t i = 0; i < indices.size(); i += 3)
	{
		VertexTextureNormalTangentBlendData& v0 = vertices[indices[i]];
		VertexTextureNormalTangentBlendData& v1 = vertices[indices[i + 1]];
		VertexTextureNormalTangentBlendData& v2 = vertices[indices[i + 2]];

		Vec3 deltaPos1 = v1.position - v0.position;
		Vec3 deltaPos2 = v2.position - v0.position;

		Vec2 deltaUV1 = v1.uv - v0.uv;
		Vec2 deltaUV2 = v2.uv - v0.uv;

		float r = 1.0f;
		float denominator = (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
		if (denominator != 0.0f)
		{
			r = 1.0f / denominator;
		}

		Vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;

		v0.tangent += tangent;
		v1.tangent += tangent;
		v2.tangent += tangent;
	}

	// 탄젠트 벡터 정규화 및 직교화
	for (auto& vertex : vertices)
	{
		
		// 노멀과 직교화
		vertex.tangent -= vertex.normal * Dot(vertex.normal, vertex.tangent);
		vertex.tangent.Normalize();
	}
}
void Model::ReadModel(wstring filename)
{
	wstring fullPath = _modelPath + filename + L".mesh";

	shared_ptr<FileUtils> file = make_shared<FileUtils>();
	file->Open(fullPath, FileMode::Read);

	// Bones
	{
		const uint32 count = file->Read<uint32>();

		for (uint32 i = 0; i < count; i++)
		{
			shared_ptr<ModelBone> bone = make_shared<ModelBone>();
			bone->index = file->Read<int32>();
			bone->name = Utils::ToWString(file->Read<string>());
			bone->parentIndex = file->Read<int32>();
			bone->transform = file->Read<Matrix>();

			_bones.push_back(bone);
		}
	}

	// Mesh
	{
		const uint32 count = file->Read<uint32>();

		for (uint32 i = 0; i < count; i++)
		{
			shared_ptr<ModelMesh> mesh = make_shared<ModelMesh>();

			mesh->name = Utils::ToWString(file->Read<string>());
			mesh->boneIndex = file->Read<int32>();

			// Material
			mesh->materialName = Utils::ToWString(file->Read<string>());

			//VertexData
			{
				const uint32 count = file->Read<uint32>();
				vector<VertexTextureNormalTangentBlendData> vertices;
				vertices.resize(count);

				void* data = vertices.data();
				file->Read(&data, sizeof(VertexTextureNormalTangentBlendData) * count);
				//mesh->geometry->AddVertices(vertices);
			

			//IndexData
			
				const uint32 indices_count = file->Read<uint32>();

				vector<uint32> indices;
				indices.resize(indices_count);

				void* indices_data = indices.data();
				file->Read(&indices_data, sizeof(uint32) * indices_count);
				mesh->geometry->AddIndices(indices);

				// test create tangnet
				CalculateTangents(vertices, indices);
				mesh->geometry->AddVertices(vertices);
			}
			
			mesh->CreateBuffers();
			vector<VertexTextureNormalTangentBlendData> v = mesh->geometry->GetVertices();
			vector<uint32> u = mesh->geometry->GetIndices();
			_meshes.push_back(mesh);
		}
	}

	BindCacheInfo();
}

void Model::ReadAnimation(wstring filename)
{
	wstring fullPath = _modelPath + filename + L".clip";

	shared_ptr<FileUtils> file = make_shared<FileUtils>();
	file->Open(fullPath, FileMode::Read);

	shared_ptr<ModelAnimation> animation = make_shared<ModelAnimation>();
	
	filesystem::path path(filename);
	animation->clipName = path.filename().wstring();
	animation->name = Utils::ToWString(file->Read<string>());
	animation->duration = file->Read<float>();
	animation->frameRate = file->Read<float>();
	animation->frameCount = file->Read<uint32>();

	uint32 keyframesCount = file->Read<uint32>();

	for (uint32 i = 0; i < keyframesCount; i++)
	{
		shared_ptr<ModelKeyframe> keyframe = make_shared<ModelKeyframe>();
		keyframe->boneName = Utils::ToWString(file->Read<string>());

		uint32 size = file->Read<uint32>();

		if (size > 0)
		{
			keyframe->transforms.resize(size);
			void* ptr = &keyframe->transforms[0];
			file->Read(&ptr, sizeof(ModelKeyframeData) * size);
		}
		animation->keyframes[keyframe->boneName] = keyframe;
	}

	_animations.push_back(animation);
	SetIsAnimatedModel(true);
}


std::shared_ptr<Material> Model::GetMaterialByName(const wstring& name)
{
	for (auto& material : _materials)
	{
		if (material->GetName() == name)
			return material;
	}

	return nullptr;
}

std::shared_ptr<ModelMesh> Model::GetMeshByName(const wstring& name)
{
	for (auto& mesh : _meshes)
	{
		if (mesh->name == name)
			return mesh;
	}

	return nullptr;
}

std::shared_ptr<ModelBone> Model::GetBoneByName(const wstring& name)
{
	for (auto& bone : _bones)
	{
		if (bone->name == name)
			return bone;
	}

	return nullptr;
}

shared_ptr<ModelAnimation> Model::GetAnimationByName(wstring name)
{
	for (auto& animation : _animations)
	{
		if (animation->name == name)
			return animation;
	}

	return nullptr;
}

void Model::CreateAnimationTransform(uint32 index)
{
	vector<Matrix> tempAnimBoneTransforms(MAX_MODEL_TRANSFORMS, Matrix::Identity);

	shared_ptr<ModelAnimation> animation = GetAnimationByIndex(index);
	int frameCount = animation->frameCount;
	int bonCount = GetBoneCount();

	for (uint32 f = 0; f < animation->frameCount; f++)
	{
		for (uint32 b = 0; b < GetBoneCount(); b++) // 모델mesh의 관절 갯수
		{
			shared_ptr<ModelBone> bone = GetBoneByIndex(b);
			// local에서의 애니메이션 변환 행렬
			Matrix matAnimation;

			shared_ptr<ModelKeyframe> frame = animation->GetKeyframe(bone->name);
			if (frame != nullptr)
			{
				ModelKeyframeData& data = frame->transforms[f];

				Matrix S, R, T;
				S = Matrix::CreateScale(data.scale.x, data.scale.y, data.scale.z);
				R = Matrix::CreateFromQuaternion(data.rotation);
				T = Matrix::CreateTranslation(data.translation.x, data.translation.y, data.translation.z);

				matAnimation = S * R * T;
			}
			else
			{
				matAnimation = Matrix::Identity;
			}


			// [ !!!!!!! ]
			// global로 가는 변환이 bone->transform 즉 T포즈 상태에서의 global 변환
			Matrix toRootMatrix = bone->transform;
			// T포즈에서 부모?(관절)을 기준으로한 상대 좌표로 변화시키는거
			Matrix invGlobal = toRootMatrix.Invert(); // T포즈 상태에서의 local로 가는 변환

			int32 parentIndex = bone->parentIndex;

			Matrix matParent = Matrix::Identity;
			if (parentIndex >= 0)
				matParent = tempAnimBoneTransforms[parentIndex];

			//부모?(관절)을 기준으로한좌표에서 global좌표로 변화(애니메이션이 적용되고나서 global로 변환)
			tempAnimBoneTransforms[b] = matAnimation *matParent;

			// 결론 
			// T포즈에서 Global좌표에 있던 정점을 invGlobal로 상대 좌표로 변화시키고 
			// tempAnimBoneTransforms[b]로 애니메이션(Matrix)적용시키고 다시 Global로 변화
			// 음 좀더 설명을 하자면 shader에 들어가는 정점은 이미 Global(모델의 local) 좌표인데
			// 이좌표에 invGlobal을 해주면 local(관절기준 상대좌표)로 가고.
			// 거기에 tempAnimBoneTransforms을 하면 local(관절기준 상대좌표)에서 애니메이션 적용 + 다시 Global로 가도록함.

			_animTransforms[index].transforms[f][b] = invGlobal * tempAnimBoneTransforms[b];

		}
	}
}

void Model::CreateTexture()
{
	if (GetAnimationCount() == 0)
		return;

	_animTransforms.resize(GetAnimationCount()); // idle/attack/die 3개애니메이션이면 3개
	for (uint32 i = 0; i < GetAnimationCount(); i++)
		CreateAnimationTransform(i);

	// Creature Texture
	{
		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));

		desc.Width = MAX_MODEL_TRANSFORMS * 4; // 4칸씩 나눠서 넣으려고?..
		desc.Height = MAX_MODEL_KEYFRAMES;
		desc.ArraySize = GetAnimationCount(); // 쉐이더에서 텍스처를 배열로 받게하려고 씀
		// 16바이트(R32G32B32A32 -> 제일 큰 크기로 한듯, 
		//			근데Matrix1개는 float 16개짜리인 64byte이기 때문에 Matrix 1개를 4개로 쪼개서 넘겨줘야함.)
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.Usage = D3D11_USAGE_IMMUTABLE;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.MipLevels = 1;
		desc.SampleDesc.Count = 1;

		const uint32 dataSize = MAX_MODEL_TRANSFORMS * sizeof(Matrix);
		const uint32 pageSize = dataSize * MAX_MODEL_KEYFRAMES;
		void* mallocPtr = ::malloc(pageSize * GetAnimationCount());

		// 파편화된 데이터를 조립한다.
		for (uint32 c = 0; c < GetAnimationCount(); c++)
		{
			uint32 startOffset = c * pageSize;

			// reinterpret_cast -> 포인터의 덧샘은 실제 값을 더하는게 아니라 타입만큼 점프 하는거니까
			//					   1바이트 크기로 변환해서 덧샘을 해야 진짜 값이 더해지는거다
			BYTE* pageStartPtr = reinterpret_cast<BYTE*>(mallocPtr) + startOffset;

			for (uint32 f = 0; f < MAX_MODEL_KEYFRAMES; f++)
			{
				void* ptr = pageStartPtr + dataSize * f;
				::memcpy(ptr, _animTransforms[c].transforms[f].data(), dataSize);
			}
		}

		// 리소스 만들기
		vector<D3D11_SUBRESOURCE_DATA> subResources(GetAnimationCount());

		for (uint32 c = 0; c < GetAnimationCount(); c++)
		{
			void* ptr = (BYTE*)mallocPtr + c * pageSize;
			subResources[c].pSysMem = ptr;
			subResources[c].SysMemPitch = dataSize;
			subResources[c].SysMemSlicePitch = pageSize;
		}

		HRESULT hr = DEVICE->CreateTexture2D(&desc, subResources.data(), _texture.GetAddressOf());
		CHECK(hr);

		::free(mallocPtr);
	}

	// Create SRV
	{
		D3D11_SHADER_RESOURCE_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
		desc.Texture2DArray.MipLevels = 1;
		desc.Texture2DArray.ArraySize = GetAnimationCount();

		HRESULT hr = DEVICE->CreateShaderResourceView(_texture.Get(), &desc, _srv.GetAddressOf());
		CHECK(hr);
	}
}

void Model::BindCacheInfo()
{
	// Mesh에 Material 캐싱
	for (const auto& mesh : _meshes)
	{
		// 이미 찾았으면 스킵
		if (mesh->material != nullptr)
			continue;

		mesh->material = GetMaterialByName(mesh->materialName);
	}

	// Mesh에 Bone 캐싱
	for (const auto& mesh : _meshes)
	{
		// 이미 찾았으면 스킵
		if (mesh->bone != nullptr)
			continue;

		mesh->bone = GetBoneByIndex(mesh->boneIndex);
	}

	// Bone 계층 정보 채우기
	if (_root == nullptr && _bones.size() > 0)
	{
		_root = _bones[0];

		for (const auto& bone : _bones)
		{
			if (bone->parentIndex >= 0)
			{
				bone->parent = _bones[bone->parentIndex];
				bone->parent->children.push_back(bone);
			}
			else
			{
				bone->parent = nullptr;
			}
		}
	}
}