#pragma once

class Terrain : public Component
{
	using Super = Component;

public:
	Terrain();
	~Terrain();

	void GetGridMesh(shared_ptr<Mesh> grid);
	virtual void Start() override;
	int32 GetSizeX() { return _sizeX; }
	int32 GetSizeZ() { return _sizeZ; }

	bool Pick(Ray ray, float& distance, Vec3& hitPoint);

private:
	shared_ptr<Mesh> _mesh;
	int32 _sizeX = 0;
	int32 _sizeZ = 0;
	vector<VertexTerrain> _vertices;
};

struct ExtendedRay : public Ray
{
	ExtendedRay(const Vec3& pos, const Vec3& dir) : Ray(pos, dir) {}

	bool Intersects(const Vec3& v0, const Vec3& v1, const Vec3& v2, float& distance, float& u, float& v) const;
};