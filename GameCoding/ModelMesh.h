#pragma once
struct ModelBone
{
	wstring name;
	int32 index;
	int32 parentIndex;
	shared_ptr<ModelBone> parent; // Cache

	Matrix transform;
	vector<shared_ptr<ModelBone>> children; // Cache
};

struct ModelMesh
{
	void CreateBuffers();
	shared_ptr<Buffer> GetBuffer() { return buffer; }
	shared_ptr<Geometry<VertexTextureNormalTangentBlendData>> GetGeometry() { return geometry; }
	wstring name;

	// Mesh
	shared_ptr<Geometry<VertexTextureNormalTangentBlendData>> geometry = make_shared<Geometry<VertexTextureNormalTangentBlendData>>();
	shared_ptr<Buffer> buffer;

	// Material
	wstring materialName = L"";
	shared_ptr<Material> material; // Cache

	// Bones
	int32 boneIndex;
	shared_ptr<ModelBone> bone; // Cache;
};

