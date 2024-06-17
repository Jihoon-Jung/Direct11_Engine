#pragma once

#include "pch.h"
#include "ResourceBase.h"
#include "Mesh.h"
#include "Shader.h"
#include "Material.h"
#include "Texture.h"
#include "Model.h"

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
private:
	using ResourecMap = map<wstring, shared_ptr<ResourceBase>>;
	ResourecMap _resources[RESOURCE_TYPE_COUNT];

};

