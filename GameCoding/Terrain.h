#pragma once

class Terrain : public Component
{
	using Super = Component;

public:
	Terrain();
	~Terrain();

	void GetGridMesh(shared_ptr<Mesh> grid);

	int32 GetSizeX() { return _sizeX; }
	int32 GetSizeZ() { return _sizeZ; }

	bool Pick(Ray ray, float& distance);

private:
	shared_ptr<Mesh> _mesh;
	int32 _sizeX = 0;
	int32 _sizeZ = 0;
	vector<VertexTextureNormalTangentBlendData> _vertices;
};

