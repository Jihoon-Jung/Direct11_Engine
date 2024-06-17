#include "pch.h"
#include "Mesh.h"

Mesh::Mesh()
	:Super(ResourceType::Mesh)
{
}

Mesh::~Mesh()
{
}
void Mesh::CreateCube()
{
	//float w2 = 0.5f;
	//float h2 = 0.5f;
	//float d2 = 0.5f;

	//vector<VertexTextureData> vtx(24);

	//// ¾Õ¸é
	//vtx[0] = VertexTextureData{ Vec3(-w2, -h2, -d2), Vec2(0.0f, 1.0f) };
	//vtx[1] = VertexTextureData{ Vec3(-w2, +h2, -d2), Vec2(0.0f, 0.0f) };
	//vtx[2] = VertexTextureData{ Vec3(+w2, +h2, -d2), Vec2(1.0f, 0.0f) };
	//vtx[3] = VertexTextureData{ Vec3(+w2, -h2, -d2), Vec2(1.0f, 1.0f) };
	//// µÞ¸é
	//vtx[4] = VertexTextureData{ Vec3(-w2, -h2, +d2), Vec2(1.0f, 1.0f) };
	//vtx[5] = VertexTextureData{ Vec3(+w2, -h2, +d2), Vec2(0.0f, 1.0f) };
	//vtx[6] = VertexTextureData{ Vec3(+w2, +h2, +d2), Vec2(0.0f, 0.0f) };
	//vtx[7] = VertexTextureData{ Vec3(-w2, +h2, +d2), Vec2(1.0f, 0.0f) };
	//// À­¸é
	//vtx[8] = VertexTextureData{ Vec3(-w2, +h2, -d2), Vec2(0.0f, 1.0f) };
	//vtx[9] = VertexTextureData{ Vec3(-w2, +h2, +d2), Vec2(0.0f, 0.0f) };
	//vtx[10] = VertexTextureData{ Vec3(+w2, +h2, +d2), Vec2(1.0f, 0.0f) };
	//vtx[11] = VertexTextureData{ Vec3(+w2, +h2, -d2), Vec2(1.0f, 1.0f) };
	//// ¾Æ·§¸é
	//vtx[12] = VertexTextureData{ Vec3(-w2, -h2, -d2), Vec2(1.0f, 1.0f) };
	//vtx[13] = VertexTextureData{ Vec3(+w2, -h2, -d2), Vec2(0.0f, 1.0f) };
	//vtx[14] = VertexTextureData{ Vec3(+w2, -h2, +d2), Vec2(0.0f, 0.0f) };
	//vtx[15] = VertexTextureData{ Vec3(-w2, -h2, +d2), Vec2(1.0f, 0.0f) };
	//// ¿ÞÂÊ¸é
	//vtx[16] = VertexTextureData{ Vec3(-w2, -h2, +d2), Vec2(0.0f, 1.0f) };
	//vtx[17] = VertexTextureData{ Vec3(-w2, +h2, +d2), Vec2(0.0f, 0.0f) };
	//vtx[18] = VertexTextureData{ Vec3(-w2, +h2, -d2), Vec2(1.0f, 0.0f) };
	//vtx[19] = VertexTextureData{ Vec3(-w2, -h2, -d2), Vec2(1.0f, 1.0f) };
	//// ¿À¸¥ÂÊ¸é
	//vtx[20] = VertexTextureData{ Vec3(+w2, -h2, -d2), Vec2(0.0f, 1.0f) };
	//vtx[21] = VertexTextureData{ Vec3(+w2, +h2, -d2), Vec2(0.0f, 0.0f) };
	//vtx[22] = VertexTextureData{ Vec3(+w2, +h2, +d2), Vec2(1.0f, 0.0f) };
	//vtx[23] = VertexTextureData{ Vec3(+w2, -h2, +d2), Vec2(1.0f, 1.0f) };

	//

	//vector<uint32> idx(36);

	//// ¾Õ¸é
	//idx[0] = 0; idx[1] = 1; idx[2] = 2;
	//idx[3] = 0; idx[4] = 2; idx[5] = 3;
	//// µÞ¸é
	//idx[6] = 4; idx[7] = 5; idx[8] = 6;
	//idx[9] = 4; idx[10] = 6; idx[11] = 7;
	//// À­¸é
	//idx[12] = 8; idx[13] = 9; idx[14] = 10;
	//idx[15] = 8; idx[16] = 10; idx[17] = 11;
	//// ¾Æ·§¸é
	//idx[18] = 12; idx[19] = 13; idx[20] = 14;
	//idx[21] = 12; idx[22] = 14; idx[23] = 15;
	//// ¿ÞÂÊ¸é
	//idx[24] = 16; idx[25] = 17; idx[26] = 18;
	//idx[27] = 16; idx[28] = 18; idx[29] = 19;
	//// ¿À¸¥ÂÊ¸é
	//idx[30] = 20; idx[31] = 21; idx[32] = 22;
	//idx[33] = 20; idx[34] = 22; idx[35] = 23;

	//_geometry = make_shared<Geometry<VertexTextureData>>();
	//_geometry->SetVertices(vtx);
	//_geometry->SetIndices(idx);

	//_buffer = make_shared<Buffer>();
	//_buffer->CreateBuffer(BufferType::VERTEX_BUFFER, _geometry->GetVertices());
	//_buffer->CreateBuffer(BufferType::INDEX_BUFFER, _geometry->GetIndices());
}

void Mesh::CreateCube_Normal()
{
	//float w2 = 0.5f;
	//float h2 = 0.5f;
	//float d2 = 0.5f;

	//vector<VertexTextureNormalData> vtx(24);

	//// ¾Õ¸é
	//vtx[0] = VertexTextureNormalData(Vec3(-w2, -h2, -d2), Vec2(0.0f, 1.0f), Vec3(0.0f, 0.0f, -1.0f));
	//vtx[1] = VertexTextureNormalData(Vec3(-w2, +h2, -d2), Vec2(0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f));
	//vtx[2] = VertexTextureNormalData(Vec3(+w2, +h2, -d2), Vec2(1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f));
	//vtx[3] = VertexTextureNormalData(Vec3(+w2, -h2, -d2), Vec2(1.0f, 1.0f), Vec3(0.0f, 0.0f, -1.0f));
	//// µÞ¸é
	//vtx[4] = VertexTextureNormalData(Vec3(-w2, -h2, +d2), Vec2(1.0f, 1.0f), Vec3(0.0f, 0.0f, 1.0f));
	//vtx[5] = VertexTextureNormalData(Vec3(+w2, -h2, +d2), Vec2(0.0f, 1.0f), Vec3(0.0f, 0.0f, 1.0f));
	//vtx[6] = VertexTextureNormalData(Vec3(+w2, +h2, +d2), Vec2(0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f));
	//vtx[7] = VertexTextureNormalData(Vec3(-w2, +h2, +d2), Vec2(1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f));
	//// À­¸é
	//vtx[8] = VertexTextureNormalData(Vec3(-w2, +h2, -d2), Vec2(0.0f, 1.0f), Vec3(0.0f, 1.0f, 0.0f));
	//vtx[9] = VertexTextureNormalData(Vec3(-w2, +h2, +d2), Vec2(0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
	//vtx[10] = VertexTextureNormalData(Vec3(+w2, +h2, +d2), Vec2(1.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
	//vtx[11] = VertexTextureNormalData(Vec3(+w2, +h2, -d2), Vec2(1.0f, 1.0f), Vec3(0.0f, 1.0f, 0.0f));
	//// ¾Æ·§¸é
	//vtx[12] = VertexTextureNormalData(Vec3(-w2, -h2, -d2), Vec2(1.0f, 1.0f), Vec3(0.0f, -1.0f, 0.0f));
	//vtx[13] = VertexTextureNormalData(Vec3(+w2, -h2, -d2), Vec2(0.0f, 1.0f), Vec3(0.0f, -1.0f, 0.0f));
	//vtx[14] = VertexTextureNormalData(Vec3(+w2, -h2, +d2), Vec2(0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f));
	//vtx[15] = VertexTextureNormalData(Vec3(-w2, -h2, +d2), Vec2(1.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f));
	//// ¿ÞÂÊ¸é
	//vtx[16] = VertexTextureNormalData(Vec3(-w2, -h2, +d2), Vec2(0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f));
	//vtx[17] = VertexTextureNormalData(Vec3(-w2, +h2, +d2), Vec2(0.0f, 0.0f), Vec3(-1.0f, 0.0f, 0.0f));
	//vtx[18] = VertexTextureNormalData(Vec3(-w2, +h2, -d2), Vec2(1.0f, 0.0f), Vec3(-1.0f, 0.0f, 0.0f));
	//vtx[19] = VertexTextureNormalData(Vec3(-w2, -h2, -d2), Vec2(1.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f));
	//// ¿À¸¥ÂÊ¸é
	//vtx[20] = VertexTextureNormalData(Vec3(+w2, -h2, -d2), Vec2(0.0f, 1.0f), Vec3(1.0f, 0.0f, 0.0f));
	//vtx[21] = VertexTextureNormalData(Vec3(+w2, +h2, -d2), Vec2(0.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f));
	//vtx[22] = VertexTextureNormalData(Vec3(+w2, +h2, +d2), Vec2(1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f));
	//vtx[23] = VertexTextureNormalData(Vec3(+w2, -h2, +d2), Vec2(1.0f, 1.0f), Vec3(1.0f, 0.0f, 0.0f));

	//vector<uint32> idx(36);

	//// ¾Õ¸é
	//idx[0] = 0; idx[1] = 1; idx[2] = 2;
	//idx[3] = 0; idx[4] = 2; idx[5] = 3;
	//// µÞ¸é
	//idx[6] = 4; idx[7] = 5; idx[8] = 6;
	//idx[9] = 4; idx[10] = 6; idx[11] = 7;
	//// À­¸é
	//idx[12] = 8; idx[13] = 9; idx[14] = 10;
	//idx[15] = 8; idx[16] = 10; idx[17] = 11;
	//// ¾Æ·§¸é
	//idx[18] = 12; idx[19] = 13; idx[20] = 14;
	//idx[21] = 12; idx[22] = 14; idx[23] = 15;
	//// ¿ÞÂÊ¸é
	//idx[24] = 16; idx[25] = 17; idx[26] = 18;
	//idx[27] = 16; idx[28] = 18; idx[29] = 19;
	//// ¿À¸¥ÂÊ¸é
	//idx[30] = 20; idx[31] = 21; idx[32] = 22;
	//idx[33] = 20; idx[34] = 22; idx[35] = 23;

	//_geometry = make_shared<Geometry<VertexTextureNormalData>>();
	//_geometry->SetVertices(vtx);
	//_geometry->SetIndices(idx);

	//_buffer = make_shared<Buffer>();
	//_buffer->CreateBuffer(BufferType::VERTEX_BUFFER, _geometry->GetVertices());
	//_buffer->CreateBuffer(BufferType::INDEX_BUFFER, _geometry->GetIndices());
}

void Mesh::CreateSphere_Normal()
{
	//float radius = 0.5f; // ±¸ÀÇ ¹ÝÁö¸§
	//uint32 stackCount = 20; // °¡·Î ºÐÇÒ
	//uint32 sliceCount = 20; // ¼¼·Î ºÐÇÒ

	//vector<VertexTextureNormalData> vtx;

	//VertexTextureNormalData v;

	//// ºÏ±Ø
	//v.position = Vec3(0.0f, radius, 0.0f);
	//v.uv = Vec2(0.5f, 0.0f);
	//v.normal = v.position;
	//v.normal.Normalize();
	//vtx.push_back(v);

	//float stackAngle = XM_PI / stackCount;
	//float sliceAngle = XM_2PI / sliceCount;

	//float deltaU = 1.f / static_cast<float>(sliceCount);
	//float deltaV = 1.f / static_cast<float>(stackCount);

	//// °í¸®¸¶´Ù µ¹¸é¼­ Á¤Á¡À» °è»êÇÑ´Ù (ºÏ±Ø/³²±Ø ´ÜÀÏÁ¡Àº °í¸®°¡ X)
	//for (uint32 y = 1; y <= stackCount - 1; ++y)
	//{
	//	float phi = y * stackAngle;

	//	// °í¸®¿¡ À§Ä¡ÇÑ Á¤Á¡
	//	for (uint32 x = 0; x <= sliceCount; ++x)
	//	{
	//		float theta = x * sliceAngle;

	//		v.position.x = radius * sinf(phi) * cosf(theta);
	//		v.position.y = radius * cosf(phi);
	//		v.position.z = radius * sinf(phi) * sinf(theta);

	//		v.uv = Vec2(deltaU * x, deltaV * y);

	//		v.normal = v.position;
	//		v.normal.Normalize();

	//		vtx.push_back(v);
	//	}
	//}

	//// ³²±Ø
	//v.position = Vec3(0.0f, -radius, 0.0f);
	//v.uv = Vec2(0.5f, 1.0f);
	//v.normal = v.position;
	//v.normal.Normalize();
	//vtx.push_back(v);


	//vector<uint32> idx(36);

	//// ºÏ±Ø ÀÎµ¦½º
	//for (uint32 i = 0; i <= sliceCount; ++i)
	//{
	//	//  [0]
	//	//   |  \
	//	//  [i+1]-[i+2]
	//	idx.push_back(0);
	//	idx.push_back(i + 2);
	//	idx.push_back(i + 1);
	//}

	//// ¸öÅë ÀÎµ¦½º
	//uint32 ringVertexCount = sliceCount + 1;
	//for (uint32 y = 0; y < stackCount - 2; ++y)
	//{
	//	for (uint32 x = 0; x < sliceCount; ++x)
	//	{
	//		//  [y, x]-[y, x+1]
	//		//  |		/
	//		//  [y+1, x]
	//		idx.push_back(1 + (y)*ringVertexCount + (x));
	//		idx.push_back(1 + (y)*ringVertexCount + (x + 1));
	//		idx.push_back(1 + (y + 1) * ringVertexCount + (x));
	//		//		 [y, x+1]
	//		//		 /	  |
	//		//  [y+1, x]-[y+1, x+1]
	//		idx.push_back(1 + (y + 1) * ringVertexCount + (x));
	//		idx.push_back(1 + (y)*ringVertexCount + (x + 1));
	//		idx.push_back(1 + (y + 1) * ringVertexCount + (x + 1));
	//	}
	//}

	//// ³²±Ø ÀÎµ¦½º
	//uint32 bottomIndex = static_cast<uint32>(vtx.size()) - 1;
	//uint32 lastRingStartIndex = bottomIndex - ringVertexCount;
	//for (uint32 i = 0; i < sliceCount; ++i)
	//{
	//	//  [last+i]-[last+i+1]
	//	//  |      /
	//	//  [bottom]
	//	idx.push_back(bottomIndex);
	//	idx.push_back(lastRingStartIndex + i);
	//	idx.push_back(lastRingStartIndex + i + 1);
	//}

	//_geometry = make_shared<Geometry<VertexTextureNormalData>>();
	//_geometry->SetVertices(vtx);
	//_geometry->SetIndices(idx);

	//_buffer = make_shared<Buffer>();
	//_buffer->CreateBuffer(BufferType::VERTEX_BUFFER, _geometry->GetVertices());
	//_buffer->CreateBuffer(BufferType::INDEX_BUFFER, _geometry->GetIndices());
}
void Mesh::CreateCube_NormalTangent()
{
	float w2 = 0.5f;
	float h2 = 0.5f;
	float d2 = 0.5f;

	vector<VertexTextureNormalTangentBlendData> vtx(24);

	// ¾Õ¸é
	vtx[0] = VertexTextureNormalTangentBlendData(Vec3(-w2, -h2, -d2), Vec2(0.0f, 1.0f), Vec3(0.0f, 0.0f, -1.0f), Vec3(1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[1] = VertexTextureNormalTangentBlendData(Vec3(-w2, +h2, -d2), Vec2(0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f), Vec3(1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[2] = VertexTextureNormalTangentBlendData(Vec3(+w2, +h2, -d2), Vec2(1.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f), Vec3(1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[3] = VertexTextureNormalTangentBlendData(Vec3(+w2, -h2, -d2), Vec2(1.0f, 1.0f), Vec3(0.0f, 0.0f, -1.0f), Vec3(1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	// µÞ¸é
	vtx[4] = VertexTextureNormalTangentBlendData(Vec3(-w2, -h2, +d2), Vec2(1.0f, 1.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[5] = VertexTextureNormalTangentBlendData(Vec3(+w2, -h2, +d2), Vec2(0.0f, 1.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[6] = VertexTextureNormalTangentBlendData(Vec3(+w2, +h2, +d2), Vec2(0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[7] = VertexTextureNormalTangentBlendData(Vec3(-w2, +h2, +d2), Vec2(1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	// À­¸é
	vtx[8] = VertexTextureNormalTangentBlendData(Vec3(-w2, +h2, -d2), Vec2(0.0f, 1.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[9] = VertexTextureNormalTangentBlendData(Vec3(-w2, +h2, +d2), Vec2(0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[10] = VertexTextureNormalTangentBlendData(Vec3(+w2, +h2, +d2), Vec2(1.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[11] = VertexTextureNormalTangentBlendData(Vec3(+w2, +h2, -d2), Vec2(1.0f, 1.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	// ¾Æ·§¸é
	vtx[12] = VertexTextureNormalTangentBlendData(Vec3(-w2, -h2, -d2), Vec2(1.0f, 1.0f), Vec3(0.0f, -1.0f, 0.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[13] = VertexTextureNormalTangentBlendData(Vec3(+w2, -h2, -d2), Vec2(0.0f, 1.0f), Vec3(0.0f, -1.0f, 0.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[14] = VertexTextureNormalTangentBlendData(Vec3(+w2, -h2, +d2), Vec2(0.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[15] = VertexTextureNormalTangentBlendData(Vec3(-w2, -h2, +d2), Vec2(1.0f, 0.0f), Vec3(0.0f, -1.0f, 0.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec4(1.0f), Vec4(1.0f));
	// ¿ÞÂÊ¸é
	vtx[16] = VertexTextureNormalTangentBlendData(Vec3(-w2, -h2, +d2), Vec2(0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[17] = VertexTextureNormalTangentBlendData(Vec3(-w2, +h2, +d2), Vec2(0.0f, 0.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[18] = VertexTextureNormalTangentBlendData(Vec3(-w2, +h2, -d2), Vec2(1.0f, 0.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[19] = VertexTextureNormalTangentBlendData(Vec3(-w2, -h2, -d2), Vec2(1.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, -1.0f), Vec4(1.0f), Vec4(1.0f));
	// ¿À¸¥ÂÊ¸é
	vtx[20] = VertexTextureNormalTangentBlendData(Vec3(+w2, -h2, -d2), Vec2(0.0f, 1.0f), Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[21] = VertexTextureNormalTangentBlendData(Vec3(+w2, +h2, -d2), Vec2(0.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[22] = VertexTextureNormalTangentBlendData(Vec3(+w2, +h2, +d2), Vec2(1.0f, 0.0f), Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec4(1.0f), Vec4(1.0f));
	vtx[23] = VertexTextureNormalTangentBlendData(Vec3(+w2, -h2, +d2), Vec2(1.0f, 1.0f), Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f), Vec4(1.0f), Vec4(1.0f));


	vector<uint32> idx(36);

	// ¾Õ¸é
	idx[0] = 0; idx[1] = 1; idx[2] = 2;
	idx[3] = 0; idx[4] = 2; idx[5] = 3;
	// µÞ¸é
	idx[6] = 4; idx[7] = 5; idx[8] = 6;
	idx[9] = 4; idx[10] = 6; idx[11] = 7;
	// À­¸é
	idx[12] = 8; idx[13] = 9; idx[14] = 10;
	idx[15] = 8; idx[16] = 10; idx[17] = 11;
	// ¾Æ·§¸é
	idx[18] = 12; idx[19] = 13; idx[20] = 14;
	idx[21] = 12; idx[22] = 14; idx[23] = 15;
	// ¿ÞÂÊ¸é
	idx[24] = 16; idx[25] = 17; idx[26] = 18;
	idx[27] = 16; idx[28] = 18; idx[29] = 19;
	// ¿À¸¥ÂÊ¸é
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
	float radius = 0.5f; // ±¸ÀÇ ¹ÝÁö¸§
	uint32 stackCount = 20; // °¡·Î ºÐÇÒ
	uint32 sliceCount = 20; // ¼¼·Î ºÐÇÒ

	vector<VertexTextureNormalTangentBlendData> vtx;

	VertexTextureNormalTangentBlendData v;

	// ºÏ±Ø
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

	// °í¸®¸¶´Ù µ¹¸é¼­ Á¤Á¡À» °è»êÇÑ´Ù (ºÏ±Ø/³²±Ø ´ÜÀÏÁ¡Àº °í¸®°¡ X)
	for (uint32 y = 1; y <= stackCount - 1; ++y)
	{
		float phi = y * stackAngle;

		// °í¸®¿¡ À§Ä¡ÇÑ Á¤Á¡
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

	// ³²±Ø
	v.position = Vec3(0.0f, -radius, 0.0f);
	v.uv = Vec2(0.5f, 1.0f);
	v.normal = v.position;
	v.normal.Normalize();
	v.tangent = Vec3(1.0f, 0.0f, 0.0f);
	v.tangent.Normalize();
	vtx.push_back(v);


	vector<uint32> idx(36);

	// ºÏ±Ø ÀÎµ¦½º
	for (uint32 i = 0; i <= sliceCount; ++i)
	{
		//  [0]
		//   |  \
		//  [i+1]-[i+2]
		idx.push_back(0);
		idx.push_back(i + 2);
		idx.push_back(i + 1);
	}

	// ¸öÅë ÀÎµ¦½º
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

	// ³²±Ø ÀÎµ¦½º
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
void Mesh::CreateRectangle()
{
	/*vector<VertexTextureData> vertices;
	vertices.resize(4);

	vertices[0].position = Vec3(-0.5f, -0.5f, 0.f);
	vertices[0].uv = Vec2(0.f, 1.f);
	vertices[1].position = Vec3(-0.5f, 0.5f, 0.f);
	vertices[1].uv = Vec2(0.f, 0.f);
	vertices[2].position = Vec3(0.5f, -0.5f, 0.f);
	vertices[2].uv = Vec2(1.f, 1.f);
	vertices[3].position = Vec3(0.5f, 0.5f, 0.f);
	vertices[3].uv = Vec2(1.f, 0.f);

	_geometry = make_shared<Geometry<VertexTextureData>>();
	_geometry->SetVertices(vertices);

	vector<uint32> indices = { 0, 1, 2, 2, 1, 3 };
	_geometry->SetIndices(indices);

	_buffer = make_shared<Buffer>();
	_buffer->CreateBuffer(BufferType::VERTEX_BUFFER, _geometry->GetVertices());
	_buffer->CreateBuffer(BufferType::INDEX_BUFFER, _geometry->GetIndices());*/
}
void Mesh::CreateSphere()
{
	//float radius = 0.5f; // ±¸ÀÇ ¹ÝÁö¸§
	//uint32 stackCount = 20; // °¡·Î ºÐÇÒ
	//uint32 sliceCount = 20; // ¼¼·Î ºÐÇÒ

	//vector<VertexTextureData> vtx;

	//VertexTextureData v;

	//// ºÏ±Ø
	//v.position = Vec3(0.0f, radius, 0.0f);
	//v.uv = Vec2(0.5f, 0.0f);
	//vtx.push_back(v);

	//float stackAngle = XM_PI / stackCount;
	//float sliceAngle = XM_2PI / sliceCount;

	//float deltaU = 1.f / static_cast<float>(sliceCount);
	//float deltaV = 1.f / static_cast<float>(stackCount);

	//// °í¸®¸¶´Ù µ¹¸é¼­ Á¤Á¡À» °è»êÇÑ´Ù (ºÏ±Ø/³²±Ø ´ÜÀÏÁ¡Àº °í¸®°¡ X)
	//for (uint32 y = 1; y <= stackCount - 1; ++y)
	//{
	//	float phi = y * stackAngle;

	//	// °í¸®¿¡ À§Ä¡ÇÑ Á¤Á¡
	//	for (uint32 x = 0; x <= sliceCount; ++x)
	//	{
	//		float theta = x * sliceAngle;

	//		v.position.x = radius * sinf(phi) * cosf(theta);
	//		v.position.y = radius * cosf(phi);
	//		v.position.z = radius * sinf(phi) * sinf(theta);

	//		v.uv = Vec2(deltaU * x, deltaV * y);

	//		vtx.push_back(v);
	//	}
	//}

	//// ³²±Ø
	//v.position = Vec3(0.0f, -radius, 0.0f);
	//v.uv = Vec2(0.5f, 1.0f);
	//vtx.push_back(v);

	//vector<uint32> idx(36);

	//// ºÏ±Ø ÀÎµ¦½º
	//for (uint32 i = 0; i <= sliceCount; ++i)
	//{
	//	//  [0]
	//	//   |  \
	//	//  [i+1]-[i+2]
	//	idx.push_back(0);
	//	idx.push_back(i + 2);
	//	idx.push_back(i + 1);
	//}

	//// ¸öÅë ÀÎµ¦½º
	//uint32 ringVertexCount = sliceCount + 1;
	//for (uint32 y = 0; y < stackCount - 2; ++y)
	//{
	//	for (uint32 x = 0; x < sliceCount; ++x)
	//	{
	//		//  [y, x]-[y, x+1]
	//		//  |		/
	//		//  [y+1, x]
	//		idx.push_back(1 + (y)*ringVertexCount + (x));
	//		idx.push_back(1 + (y)*ringVertexCount + (x + 1));
	//		idx.push_back(1 + (y + 1) * ringVertexCount + (x));
	//		//		 [y, x+1]
	//		//		 /	  |
	//		//  [y+1, x]-[y+1, x+1]
	//		idx.push_back(1 + (y + 1) * ringVertexCount + (x));
	//		idx.push_back(1 + (y)*ringVertexCount + (x + 1));
	//		idx.push_back(1 + (y + 1) * ringVertexCount + (x + 1));
	//	}
	//}

	//// ³²±Ø ÀÎµ¦½º
	//uint32 bottomIndex = static_cast<uint32>(vtx.size()) - 1;
	//uint32 lastRingStartIndex = bottomIndex - ringVertexCount;
	//for (uint32 i = 0; i < sliceCount; ++i)
	//{
	//	//  [last+i]-[last+i+1]
	//	//  |      /
	//	//  [bottom]
	//	idx.push_back(bottomIndex);
	//	idx.push_back(lastRingStartIndex + i);
	//	idx.push_back(lastRingStartIndex + i + 1);
	//}
	//_geometry = make_shared<Geometry<VertexTextureData>>();
	//_geometry->SetVertices(vtx);
	//_geometry->SetIndices(idx);

	//_buffer = make_shared<Buffer>();
	//_buffer->CreateBuffer(BufferType::VERTEX_BUFFER, _geometry->GetVertices());
	//_buffer->CreateBuffer(BufferType::INDEX_BUFFER, _geometry->GetIndices());
}
