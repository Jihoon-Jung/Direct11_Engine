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
	void CreateSphere(); 
	void CreateRectangle();
	void CreateCube();
	void CreateCube_Normal();
	void CreateSphere_Normal();
	void CreateCube_NormalTangent();
	void CreateSphere_NormalTangent();

	shared_ptr<Buffer> GetBuffer() { return _buffer; }
	shared_ptr<Geometry<VertexTextureNormalTangentBlendData>> GetGeometry() { return _geometry; }
private:
	shared_ptr<Geometry<VertexTextureNormalTangentBlendData>> _geometry;
	shared_ptr<Buffer> _buffer;
};

