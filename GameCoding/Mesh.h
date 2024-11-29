#pragma once
#include "Buffer.h"
#include "VertexDescription.h"
#include "Geometry.h"
#include "ResourceBase.h"

struct TerrainInfo
{
	wstring heightMapFilename;
	wstring layerMapFilename0;
	wstring layerMapFilename1;
	wstring layerMapFilename2;
	wstring layerMapFilename3;
	wstring layerMapFilename4;
	wstring blendMapFilename;
	float heightScale;
	uint32 heightmapWidth;
	uint32 heightmapHeight;
	float cellSpacing;
};

class Mesh : public ResourceBase
{
	using Super = ResourceBase;
public:
	Mesh();
	~Mesh();
	void CreateQuad_NormalTangent();
	void CreateGrid_NormalTangent(int32 sizeX, int32 sizeZ);
	void CreateCube_NormalTangent();
	void CreateSphere_NormalTangent();
	void CreateCylinder_NormalTangent();
	void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, 
		uint32 sliceCount, uint32 stackCount, vector<VertexTextureNormalTangentBlendData>& vtx, vector<uint32>& idx);
	void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height,
		uint32 sliceCount, uint32 stackCount, vector<VertexTextureNormalTangentBlendData>& vtx, vector<uint32>& idx);

	int32 GetGridSizeX() { return _sizeofGridX; }
	int32 GetGridSizeZ() { return _sizeofGridZ; }
	shared_ptr<Buffer> GetBuffer() { return _buffer; }
	shared_ptr<Geometry<VertexTextureNormalTangentBlendData>> GetGeometry() { return _geometry; }
	shared_ptr <Geometry<VertexTerrain>> GetTerrainGeometry() { return _geometryForTerrain; }
	// for terrain
	void CreateTerrain();
	float GetWidth() const;
	float GetDepth() const;
	float GetHeight(float x, float z) const;
	void LoadHeightmap();
	void Smooth();
	bool InBounds(int32 i, int32 j);
	float Average(int32 i, int32 j);
	void CalcAllPatchBoundsY();
	void CalcPatchBoundsY(uint32 i, uint32 j);
	void BuildQuadPatchVB();
	void BuildHeightmapSRV();
	void SetMeshName(wstring name) { _meshName = name; }

	vector<uint32> BuildQuadPatchIB();
	ComPtr<ID3D11ShaderResourceView> GetLayerMapArraySRV() { return _layerMapArraySRV; }
	ComPtr<ID3D11ShaderResourceView> GetBlendMapSRV(){ return _blendMapSRV; }
	ComPtr<ID3D11ShaderResourceView> GetHeightMapSRV() { return _heightMapSRV; }
	TerrainInfo GetTerrainInfo() { return _info; }
	wstring GetMeshName() { return _meshName; }
	
private:
	shared_ptr<Geometry<VertexTextureNormalTangentBlendData>> _geometry;
	shared_ptr<Geometry<VertexTerrain>> _geometryForTerrain;
	shared_ptr<Buffer> _buffer;
	int32 _sizeofGridX = 0;
	int32 _sizeofGridZ = 0;

	// for terrain
	TerrainInfo _info;
	uint32 _numPatchVertices = 0;
	uint32 _numPatchQuadFaces = 0;
	uint32 _numPatchVertRows = 0;
	uint32 _numPatchVertCols = 0;
	vector<Vec2> _patchBoundsY;
	vector<float> _heightmap;

	ComPtr<ID3D11ShaderResourceView> _layerMapArraySRV;
	ComPtr<ID3D11ShaderResourceView> _blendMapSRV;
	ComPtr<ID3D11ShaderResourceView> _heightMapSRV;
	// Divide heightmap into patches such that each patch has CellsPerPatch cells
	// and CellsPerPatch+1 vertices.  Use 64 so that if we tessellate all the way 
	// to 64, we use all the data from the heightmap.  
	static const int CellsPerPatch = 64;
	wstring _meshName = L"None";
};

