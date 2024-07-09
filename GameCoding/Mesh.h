#pragma once
#include "Buffer.h"
#include "VertexDescription.h"
#include "Geometry.h"
#include "ResourceBase.h"

class Mesh : public ResourceBase
{
	using Super = ResourceBase;
public:
	Mesh();
	~Mesh();
	void CreateQuard_NormalTangent();
	void CreateGrid_NormalTangent(int32 sizeX, int32 sizeZ);
	void CreateCube_NormalTangent();
	void CreateSphere_NormalTangent();
	int32 GetGridSizeX() { return _sizeofGridX; }
	int32 GetGridSizeZ() { return _sizeofGridZ; }
	shared_ptr<Buffer> GetBuffer() { return _buffer; }
	shared_ptr<Geometry<VertexTextureNormalTangentBlendData>> GetGeometry() { return _geometry; }
private:
	shared_ptr<Geometry<VertexTextureNormalTangentBlendData>> _geometry;
	shared_ptr<Buffer> _buffer;
	int32 _sizeofGridX = 0;
	int32 _sizeofGridZ = 0;
};

