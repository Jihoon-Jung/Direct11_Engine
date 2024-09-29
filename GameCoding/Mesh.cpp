#include "pch.h"
#include "Mesh.h"
#include "Texture.h"
#include "MathHelper.h"
#include <fstream>
#include <sstream>
#include <algorithm>
Mesh::Mesh()
	:Super(ResourceType::Mesh)
{
}

Mesh::~Mesh()
{
}

void Mesh::CreateQuad_NormalTangent()
{
	vector<VertexTextureNormalTangentBlendData> vtx;
	vtx.resize(4);

	vtx[0].position = Vec3(-1.0f, -1.0f, 0.f);
	vtx[0].uv = Vec2(0.f, 1.f);

	vtx[1].position = Vec3(-1.0f, 1.0f, 0.f);
	vtx[1].uv = Vec2(0.f, 0.f);

	vtx[2].position = Vec3(1.0f, -1.0f, 0.f);
	vtx[2].uv = Vec2(1.f, 1.f);

	vtx[3].position = Vec3(1.0f, 1.0f, 0.f);
	vtx[3].uv = Vec2(1.f, 0.f);

	vector<uint32> idx = { 0, 1, 2, 2, 1, 3 };

	_geometry = make_shared<Geometry<VertexTextureNormalTangentBlendData>>();
	_geometry->SetVertices(vtx);
	_geometry->SetIndices(idx);

	_buffer = make_shared<Buffer>();
	_buffer->CreateBuffer(BufferType::VERTEX_BUFFER, _geometry->GetVertices());
	_buffer->CreateBuffer(BufferType::INDEX_BUFFER, _geometry->GetIndices());
}
void Mesh::CreateGrid_NormalTangent(int32 sizeX, int32 sizeZ)
{
	_sizeofGridX = sizeX;
	_sizeofGridZ = sizeZ;

	vector<VertexTextureNormalTangentBlendData> vtx;

	for (int32 z = 0; z < sizeZ + 1; z++)
	{
		for (int32 x = 0; x < sizeX + 1; x++)
		{
			VertexTextureNormalTangentBlendData v;
			v.position = Vec3(static_cast<float>(x), 0, static_cast<float>(z));
			v.uv = Vec2(static_cast<float>(x), static_cast<float>(sizeZ - z));
			v.normal = Vec3(0.f, 1.f, 0.f);
			v.tangent = Vec3(1.f, 0.f, 0.f);

			vtx.push_back(v);
		}
	}


	vector<uint32> idx;

	for (int32 z = 0; z < sizeZ; z++)
	{
		for (int32 x = 0; x < sizeX; x++)
		{
			//  [0]
			//   |	\
			//  [2] - [1]
			idx.push_back((sizeX + 1) * (z + 1) + (x));
			idx.push_back((sizeX + 1) * (z)+(x + 1));
			idx.push_back((sizeX + 1) * (z)+(x));
			//  [1] - [2]
			//   	\  |
			//		  [0]
			idx.push_back((sizeX + 1) * (z)+(x + 1));
			idx.push_back((sizeX + 1) * (z + 1) + (x));
			idx.push_back((sizeX + 1) * (z + 1) + (x + 1));
		}
	}

	_geometry = make_shared<Geometry<VertexTextureNormalTangentBlendData>>();
	_geometry->SetVertices(vtx);
	_geometry->SetIndices(idx);

	_buffer = make_shared<Buffer>();
	_buffer->CreateBuffer(BufferType::VERTEX_BUFFER, _geometry->GetVertices());
	_buffer->CreateBuffer(BufferType::INDEX_BUFFER, _geometry->GetIndices());
}
void Mesh::CreateCube_NormalTangent()
{
	float w2 = 0.5f;
	float h2 = 0.5f;
	float d2 = 0.5f;

	vector<VertexTextureNormalTangentBlendData> vtx(24);

	// 앞면
	vtx[0] = VertexTextureNormalTangentBlendData(Vec3(-w2, -h2, -d2), Vec2(0.0f, 1.0f), Vec3(0.0f, 0.0f, -1.0f), Vec3(1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[1] = VertexTextureNormalTangentBlendData(Vec3(-w2, +h2, -d2), Vec2(0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f), Vec3(1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[2] = VertexTextureNormalTangentBlendData(Vec3(+w2, +h2, -d2), Vec2(1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f), Vec3(1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[3] = VertexTextureNormalTangentBlendData(Vec3(+w2, -h2, -d2), Vec2(1.0f, 1.0f), Vec3(0.0f, 0.0f, -1.0f), Vec3(1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	// 뒷면
	vtx[4] = VertexTextureNormalTangentBlendData(Vec3(-w2, -h2, +d2), Vec2(1.0f, 1.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[5] = VertexTextureNormalTangentBlendData(Vec3(+w2, -h2, +d2), Vec2(0.0f, 1.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[6] = VertexTextureNormalTangentBlendData(Vec3(+w2, +h2, +d2), Vec2(0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[7] = VertexTextureNormalTangentBlendData(Vec3(-w2, +h2, +d2), Vec2(1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	// 윗면
	vtx[8] = VertexTextureNormalTangentBlendData(Vec3(-w2, +h2, -d2), Vec2(0.0f, 1.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[9] = VertexTextureNormalTangentBlendData(Vec3(-w2, +h2, +d2), Vec2(0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[10] = VertexTextureNormalTangentBlendData(Vec3(+w2, +h2, +d2), Vec2(1.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[11] = VertexTextureNormalTangentBlendData(Vec3(+w2, +h2, -d2), Vec2(1.0f, 1.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	// 아랫면
	vtx[12] = VertexTextureNormalTangentBlendData(Vec3(-w2, -h2, -d2), Vec2(1.0f, 1.0f), Vec3(0.0f, -1.0f, 0.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[13] = VertexTextureNormalTangentBlendData(Vec3(+w2, -h2, -d2), Vec2(0.0f, 1.0f), Vec3(0.0f, -1.0f, 0.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[14] = VertexTextureNormalTangentBlendData(Vec3(+w2, -h2, +d2), Vec2(0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[15] = VertexTextureNormalTangentBlendData(Vec3(-w2, -h2, +d2), Vec2(1.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	// 왼쪽면
	vtx[16] = VertexTextureNormalTangentBlendData(Vec3(-w2, -h2, +d2), Vec2(0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[17] = VertexTextureNormalTangentBlendData(Vec3(-w2, +h2, +d2), Vec2(0.0f, 0.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[18] = VertexTextureNormalTangentBlendData(Vec3(-w2, +h2, -d2), Vec2(1.0f, 0.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[19] = VertexTextureNormalTangentBlendData(Vec3(-w2, -h2, -d2), Vec2(1.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f), Vec4(1.0f), Vec4(1.0f));
	// 오른쪽면
	vtx[20] = VertexTextureNormalTangentBlendData(Vec3(+w2, -h2, -d2), Vec2(0.0f, 1.0f), Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[21] = VertexTextureNormalTangentBlendData(Vec3(+w2, +h2, -d2), Vec2(0.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[22] = VertexTextureNormalTangentBlendData(Vec3(+w2, +h2, +d2), Vec2(1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[23] = VertexTextureNormalTangentBlendData(Vec3(+w2, -h2, +d2), Vec2(1.0f, 1.0f), Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec4(1.0f), Vec4(1.0f));


	vector<uint32> idx(36);

	// 앞면
	idx[0] = 0; idx[1] = 1; idx[2] = 2;
	idx[3] = 0; idx[4] = 2; idx[5] = 3;
	// 뒷면
	idx[6] = 4; idx[7] = 5; idx[8] = 6;
	idx[9] = 4; idx[10] = 6; idx[11] = 7;
	// 윗면
	idx[12] = 8; idx[13] = 9; idx[14] = 10;
	idx[15] = 8; idx[16] = 10; idx[17] = 11;
	// 아랫면
	idx[18] = 12; idx[19] = 13; idx[20] = 14;
	idx[21] = 12; idx[22] = 14; idx[23] = 15;
	// 왼쪽면
	idx[24] = 16; idx[25] = 17; idx[26] = 18;
	idx[27] = 16; idx[28] = 18; idx[29] = 19;
	// 오른쪽면
	idx[30] = 20; idx[31] = 21; idx[32] = 22;
	idx[33] = 20; idx[34] = 22; idx[35] = 23;

	_geometry = make_shared<Geometry<VertexTextureNormalTangentBlendData>>();
	_geometry->SetVertices(vtx);
	_geometry->SetIndices(idx);

	_buffer = make_shared<Buffer>();
	_buffer->CreateBuffer(BufferType::VERTEX_BUFFER, _geometry->GetVertices());
	_buffer->CreateBuffer(BufferType::INDEX_BUFFER, _geometry->GetIndices());

}
void Mesh::CreateSphere_NormalTangent()
{
	float radius = 0.5f; // 구의 반지름
	uint32 stackCount = 20; // 가로 분할
	uint32 sliceCount = 20; // 세로 분할

	vector<VertexTextureNormalTangentBlendData> vtx;

	VertexTextureNormalTangentBlendData v;

	// 북극
	v.position = Vec3(0.0f, radius, 0.0f);
	v.uv = Vec2(0.5f, 0.0f);
	v.normal = v.position;
	v.normal.Normalize();
	v.tangent = Vec3(1.0f, 0.0f, 0.0f);
	v.tangent.Normalize();
	vtx.push_back(v);

	float stackAngle = XM_PI / stackCount;
	float sliceAngle = XM_2PI / sliceCount;

	float deltaU = 1.f / static_cast<float>(sliceCount);
	float deltaV = 1.f / static_cast<float>(stackCount);

	// 고리마다 돌면서 정점을 계산한다 (북극/남극 단일점은 고리가 X)
	for (uint32 y = 1; y <= stackCount - 1; ++y)
	{
		float phi = y * stackAngle;

		// 고리에 위치한 정점
		for (uint32 x = 0; x <= sliceCount; ++x)
		{
			float theta = x * sliceAngle;

			v.position.x = radius * sinf(phi) * cosf(theta);
			v.position.y = radius * cosf(phi);
			v.position.z = radius * sinf(phi) * sinf(theta);

			v.uv = Vec2(deltaU * x, deltaV * y);

			v.normal = v.position;
			v.normal.Normalize();

			v.tangent.x = -radius * sinf(phi) * sinf(theta);
			v.tangent.y = 0.0f;
			v.tangent.z = radius * sinf(phi) * cosf(theta);
			v.tangent.Normalize();

			vtx.push_back(v);
		}
	}

	// 남극
	v.position = Vec3(0.0f, -radius, 0.0f);
	v.uv = Vec2(0.5f, 1.0f);
	v.normal = v.position;
	v.normal.Normalize();
	v.tangent = Vec3(1.0f, 0.0f, 0.0f);
	v.tangent.Normalize();
	vtx.push_back(v);


	vector<uint32> idx(36);

	// 북극 인덱스
	for (uint32 i = 0; i <= sliceCount; ++i)
	{
		//  [0]
		//   |  \
		//  [i+1]-[i+2]
		idx.push_back(0);
		idx.push_back(i + 2);
		idx.push_back(i + 1);
	}

	// 몸통 인덱스
	uint32 ringVertexCount = sliceCount + 1;
	for (uint32 y = 0; y < stackCount - 2; ++y)
	{
		for (uint32 x = 0; x < sliceCount; ++x)
		{
			//  [y, x]-[y, x+1]
			//  |		/
			//  [y+1, x]
			idx.push_back(1 + (y)*ringVertexCount + (x));
			idx.push_back(1 + (y)*ringVertexCount + (x + 1));
			idx.push_back(1 + (y + 1) * ringVertexCount + (x));
			//		 [y, x+1]
			//		 /	  |
			//  [y+1, x]-[y+1, x+1]
			idx.push_back(1 + (y + 1) * ringVertexCount + (x));
			idx.push_back(1 + (y)*ringVertexCount + (x + 1));
			idx.push_back(1 + (y + 1) * ringVertexCount + (x + 1));
		}
	}

	// 남극 인덱스
	uint32 bottomIndex = static_cast<uint32>(vtx.size()) - 1;
	uint32 lastRingStartIndex = bottomIndex - ringVertexCount;
	for (uint32 i = 0; i < sliceCount; ++i)
	{
		//  [last+i]-[last+i+1]
		//  |      /
		//  [bottom]
		idx.push_back(bottomIndex);
		idx.push_back(lastRingStartIndex + i);
		idx.push_back(lastRingStartIndex + i + 1);
	}

	_geometry = make_shared<Geometry<VertexTextureNormalTangentBlendData>>();
	_geometry->SetVertices(vtx);
	_geometry->SetIndices(idx);

	_buffer = make_shared<Buffer>();
	_buffer->CreateBuffer(BufferType::VERTEX_BUFFER, _geometry->GetVertices());
	_buffer->CreateBuffer(BufferType::INDEX_BUFFER, _geometry->GetIndices());
}

void Mesh::CreateTerrain(TerrainInfo info)
{
	_info = info;

	// Divide heightmap into patches such that each patch has CellsPerPatch.
	// heightmapHeight = 2049, CellsPerPatch = 64
	// 1패치당 64개의 셀을 사용하겠다 라는말인듯.
	_numPatchVertRows = ((_info.heightmapHeight - 1) / CellsPerPatch) + 1; // _numPatchVertRows = 33 
	_numPatchVertCols = ((_info.heightmapWidth - 1) / CellsPerPatch) + 1; // _numPatchVertCols = 33      => 33 x 33개의 패치를 사용하고 각 패치는 64개의 셀을 사용

	_numPatchVertices = _numPatchVertRows * _numPatchVertCols; // 33 * 33 = 1089
	_numPatchQuadFaces = (_numPatchVertRows - 1) * (_numPatchVertCols - 1); // 332 * 32 = 1024

	LoadHeightmap();
	Smooth();
	CalcAllPatchBoundsY();

	BuildQuadPatchVB();
	BuildHeightmapSRV();

	std::vector<std::wstring> layerFilenames;
	layerFilenames.push_back(_info.layerMapFilename0);
	layerFilenames.push_back(_info.layerMapFilename1);
	layerFilenames.push_back(_info.layerMapFilename2);
	layerFilenames.push_back(_info.layerMapFilename3);
	layerFilenames.push_back(_info.layerMapFilename4);

	shared_ptr<Texture> texture = make_shared<Texture>();
	_layerMapArraySRV = texture->CreateTexture2DArraySRV(layerFilenames);

	_blendMapSRV = texture->LoadTextureFromDDS(_info.blendMapFilename.c_str());

}

float Mesh::GetWidth() const
{
	// Total terrain width.
	return (_info.heightmapWidth - 1) * _info.cellSpacing; // cellSpacing = 0.5
}

float Mesh::GetDepth() const
{
	// Total terrain depth.
	return (_info.heightmapHeight - 1) * _info.cellSpacing;
}

float Mesh::GetHeight(float x, float z) const
{
	// Transform from terrain local space to "cell" space.
	float c = (x + 0.5f * GetWidth()) / _info.cellSpacing;
	float d = (z - 0.5f * GetDepth()) / -_info.cellSpacing;

	// Get the row and column we are in.
	int row = (int)floorf(d);
	int col = (int)floorf(c);

	// Grab the heights of the cell we are in.
	// A*--*B
	//  | /|
	//  |/ |
	// C*--*D
	float A = _heightmap[row * _info.heightmapWidth + col];
	float B = _heightmap[row * _info.heightmapWidth + col + 1];
	float C = _heightmap[(row + 1) * _info.heightmapWidth + col];
	float D = _heightmap[(row + 1) * _info.heightmapWidth + col + 1];

	// Where we are relative to the cell.
	float s = c - (float)col;
	float t = d - (float)row;

	// If upper triangle ABC.
	if (s + t <= 1.0f)
	{
		float uy = B - A;
		float vy = C - A;
		return A + s * uy + t * vy;
	}
	else // lower triangle DCB.
	{
		float uy = C - D;
		float vy = B - D;
		return D + (1.0f - s) * uy + (1.0f - t) * vy;
	}
}

void Mesh::LoadHeightmap()
{
	// A height for each vertex
	std::vector<unsigned char> in(_info.heightmapWidth * _info.heightmapHeight);

	// Open the file.
	std::ifstream inFile;
	inFile.open(_info.heightMapFilename.c_str(), std::ios_base::binary);

	if (inFile)
	{
		// Read the RAW bytes.
		inFile.read((char*)&in[0], (std::streamsize)in.size());

		// Done with file.
		inFile.close();
	}

	// Copy the array data into a float array and scale it.
	_heightmap.resize(_info.heightmapHeight * _info.heightmapWidth, 0);

	for (uint32 i = 0; i < _info.heightmapHeight * _info.heightmapWidth; ++i)
	{
		_heightmap[i] = (in[i] / 255.0f) * _info.heightScale;
	}
}

void Mesh::Smooth()
{
	std::vector<float> dest(_heightmap.size());

	for (uint32 i = 0; i < _info.heightmapHeight; ++i)
	{
		for (uint32 j = 0; j < _info.heightmapWidth; ++j)
		{
			dest[i * _info.heightmapWidth + j] = Average(i, j);
		}
	}

	// Replace the old heightmap with the filtered one.
	_heightmap = dest;
}

bool Mesh::InBounds(int32 i, int32 j)
{
	// True if ij are valid indices; false otherwise.
	return
		i >= 0 && i < (int32)_info.heightmapHeight&&
		j >= 0 && j < (int32)_info.heightmapWidth;
}

float Mesh::Average(int32 i, int32 j)
{
	// Function computes the average height of the ij element.
	// It averages itself with its eight neighbor pixels.  Note
	// that if a pixel is missing neighbor, we just don't include it
	// in the average--that is, edge pixels don't have a neighbor pixel.
	//
	// ----------
	// | 1| 2| 3|
	// ----------
	// |4 |ij| 6|
	// ----------
	// | 7| 8| 9|
	// ----------

	float avg = 0.0f;
	float num = 0.0f;

	// Use int to allow negatives.  If we use UINT, @ i=0, m=i-1=UINT_MAX
	// and no iterations of the outer for loop occur.
	for (int32 m = i - 1; m <= i + 1; ++m)
	{
		for (int32 n = j - 1; n <= j + 1; ++n)
		{
			if (InBounds(m, n))
			{
				avg += _heightmap[m * _info.heightmapWidth + n];
				num += 1.0f;
			}
		}
	}

	return avg / num;
}

void Mesh::CalcAllPatchBoundsY()
{
	_patchBoundsY.resize(_numPatchQuadFaces);

	// For each patch
	for (uint32 i = 0; i < _numPatchVertRows - 1; ++i)// _numPatchVertRows = 33
	{
		for (uint32 j = 0; j < _numPatchVertCols - 1; ++j)// _numPatchVertCols = 33
		{
			CalcPatchBoundsY(i, j);
		}
	}
}

void Mesh::CalcPatchBoundsY(uint32 i, uint32 j)
{
	// Scan the heightmap values this patch covers and compute the min/max height.

	uint32 x0 = j * CellsPerPatch; // CellsPerPatch = 64
	uint32 x1 = (j + 1) * CellsPerPatch;

	uint32 y0 = i * CellsPerPatch;
	uint32 y1 = (i + 1) * CellsPerPatch;

	float minY = +3.402823466e+38F;
	float maxY = -3.402823466e+38F;

	for (uint32 y = y0; y <= y1; ++y)
	{
		for (uint32 x = x0; x <= x1; ++x)
		{
			uint32 k = y * _info.heightmapWidth + x;
			minY = min(minY, _heightmap[k]);
			maxY = max(maxY, _heightmap[k]);
		}
	}

	uint32 patchID = i * (_numPatchVertCols - 1) + j; // _numPatchVertCols = 33 , 
	_patchBoundsY[patchID] = Vec2(minY, maxY);
}

void Mesh::BuildQuadPatchVB()
{
	vector<VertexTerrain> patchVertices(_numPatchVertRows * _numPatchVertCols);// _numPatchVertRows = 33, _numPatchVertCols = 33

	_sizeofGridX = _numPatchVertRows;
	_sizeofGridZ = _numPatchVertCols;

	float halfWidth = 0.5f * GetWidth(); // 512
	float halfDepth = 0.5f * GetDepth(); // 512
	// _numPatchVertCols = 33
	// _numPatchVertRows = 33
	float patchWidth = GetWidth() / (_numPatchVertCols - 1); // 32
	float patchDepth = GetDepth() / (_numPatchVertRows - 1); // 32
	float du = 1.0f / (_numPatchVertCols - 1); // 0.03125
	float dv = 1.0f / (_numPatchVertRows - 1); // 0.03125

	for (uint32 i = 0; i < _numPatchVertRows; ++i)
	{
		float z = halfDepth - i * patchDepth;
		for (uint32 j = 0; j < _numPatchVertCols; ++j)
		{
			float x = -halfWidth + j * patchWidth;

			patchVertices[i * _numPatchVertCols + j].position = XMFLOAT3(x, 0.0f, z);

			// Stretch texture over grid.
			patchVertices[i * _numPatchVertCols + j].uv.x = j * du;
			patchVertices[i * _numPatchVertCols + j].uv.y = i * dv;
		}
	}

	// Store axis-aligned bounding box y-bounds in upper-left patch corner.
	for (uint32 i = 0; i < _numPatchVertRows - 1; ++i)
	{
		for (uint32 j = 0; j < _numPatchVertCols - 1; ++j)
		{
			uint32 patchID = i * (_numPatchVertCols - 1) + j;
			patchVertices[i * _numPatchVertCols + j].BoundsY = _patchBoundsY[patchID];
		}
	}

	vector<uint32> idx = BuildQuadPatchIB();

	_geometryForTerrain = make_shared<Geometry<VertexTerrain>>();
	_geometryForTerrain->SetVertices(patchVertices);
	_geometryForTerrain->SetIndices(idx);

	_buffer = make_shared<Buffer>();
	_buffer->CreateBuffer(BufferType::VERTEX_BUFFER, _geometryForTerrain->GetVertices());
	_buffer->CreateBuffer(BufferType::INDEX_BUFFER, _geometryForTerrain->GetIndices());
}

vector<uint32> Mesh::BuildQuadPatchIB()
{
	vector<uint32> indices(_numPatchQuadFaces * 4); // 4 indices per quad face

	// Iterate over each quad and compute indices.
	int32 k = 0;
	for (uint32 i = 0; i < _numPatchVertRows - 1; ++i) // _numPatchVertRows = 33
	{
		for (uint32 j = 0; j < _numPatchVertCols - 1; ++j) // _numPatchVertCols = 33
		{
			// Top row of 2x2 quad patch
			indices[k] = i * _numPatchVertCols + j;
			indices[k + 1] = i * _numPatchVertCols + j + 1;

			// Bottom row of 2x2 quad patch
			indices[k + 2] = (i + 1) * _numPatchVertCols + j;
			indices[k + 3] = (i + 1) * _numPatchVertCols + j + 1;

			k += 4; // next quad
		}
	}

	// Return the computed indices vector.
	return indices;
}


void Mesh::BuildHeightmapSRV()
{
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = _info.heightmapWidth;
	texDesc.Height = _info.heightmapHeight;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R16_FLOAT;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	// HALF is defined in xnamath.h, for storing 16-bit float.
	vector<uint16> hmap(_heightmap.size());
	std::transform(_heightmap.begin(), _heightmap.end(), hmap.begin(), MathHelper::ConvertFloatToHalf);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = &hmap[0];
	data.SysMemPitch = _info.heightmapWidth * sizeof(uint16);
	data.SysMemSlicePitch = 0;

	ID3D11Texture2D* hmapTex = 0;
	HRESULT hr = DEVICE->CreateTexture2D(&texDesc, &data, &hmapTex);
	CHECK(hr);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = texDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;
	hr = DEVICE->CreateShaderResourceView(hmapTex, &srvDesc, _heightMapSRV.GetAddressOf());
	CHECK(hr);
}

void Mesh::CreateCylinder_NormalTangent()
{
	float bottomRadius = 0.5f;
	float topRadius = 0.5f;
	float height = 3.0f;
	UINT32 sliceCount = 15;
	UINT32 stackCount = 15;

	float stackHeight = height / stackCount;

	// Amount to increment radius as we move up each stack level from bottom to top.
	float radiusStep = (topRadius - bottomRadius) / stackCount;

	UINT ringCount = stackCount + 1;

	vector<VertexTextureNormalTangentBlendData> vtx;

	// Compute vertices for each stack ring starting at the bottom and moving up.
	for (uint32 i = 0; i < ringCount; ++i)
	{
		float y = -0.5f * height + i * stackHeight;
		float r = bottomRadius + i * radiusStep;

		// vertices of ring
		float dTheta = 2.0f * XM_PI / sliceCount;
		for (uint32 j = 0; j <= sliceCount; ++j)
		{
			VertexTextureNormalTangentBlendData vertex;

			float c = cosf(j * dTheta);
			float s = sinf(j * dTheta);

			vertex.position = XMFLOAT3(r * c, y, r * s);

			vertex.uv.x = (float)j / sliceCount;
			vertex.uv.y = 1.0f - (float)i / stackCount;


			// This is unit length.
			vertex.tangent = XMFLOAT3(-s, 0.0f, c);

			float dr = bottomRadius - topRadius;
			XMFLOAT3 bitangent(dr * c, -height, dr * s);

			XMVECTOR T = ::XMLoadFloat3(&vertex.tangent);
			XMVECTOR B = ::XMLoadFloat3(&bitangent);
			XMVECTOR N = ::XMVector3Normalize(::XMVector3Cross(T, B));
			::XMStoreFloat3(&vertex.normal, N);

			vtx.push_back(vertex);
		}
	}

	vector<uint32> idx;
	// Add one because we duplicate the first and last vertex per ring
	// since the texture coordinates are different.
	UINT ringVertexCount = sliceCount + 1;

	// Compute indices for each stack.
	for (UINT i = 0; i < stackCount; ++i)
	{
		for (UINT j = 0; j < sliceCount; ++j)
		{
			idx.push_back(i * ringVertexCount + j);
			idx.push_back((i + 1) * ringVertexCount + j);
			idx.push_back((i + 1) * ringVertexCount + j + 1);

			idx.push_back(i * ringVertexCount + j);
			idx.push_back((i + 1) * ringVertexCount + j + 1);
			idx.push_back(i * ringVertexCount + j + 1);
		}
	}

	BuildCylinderTopCap(bottomRadius, topRadius, height, sliceCount, stackCount, vtx, idx);
	BuildCylinderBottomCap(bottomRadius, topRadius, height, sliceCount, stackCount, vtx, idx);

	

	_geometry = make_shared<Geometry<VertexTextureNormalTangentBlendData>>();
	_geometry->SetVertices(vtx);
	_geometry->SetIndices(idx);

	_buffer = make_shared<Buffer>();
	_buffer->CreateBuffer(BufferType::VERTEX_BUFFER, _geometry->GetVertices());
	_buffer->CreateBuffer(BufferType::INDEX_BUFFER, _geometry->GetIndices());
}

void Mesh::BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount, 
								vector<VertexTextureNormalTangentBlendData>& vtx, vector<uint32>& idx)
{
	uint32 baseIndex = (uint32)vtx.size();

	float y = 0.5f * height;
	float dTheta = 2.0f * XM_PI / sliceCount;

	// Duplicate cap ring vertices because the texture coordinates and normals differ.
	for (uint32 i = 0; i <= sliceCount; ++i)
	{
		float x = topRadius * cosf(i * dTheta);
		float z = topRadius * sinf(i * dTheta);

		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;
		VertexTextureNormalTangentBlendData vertex;
		vertex.position = Vec3(x, y, z);
		vertex.normal = Vec3(0.0f, 1.0f, 0.0f);
		vertex.tangent = Vec3(1.0f, 0.0f, 0.0f);
		vertex.uv = Vec2(u, v);
		vtx.push_back(vertex);
	}

	// Cap center vertex.
	VertexTextureNormalTangentBlendData vertex;
	vertex.position = Vec3(0.0f, y, 0.0f);
	vertex.normal = Vec3(0.0f, 1.0f, 0.0f);
	vertex.tangent = Vec3(1.0f, 0.0f, 0.0f);
	vertex.uv = Vec2(0.5f, 0.5f);
	vtx.push_back(vertex);

	// Index of center vertex.
	uint32 centerIndex = (uint32)vtx.size() - 1;

	for (uint32 i = 0; i < sliceCount; ++i)
	{
		idx.push_back(centerIndex);
		idx.push_back(baseIndex + i + 1);
		idx.push_back(baseIndex + i);
	}
}

void Mesh::BuildCylinderBottomCap(float bottomRadius, float topRadius, float height,
									uint32 sliceCount, uint32 stackCount, vector<VertexTextureNormalTangentBlendData>& vtx, vector<uint32>& idx)
{
	// 
	// Build bottom cap.
	//

	uint32 baseIndex = (uint32)vtx.size();
	float y = -0.5f * height;

	// vertices of ring
	float dTheta = 2.0f * XM_PI / sliceCount;
	for (uint32 i = 0; i <= sliceCount; ++i)
	{
		float x = bottomRadius * cosf(i * dTheta);
		float z = bottomRadius * sinf(i * dTheta);

		// Scale down by the height to try and make top cap texture coord area
		// proportional to base.
		float u = x / height + 0.5f;
		float v = z / height + 0.5f;

		VertexTextureNormalTangentBlendData vertex;
		vertex.position = Vec3(x, y, z);
		vertex.normal = Vec3(0.0f, -1.0f, 0.0f);
		vertex.tangent = Vec3(1.0f, 0.0f, 0.0f);
		vertex.uv = Vec2(u, v);

		vtx.push_back(vertex);
	}

	// Cap center vertex.
	VertexTextureNormalTangentBlendData vertex;
	vertex.position = Vec3(0.0f, y, 0.0f);
	vertex.normal = Vec3(0.0f, -1.0f, 0.0f);
	vertex.tangent = Vec3(1.0f, 0.0f, 0.0f);
	vertex.uv = Vec2(0.5f, 0.5f);
	vtx.push_back(vertex);

	// Cache the index of center vertex.
	uint32 centerIndex = (uint32)vtx.size() - 1;

	for (uint32 i = 0; i < sliceCount; ++i)
	{
		idx.push_back(centerIndex);
		idx.push_back(baseIndex + i);
		idx.push_back(baseIndex + i + 1);
	}
}
