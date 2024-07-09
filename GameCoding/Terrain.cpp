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
	_vertices = _mesh->GetGeometry()->GetVertices();
}

bool Terrain::Pick(Ray ray, float& distance)
{
	//_mesh->GetGeometry()->GetVertices();

	for (int32 z = 0; z < _sizeZ; z++)
	{
		for (int32 x = 0; x < _sizeX; x++)
		{
			uint32 index[4];
			index[0] = (_sizeX + 1) * z + x;
			index[1] = (_sizeX + 1) * z + x + 1;
			index[2] = (_sizeX + 1) * (z + 1) + x;
			index[3] = (_sizeX + 1) * (z + 1) + x + 1;

			Vec3 p[4];
			for (int32 i = 0; i < 4; i++)
				p[i] = _vertices[index[i]].position;

			//  [2]
			//   |	\
			//  [0] - [1]
			if (ray.Intersects(p[0], p[1], p[2], OUT distance))
			{
				return true;
			}

			//  [2] - [3]
			//   	\  |
			//		  [1]
			if (ray.Intersects(p[3], p[1], p[2], OUT distance))
			{
				return true;
			}
		}
	}
	return false;
}
