#include "pch.h"
#include "Terrain.h"

Terrain::Terrain()
	: Super(ComponentType::Terrain)
{
	
}

Terrain::~Terrain()
{
}

void Terrain::GetGridMesh(shared_ptr<Mesh> grid)
{
	_sizeX = grid->GetGridSizeX();
	_sizeZ = grid->GetGridSizeZ();
	
	_mesh = grid;
	_vertices = _mesh->GetTerrainGeometry()->GetVertices();

	Vec3 localPosition = GetGameObject()->transform()->GetLocalPosition();
	for (int i = 0; i < _vertices.size(); i++)
	{
        _vertices[i].position;// += localPosition;
        _vertices[i].position.y = _vertices[i].BoundsY.y;
	}
}

void Terrain::Start()
{
	
}

bool Terrain::Pick(Ray ray, float& distance, Vec3& hitPoint)
{
    ExtendedRay extendedRay(ray.position, ray.direction);

    for (int32 z = 0; z < _sizeZ; ++z)
    {
        for (int32 x = 0; x < _sizeX; ++x)
        {
            uint32 index[4];
            index[0] = (_sizeX + 1) * z + x;
            index[1] = (_sizeX + 1) * z + x + 1;
            index[2] = (_sizeX + 1) * (z + 1) + x;
            index[3] = (_sizeX + 1) * (z + 1) + x + 1;

            Vec3 p[4];
            for (int32 i = 0; i < 4; ++i)
                p[i] = _vertices[index[i]].position;

            float u, v;
            if (extendedRay.Intersects(p[0], p[1], p[2], distance, u, v))
            {
                hitPoint = p[0] * (1 - u - v) + p[1] * u + p[2] * v;
                return true;
            }

            if (extendedRay.Intersects(p[3], p[1], p[2], distance, u, v))
            {
                hitPoint = p[3] * (1 - u - v) + p[1] * u + p[2] * v;
                return true;
            }
        }
    }
    return false;
}
//Moller-Trumbore 알고리즘을 사용하여 더 정확한 Ray와 물체 충돌 위치 계산
bool ExtendedRay::Intersects(const Vec3& v0, const Vec3& v1, const Vec3& v2, float& distance, float& u, float& v) const
{
    const float EPSILON = 0.0000001f;
    Vec3 edge1 = v1 - v0;
    Vec3 edge2 = v2 - v0;
    Vec3 h = direction.Cross(edge2);
    float a = edge1.Dot(h);
    if (a > -EPSILON && a < EPSILON)
        return false; // This ray is parallel to this triangle.

    float f = 1.0f / a;
    Vec3 s = position - v0;
    u = f * s.Dot(h);
    if (u < 0.0f || u > 1.0f)
        return false;

    Vec3 q = s.Cross(edge1);
    v = f * direction.Dot(q);
    if (v < 0.0f || u + v > 1.0f)
        return false;

    distance = f * edge2.Dot(q);
    return distance > EPSILON; // ray intersection
}