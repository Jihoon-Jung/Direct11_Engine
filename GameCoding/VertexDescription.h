#pragma once
struct VertexTextureData
{
	Vec3 position = { 0.f, 0.f, 0.f };
	Vec2 uv = { 0.f, 0.f };

	static vector<D3D11_INPUT_ELEMENT_DESC> descs;
};
struct VertexTextureNormalData
{
	Vec3 position = { 0.f, 0.f, 0.f };
	Vec2 uv = { 0.f, 0.f };
	Vec3 normal = { 0.f, 0.f, 0.f };
	VertexTextureNormalData() {};
	VertexTextureNormalData(const Vec3& pos, const Vec2& texCoord, const Vec3& norm)
		: position(pos), uv(texCoord), normal(norm) {}
	static vector<D3D11_INPUT_ELEMENT_DESC> descs;
};

struct VertexTextureNormalTangentData
{
	Vec3 position = { 0, 0, 0 };
	Vec2 uv = { 0, 0 };
	Vec3 normal = { 0, 0, 0 };
	Vec3 tangent = { 0, 0, 0 };
	VertexTextureNormalTangentData() {};
	VertexTextureNormalTangentData(const Vec3& pos, const Vec2& texCoord, const Vec3& norm, const Vec3& tan)
		: position(pos), uv(texCoord), normal(norm), tangent(tan) {}
	static vector<D3D11_INPUT_ELEMENT_DESC> descs;
};

struct VertexTextureNormalTangentBlendData
{
	Vec3 position = { 0, 0, 0 };
	Vec2 uv = { 0, 0 };
	Vec3 normal = { 0, 0, 0 };
	Vec3 tangent = { 0, 0, 0 };
	Vec4 blendIndices = { 0, 0, 0, 0 };
	Vec4 blendWeights = { 0, 0, 0, 0 };
	static vector<D3D11_INPUT_ELEMENT_DESC> descs;
};

struct VertexBillboard
{
	Vec3 position;
	Vec2 uv;
	Vec2 scale;
	static vector<D3D11_INPUT_ELEMENT_DESC> descs;
};