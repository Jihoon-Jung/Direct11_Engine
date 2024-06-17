#pragma once
enum class ResourceType
{
	None,
	Mesh,
	Model,
	Shader,
	Texture,
	Material,
	Animation
};
enum
{
	RESOURCE_TYPE_COUNT = 6
};
class ResourceBase
{
public:
	ResourceBase(ResourceType type);
	virtual ~ResourceBase();

	ResourceType GetType() { return _type; }

	void SetName(const wstring& name) { _name = name; }
	const wstring& GetName() { return _name; }

protected:
	virtual void Load(const wstring& path) { }
	virtual void Save(const wstring& path) { }

protected:
	ResourceType _type = ResourceType::None;
	wstring _name;
	wstring _path;
};

