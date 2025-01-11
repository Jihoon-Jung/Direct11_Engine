#pragma once

#include "pch.h"
#include "ResourceBase.h"
#include "Mesh.h"
#include "Shader.h"
#include "Material.h"
#include "Texture.h"
#include "Model.h"
#include <filesystem>
#include <fstream>

class ResourceManager
{
protected:
	ResourceManager() {}
	virtual ~ResourceManager() {}
public:
	ResourceManager(const ResourceManager&) = delete;

	static ResourceManager& GetInstance()
	{
		static ResourceManager instance;
		return instance;
	}
	
	void Init();
	void AddResource();
	void SaveResource();
	void WriteMeshToXML(const wstring& meshName, const wstring& meshType, const wstring& finalPath);
	void WriteTextureToXML(const wstring& imagePath, const wstring& textureNamae, const wstring& finalPath);
	void WriteShaderToXML(const wstring& shaderPath, const wstring& shaderName,
		const vector<ShaderType>& shaderTypes, const vector<InputLayoutType>& inputLayouts,
		const map<wstring, uint32>& slots, const wstring& finalPath);
	void WriteMaterialToXML(const wstring& textureName, const wstring& normalMapName,
		const MaterialDesc& matDesc, const wstring& shaderName, const wstring& materialName, const wstring& finalPath);
	void WriteModelToXML(const wstring& modelPath, const wstring& shaderName,
		const wstring& materialPath, const wstring& modelName, const vector<wstring>& animPaths, const wstring& finalPath);

	void LoadMeshData(wstring path);
	void LoadTextureData(wstring path);
	void LoadShaderData(wstring path);
	void LoadMaterialData(const wstring& path);
	void LoadModelData(const wstring& path);
	void LoadResourcesByType(const filesystem::path& folderPath,
		const string& extension, function<void(const wstring&)> loadFunc);

	void WriteScriptToXML(const string& className, const string& displayName, const wstring& finalPath);
	void LoadScriptData(const wstring& path);
	void SaveScripts();  // ComponentFactory에 등록된 모든 스크립트를 XML로 저장

	template<typename T>
	ResourceType GetResourceType()
	{
		if (std::is_same_v<T, Mesh>)
			return ResourceType::Mesh;
		if (std::is_same_v<T, Model>)
			return ResourceType::Model;
		if (std::is_same_v<T, Shader>)
			return ResourceType::Shader;
		if (std::is_same_v<T, Texture>)
			return ResourceType::Texture;
		if (std::is_same_v<T, Material>)
			return ResourceType::Material;
		//if (std::is_same_v<T, Animation>)
		//	return ResourceType::Animation;

		assert(false);
		return ResourceType::None;
	}
	void CreateResource();

	template <typename T>
	void AddResource(const wstring& key, shared_ptr<T> value)
	{
		ResourceType type = GetResourceType<T>();
		ResourecMap& map = _resources[static_cast<uint8>(type)];

		auto it = map.find(key);
		if (it != map.end())
		{
			printf("that resource is already in storage");
			return;
		}

		map[key] = value;
	}
	
	template <typename T>
	shared_ptr<T> GetResource(const wstring& key)
	{
		ResourceType type = GetResourceType<T>();
		ResourecMap& map = _resources[static_cast<uint8>(type)];

		auto it = map.find(key);
		shared_ptr<T> result;
		if (it != map.end())
		{
			result = static_pointer_cast<T>(it->second);
		}
		return result;// static_pointer_cast<T>(it->second);
	}

	template <typename T>
	wstring GetResourceName(const shared_ptr<T>& resource)
	{
		ResourceType type = GetResourceType<T>();
		ResourecMap& map = _resources[static_cast<uint8>(type)];

		for (const auto& pair : map)
		{
			if (pair.second == resource)
			{
				return pair.first;
			}
		}

		return L"";  // 리소스를 찾지 못한 경우
	}
private:
	using ResourecMap = map<wstring, shared_ptr<ResourceBase>>;
	ResourecMap _resources[RESOURCE_TYPE_COUNT];

	string LoadScriptContent(const string& className, bool isHeader);  // 헤더파일도 로드하도록 수정
};

