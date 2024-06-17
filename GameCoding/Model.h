#pragma once
#include "ModelMesh.h"
#include "ModelAnimation.h"

class Model : public ResourceBase, public std::enable_shared_from_this<Model>
{
	using Super = ResourceBase;
public:
	Model();
	~Model();
	
public:
	void ReadMaterial(wstring filename);
	void ReadModel(wstring filename);
	void ReadAnimation(wstring filename);


	void SetVertexShaderForMaterial(shared_ptr<Shader> vertexShader) { _vertexShader = vertexShader; }
	void SetPixelShaderForMaterial(shared_ptr<Shader> pixelShader) { _pixelShader = pixelShader; }
	uint32 GetMaterialCount() { return static_cast<uint32>(_materials.size()); }
	vector<shared_ptr<Material>>& GetMaterials() { return _materials; }
	shared_ptr<Material> GetMaterialByIndex(uint32 index) { return _materials[index]; }
	shared_ptr<Material> GetMaterialByName(const wstring& name);

	uint32 GetMeshCount() { return static_cast<uint32>(_meshes.size()); }
	vector<shared_ptr<ModelMesh>>& GetMeshes() { return _meshes; }
	shared_ptr<ModelMesh> GetMeshByIndex(uint32 index) { return _meshes[index]; }
	shared_ptr<ModelMesh> GetMeshByName(const wstring& name);

	uint32 GetBoneCount() { return static_cast<uint32>(_bones.size()); }
	vector<shared_ptr<ModelBone>>& GetBones() { return _bones; }
	shared_ptr<ModelBone> GetBoneByIndex(uint32 index) { return (index < 0 || index >= _bones.size() ? nullptr : _bones[index]); }
	shared_ptr<ModelBone> GetBoneByName(const wstring& name);

	uint32 GetAnimationCount() { return _animations.size(); }
	vector<shared_ptr<ModelAnimation>>& GetAnimations() { return _animations; }
	shared_ptr<ModelAnimation> GetAnimationByIndex(UINT index) { return (index < 0 || index >= _animations.size()) ? nullptr : _animations[index]; }
	shared_ptr<ModelAnimation> GetAnimationByName(wstring name);
	ComPtr<ID3D11ShaderResourceView> GetAnimationTextureBuffer() { return _srv; }
	vector<AnimTransform>& GetAnimTransforms() { return _animTransforms; };
	void CreateAnimationTransform(uint32 index);
	void CreateTexture();
	void SetIsAnimatedModel(bool value) { isAnimatedModel = value; }
	bool HasAnimation() { return isAnimatedModel; }


private:
	void BindCacheInfo();

private:
	shared_ptr<Shader> _vertexShader;
	shared_ptr<Shader> _pixelShader;
private:
	wstring _modelPath = L"../Resources/Models/";
	wstring _texturePath = L"../Resources/Textures/";

private:

	bool isAnimatedModel = false;
	shared_ptr<ModelBone> _root;
	vector<shared_ptr<Material>> _materials;
	vector<shared_ptr<ModelBone>> _bones;
	vector<shared_ptr<ModelMesh>> _meshes;

	vector<shared_ptr<ModelAnimation>> _animations;
	vector<AnimTransform> _animTransforms;
	ComPtr<ID3D11Texture2D> _texture;
	ComPtr<ID3D11ShaderResourceView> _srv;

};

