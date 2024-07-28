#pragma once
enum class InputLayoutType
{
	VertexTextureNormalTangentBlendData,
	VertexTextureNormalBillboard,
	VertexBillboard_Geometry
};
struct CameraBuffer
{
	Matrix viewMatrix = Matrix::Identity;
	Matrix projectionMatrix = Matrix::Identity;
};

struct TransformBuffer
{
	Matrix worldMatrix = Matrix::Identity;
	Matrix inverseTransposeWorldMatrix = Matrix::Identity;
};

struct WVPBuffer
{
	Matrix viewMatrix = Matrix::Identity;
	Matrix projectionMatrix = Matrix::Identity;
	Matrix worldMatrix = Matrix::Identity;
};
struct MaterialDesc
{
	Color ambient = Color(1.f, 1.f, 1.f, 1.f);
	Color diffuse = Color(1.f, 1.f, 1.f, 1.f);
	Color specular = Color(1.f, 1.f, 1.f, 1.f);
	Color emissive = Color(1.f, 1.f, 1.f, 1.f);
};
struct LightDesc
{
	Color ambient = Color(1.f, 1.f, 1.f, 1.f);
	Color diffuse = Color(1.f, 1.f, 1.f, 1.f);
	Color specular = Color(1.f, 1.f, 1.f, 1.f);
	Color emissive = Color(1.f, 1.f, 1.f, 1.f);

};
struct LightAndCameraPos
{
	Vec3 lightPosition = Vec3(0.f, 0.f, 0.f);
	float padding1 = 1.0f;
	Vec3 cameraPosition = Vec3(0.f, 0.f, 0.f);
	float padding2 = 1.0f;
};
struct CameraPos
{
	Vec3 cameraPosition = Vec3(0.f, 0.f, 0.f);
	float padding = 0.0f;
};
// Bone
#define MAX_MODEL_TRANSFORMS 250
// KeyFrame
#define MAX_MODEL_KEYFRAMES 500

struct BoneBuffer
{
	Matrix BoneTransforms[MAX_MODEL_TRANSFORMS];
};

struct BoneIndex
{
	uint32 index;
	Vec3 padding;
};

// Animation
struct KeyframeDesc
{
	int animIndex = 0;
	int currFrame = 0;
	int nextFrame = 0;
	float ratio = 0.f;
	float sumTime = 0.f;// 현재까지 경과된 시간
	float speed = 1.f; // 애니메이션 재생 속도
	Vec2 padding;
};

struct BlendAnimDesc
{
	BlendAnimDesc()
	{
		curr.animIndex = 0;
		next.animIndex = 1;
	}
	void SetAnimIndex(int i, int j)
	{
		curr.animIndex = i;
		next.animIndex = j;
	}
	void ClearNextAnim()
	{
		curr = next;
		next.animIndex = (next.animIndex + 1) % 3;
		next.currFrame = 0;
		next.nextFrame = 0;
		next.sumTime = 0;
		blendSumTime = 0;
		blendRatio = 0;
	}
	void SetAnimSpeed(float s1, float s2)
	{
		curr.speed = s1;
		next.speed = s2;
	}
	float blendDuration = 1.0f;
	float blendRatio = 0.f;
	float blendSumTime = 0.f;
	float padding = 0.f;
	KeyframeDesc curr;
	KeyframeDesc next;
};
struct AnimationTransformBuffer
{
	Matrix transform[11][67];
};