
#define MAX_MODEL_TRANSFORMS 250
#define MAX_MODEL_KEYFRAMES 500
#include "LightHelper.hlsl"

struct VS_INPUT
{
	float4 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float4 blendIndices : BLENDINDICES;
	float4 blendWeights : BLENDWEIGHTS;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float4 lightPosition : LIGHT_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 worldPosition : POSITION1;
};

cbuffer CameraBuffer : register(b0)
{
	row_major matrix viewMatrix;
	row_major matrix projectionMatrix;
}

cbuffer TransformBuffer : register(b1)
{
	row_major matrix worldMatrix;
}
cbuffer LightMaterial : register(b2)
{
	float4 materialDiffuse;
	float4 materialAmbient;
	float4 materialSpecular;
	float4 materialEmissive;
}
cbuffer LightDesc : register(b3)
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float4 emissive;

}
cbuffer LightAndCameraPos : register(b4)
{
	float3 lightPosition;
	float padding1;
	float3 cameraPosition;
	float padding2;
}

struct KeyframeDesc
{
	int animIndex;
	uint currFrame;
	uint nextFrame;
	float ratio;
	float sumTime;
	float speed;
	int activeAnimation;
	int padding3;
};
struct blendFrameDesc
{
	float blendDuration;
	float blendRatio;
	float blendSumTime;
	float padding;
	KeyframeDesc curr;
	KeyframeDesc next;
};

cbuffer BlendBuffer : register(b5)
{
	blendFrameDesc blendFrames;
};

cbuffer LightSpaceTransformBuffer : register(b6)
{
	row_major matrix light_viewMatrix;
	row_major matrix light_projectionMatrix;
};

SamplerState sampler0 : register(s0);
SamplerState shadowSampler : register(s1);
Texture2D normalMap : register(t0);
Texture2D specularMap : register(t1);
Texture2D diffuseMap : register(t2);
Texture2DArray TransformMap : register(t3);
Texture2D shadowMap : register(t4);


matrix GetAnimationMatrix(VS_INPUT input)
{
	float indices[4] = { input.blendIndices.x, input.blendIndices.y, input.blendIndices.z, input.blendIndices.w };
	float weights[4] = { input.blendWeights.x, input.blendWeights.y, input.blendWeights.z, input.blendWeights.w };

	int animIndex[2];
	int currFrame[2];
	int nextFrame[2];
	float ratio[2];

	animIndex[0] = blendFrames.curr.animIndex;
	currFrame[0] = blendFrames.curr.currFrame;
	nextFrame[0] = blendFrames.curr.nextFrame;
	ratio[0] = blendFrames.curr.ratio;

	animIndex[1] = blendFrames.next.animIndex;
	currFrame[1] = blendFrames.next.currFrame;
	nextFrame[1] = blendFrames.next.nextFrame;
	ratio[1] = blendFrames.next.ratio;

	float4 c0, c1, c2, c3;
	float4 n0, n1, n2, n3;
	matrix curr = 0;
	matrix next = 0;
	matrix transform = 0;

	for (int i = 0; i < 4; i++)
	{
		c0 = TransformMap.Load(int4(indices[i] * 4 + 0, currFrame[0], animIndex[0], 0));
		c1 = TransformMap.Load(int4(indices[i] * 4 + 1, currFrame[0], animIndex[0], 0));
		c2 = TransformMap.Load(int4(indices[i] * 4 + 2, currFrame[0], animIndex[0], 0));
		c3 = TransformMap.Load(int4(indices[i] * 4 + 3, currFrame[0], animIndex[0], 0));
		curr = matrix(c0, c1, c2, c3);

		n0 = TransformMap.Load(int4(indices[i] * 4 + 0, nextFrame[0], animIndex[0], 0));
		n1 = TransformMap.Load(int4(indices[i] * 4 + 1, nextFrame[0], animIndex[0], 0));
		n2 = TransformMap.Load(int4(indices[i] * 4 + 2, nextFrame[0], animIndex[0], 0));
		n3 = TransformMap.Load(int4(indices[i] * 4 + 3, nextFrame[0], animIndex[0], 0));
		next = matrix(n0, n1, n2, n3);

		matrix result = lerp(curr, next, ratio[0]);

		// ���� �ִϸ��̼�
		if (animIndex[1] >= 0)
		{
			c0 = TransformMap.Load(int4(indices[i] * 4 + 0, currFrame[1], animIndex[1], 0));
			c1 = TransformMap.Load(int4(indices[i] * 4 + 1, currFrame[1], animIndex[1], 0));
			c2 = TransformMap.Load(int4(indices[i] * 4 + 2, currFrame[1], animIndex[1], 0));
			c3 = TransformMap.Load(int4(indices[i] * 4 + 3, currFrame[1], animIndex[1], 0));
			curr = matrix(c0, c1, c2, c3);

			n0 = TransformMap.Load(int4(indices[i] * 4 + 0, nextFrame[1], animIndex[1], 0));
			n1 = TransformMap.Load(int4(indices[i] * 4 + 1, nextFrame[1], animIndex[1], 0));
			n2 = TransformMap.Load(int4(indices[i] * 4 + 2, nextFrame[1], animIndex[1], 0));
			n3 = TransformMap.Load(int4(indices[i] * 4 + 3, nextFrame[1], animIndex[1], 0));
			next = matrix(n0, n1, n2, n3);

			matrix nextResult = lerp(curr, next, ratio[1]);
			result = lerp(result, nextResult, blendFrames.blendRatio);
		}

		transform += mul(weights[i], result);
	}

	return transform;
}
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;
	matrix m;

	if (blendFrames.curr.activeAnimation == 1)
	{
		m = GetAnimationMatrix(input); // �ִϸ��̼� ����
	}
	else
	{
		m = matrix(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1
			);
	}
	// ���� ��ȯ
	output.position = mul(input.position, m);
	output.position = mul(output.position, worldMatrix);

	// light space ��ȯ
	output.lightPosition = mul(output.position, light_viewMatrix);
	output.lightPosition = mul(output.lightPosition, light_projectionMatrix);

	output.worldPosition = output.position.xyz;
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	output.uv = input.uv;

	// ��ְ� ź��Ʈ�� �ִϸ��̼� ��� ���� (ȸ�� ���и� �����ؼ� ����)
	float3x3 rotationMatrix = (float3x3)m; // �ִϸ��̼� ��Ʈ������ ȸ�� �κи� ���
	output.normal = normalize(mul(input.normal, rotationMatrix));
	output.normal = normalize(mul(output.normal, (float3x3)worldMatrix));

	output.tangent = mul(input.tangent, rotationMatrix); // ź��Ʈ ��ȯ
	output.tangent = mul(output.tangent, (float3x3)worldMatrix); // ���� ��ȯ

	return output;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
	float3 lightDirection = normalize(lightPosition);
	float3 viewDirection = normalize(cameraPosition - input.worldPosition);
	float3 normal = normalize(input.normal);
	float4 textureColor = diffuseMap.Sample(sampler0, input.uv);

	float3 n = normalMap.Sample(sampler0, input.uv).rgb;
	float3 bumpedNormal = NormalSampleToWorldSpace(n, input.normal, input.tangent);


	Material mat;
	mat.Ambient = materialAmbient;
	mat.Diffuse = materialDiffuse;
	mat.Specular = materialSpecular;

	DirectionalLight light;
	light.Ambient = ambient + float4(0.3, 0.3, 0.3, 1.0);
	light.Diffuse = diffuse + float4(0.3, 0.3, 0.3, 1.0);
	light.Specular = specular + float4(0.3, 0.3, 0.3, 1.0);
	light.Direction = lightDirection;

	Shadow shadow;  
	shadow.lightPosition = input.lightPosition;
	shadow.shadowSampler = shadowSampler;
	shadow.shadowMap = shadowMap;
	shadow.normal = normal;
	shadow.lightDir = -lightDirection;
	float shadowFactor = CalculateShadowFactor(shadow);

	ComputeDirectionalLight(mat, light, normal, viewDirection, textureColor, shadowFactor);

	return textureColor;
}










//#define MAX_MODEL_TRANSFORMS 250
//#define MAX_MODEL_KEYFRAMES 500
//#include "LightHelper.hlsl"
//
//struct VS_INPUT
//{
//	float4 position : POSITION;
//	float2 uv : TEXCOORD;
//	float3 normal : NORMAL;
//	float3 tangent : TANGENT;
//	float4 blendIndices : BLENDINDICES;
//	float4 blendWeights : BLENDWEIGHTS;
//};
//
//struct VS_OUTPUT
//{
//	float4 position : SV_POSITION;
//	float4 lightPosition : LIGHT_POSITION;
//	float2 uv : TEXCOORD;
//	float3 normal : NORMAL;
//	float3 tangent : TANGENT;
//	float3 worldPosition : POSITION1;
//};
//
//cbuffer CameraBuffer : register(b0)
//{
//	row_major matrix viewMatrix;
//	row_major matrix projectionMatrix;
//}
//
//cbuffer TransformBuffer : register(b1)
//{
//	row_major matrix worldMatrix;
//}
//cbuffer LightMaterial : register(b2)
//{
//	float4 materialDiffuse;
//	float4 materialAmbient;
//	float4 materialSpecular;
//	float4 materialEmissive;
//}
//cbuffer LightDesc : register(b3)
//{
//	float4 ambient;
//	float4 diffuse;
//	float4 specular;
//	float4 emissive;
//
//}
//cbuffer LightAndCameraPos : register(b4)
//{
//	float3 lightPosition;
//	float padding1;
//	float3 cameraPosition;
//	float padding2;
//}
//
//struct KeyframeDesc
//{
//	int animIndex;
//	uint currFrame;
//	uint nextFrame;
//	float ratio;
//	float sumTime;
//	float speed;
//	int activeAnimation;
//	int padding3;
//};
//struct blendFrameDesc
//{
//	float blendDuration;
//	float blendRatio;
//	float blendSumTime;
//	float padding;
//	KeyframeDesc curr;
//	KeyframeDesc next;
//};
//
//cbuffer BlendBuffer : register(b5)
//{
//	blendFrameDesc blendFrames;
//};
//
//cbuffer LightSpaceTransformBuffer : register(b6)
//{
//	row_major matrix light_viewMatrix;
//	row_major matrix light_projectionMatrix;
//};
//
//SamplerState sampler0 : register(s0);
//SamplerState shadowSampler : register(s1);
//Texture2D normalMap : register(t0);
//Texture2D specularMap : register(t1);
//Texture2D diffuseMap : register(t2);
//Texture2DArray TransformMap : register(t3);
//Texture2D shadowMap : register(t4);
//
//
/////////////////////////////////////////
//// ������ (T, R, S) ����ü ����
/////////////////////////////////////////
//struct TRS
//{
//	float3 T;       // Translation
//	float4 Q;       // Rotation(Quaternion)
//	float3 S;       // Scale
//};
//
//// ȸ�� ��� -> ���ʹϾ� ��ȯ 
//// (���Ĵ��� �����, ��Ȯ��/���� ����ȭ ����)
//float4 QuaternionFromRotationMatrix(float3x3 M)
//{
//	float4 q = 0;
//
//	float trace = M[0][0] + M[1][1] + M[2][2];
//	if (trace > 0.0)
//	{
//		float s = sqrt(trace + 1.0);
//		q.w = s * 0.5;
//		s = 0.5 / s;
//		q.x = (M[2][1] - M[1][2]) * s;
//		q.y = (M[0][2] - M[2][0]) * s;
//		q.z = (M[1][0] - M[0][1]) * s;
//	}
//	else
//	{
//		// �� �� ���� ū �밢���Ҹ� ��������
//		if (M[0][0] > M[1][1] && M[0][0] > M[2][2])
//		{
//			float s = sqrt(1.0 + M[0][0] - M[1][1] - M[2][2]);
//			float invS = 0.5 / s;
//			q.x = 0.5 * s;
//			q.y = (M[0][1] + M[1][0]) * invS;
//			q.z = (M[0][2] + M[2][0]) * invS;
//			q.w = (M[2][1] - M[1][2]) * invS;
//		}
//		else if (M[1][1] > M[2][2])
//		{
//			float s = sqrt(1.0 + M[1][1] - M[0][0] - M[2][2]);
//			float invS = 0.5 / s;
//			q.x = (M[0][1] + M[1][0]) * invS;
//			q.y = 0.5 * s;
//			q.z = (M[1][2] + M[2][1]) * invS;
//			q.w = (M[0][2] - M[2][0]) * invS;
//		}
//		else
//		{
//			float s = sqrt(1.0 + M[2][2] - M[0][0] - M[1][1]);
//			float invS = 0.5 / s;
//			q.x = (M[0][2] + M[2][0]) * invS;
//			q.y = (M[1][2] + M[2][1]) * invS;
//			q.z = 0.5 * s;
//			q.w = (M[1][0] - M[0][1]) * invS;
//		}
//	}
//
//	return normalize(q);
//}
//
/////////////////////////////////////////
//// ���(4x4) -> (T, R, S) ����
/////////////////////////////////////////
//TRS DecomposeMatrix(matrix m)
//{
//	TRS outTRS;
//
//	// 1) Translation ����
//	// m[3].xyz (HLSL���� row_major ������ �� ����)
//	float4 c3 = float4(m[3][0], m[3][1], m[3][2], m[3][3]);
//	outTRS.T = c3.xyz;
//
//	// 2) ������ ����
//	// ��� 3x3���� ������ �� ���̸� ����
//	float3x3 rot3x3 = (float3x3)m;
//	float3 scale;
//	scale.x = length(rot3x3[0]);
//	scale.y = length(rot3x3[1]);
//	scale.z = length(rot3x3[2]);
//	outTRS.S = scale;
//
//	// 3) ȸ�� ��� ���� (������ ����)
//	// rot3x3[i] /= scale; �� ������ ���� ȸ���� ����
//	if (scale.x != 0) rot3x3[0] /= scale.x;
//	if (scale.y != 0) rot3x3[1] /= scale.y;
//	if (scale.z != 0) rot3x3[2] /= scale.z;
//
//	// 4) ȸ�� ��� -> ���ʹϾ� ��ȯ
//	outTRS.Q = QuaternionFromRotationMatrix(rot3x3);
//
//	return outTRS;
//}
//
//// ���ʹϾ� -> ȸ�� ���
//float3x3 RotationMatrixFromQuaternion(float4 q)
//{
//	float xx = q.x * q.x;
//	float yy = q.y * q.y;
//	float zz = q.z * q.z;
//	float xy = q.x * q.y;
//	float xz = q.x * q.z;
//	float yz = q.y * q.z;
//	float wx = q.w * q.x;
//	float wy = q.w * q.y;
//	float wz = q.w * q.z;
//
//	float3x3 m;
//	m[0][0] = 1.0 - 2.0 * (yy + zz);
//	m[0][1] = 2.0 * (xy - wz);
//	m[0][2] = 2.0 * (xz + wy);
//	m[1][0] = 2.0 * (xy + wz);
//	m[1][1] = 1.0 - 2.0 * (xx + zz);
//	m[1][2] = 2.0 * (yz - wx);
//	m[2][0] = 2.0 * (xz - wy);
//	m[2][1] = 2.0 * (yz + wx);
//	m[2][2] = 1.0 - 2.0 * (xx + yy);
//
//	return m;
//}
//
////////////////////////////////////////////
//// (T, R, S) -> ���(4x4) �ռ�
////////////////////////////////////////////
//matrix ComposeMatrix(TRS t)
//{
//	// ȸ�� ���ʹϾ� -> 3x3
//	float3x3 r = RotationMatrixFromQuaternion(t.Q);
//
//	// ������ ����
//	r[0] *= t.S.x;
//	r[1] *= t.S.y;
//	r[2] *= t.S.z;
//
//	// 4x4 ��Ŀ� ����
//	matrix M = (matrix)0;
//	// row_major��� ����
//	M[0] = float4(r[0].x, r[0].y, r[0].z, 0);
//	M[1] = float4(r[1].x, r[1].y, r[1].z, 0);
//	M[2] = float4(r[2].x, r[2].y, r[2].z, 0);
//	M[3] = float4(t.T.x, t.T.y, t.T.z, 1);
//
//	return M;
//}
//
///////////////////////////////////////////
//// ���ʹϾ� slerp (���� ����)
///////////////////////////////////////////
//float4 QuaternionSlerp(float4 q1, float4 q2, float t)
//{
//	// (�� ������ ���� �Ǵ� ����ȭ)
//	// ���� slerp ���� �� dot üũ, ������ �����ֱ� �� ó�� �ʿ�
//	float dotVal = dot(q1, q2);
//
//	// ���� dot�̸� ��������(�� ª�� ������ ȸ��)
//	float4 q2Fix = (dotVal < 0) ? -q2 : q2;
//	if (dotVal < 0) dotVal = -dotVal;
//
//	// �Ӱ谪
//	const float THRESHOLD = 0.9995;
//	if (dotVal > THRESHOLD)
//	{
//		// ��������(Fast path)
//		float4 result = lerp(q1, q2Fix, t);
//		return normalize(result);
//	}
//
//	// ��¥ slerp
//	float theta0 = acos(dotVal);
//	float theta = theta0 * t;
//	float sinTheta = sin(theta);
//	float sinTheta0 = sin(theta0);
//	float s0 = cos(theta) - dotVal * (sinTheta / sinTheta0);
//	float s1 = sinTheta / sinTheta0;
//	return s0 * q1 + s1 * q2Fix;
//}
//
///////////////////////////////////////////
//// TRS���� ����
////  - T,S �� ����
////  - R �� ���ʹϾ� slerp
///////////////////////////////////////////
//TRS LerpTRS(TRS a, TRS b, float t)
//{
//	TRS result;
//	result.T = lerp(a.T, b.T, t);
//	result.S = lerp(a.S, b.S, t);
//	result.Q = QuaternionSlerp(a.Q, b.Q, t);
//	return result;
//}
//
//
//////////////////////////////////////////////
//// �� ����� �о���� �κ� (���� �ڵ忡�� Load)
//////////////////////////////////////////////
//matrix LoadBoneMatrix(float boneIdx, int frame, int animIdx)
//{
//	float4 c0 = TransformMap.Load(int4(boneIdx * 4 + 0, frame, animIdx, 0));
//	float4 c1 = TransformMap.Load(int4(boneIdx * 4 + 1, frame, animIdx, 0));
//	float4 c2 = TransformMap.Load(int4(boneIdx * 4 + 2, frame, animIdx, 0));
//	float4 c3 = TransformMap.Load(int4(boneIdx * 4 + 3, frame, animIdx, 0));
//	return matrix(c0, c1, c2, c3);
//}
//
//
//
//matrix GetAnimationMatrix(VS_INPUT input)
//{
//	float indices[4] = { input.blendIndices.x, input.blendIndices.y, input.blendIndices.z, input.blendIndices.w };
//	float weights[4] = { input.blendWeights.x, input.blendWeights.y, input.blendWeights.z, input.blendWeights.w };
//
//	int   animIndex[2];
//	int   currFrame[2];
//	int   nextFrame[2];
//	float ratio[2];
//
//	animIndex[0] = blendFrames.curr.animIndex;
//	currFrame[0] = blendFrames.curr.currFrame;
//	nextFrame[0] = blendFrames.curr.nextFrame;
//	ratio[0] = blendFrames.curr.ratio;
//
//	animIndex[1] = blendFrames.next.animIndex;
//	currFrame[1] = blendFrames.next.currFrame;
//	nextFrame[1] = blendFrames.next.nextFrame;
//	ratio[1] = blendFrames.next.ratio;
//
//	matrix transform = 0;
//
//	for (int i = 0; i < 4; i++)
//	{
//		float boneIndex = indices[i];
//		float boneWeight = weights[i];
//		if (boneWeight <= 0.0f)
//			continue;
//
//		//////////////////////////////////////////
//		// 1) ���� �ִϸ��̼�(Anim0) ��� ����
//		//////////////////////////////////////////
//		matrix curr0 = LoadBoneMatrix(boneIndex, currFrame[0], animIndex[0]);
//		matrix next0 = LoadBoneMatrix(boneIndex, nextFrame[0], animIndex[0]);
//
//		TRS currTRS0 = DecomposeMatrix(curr0);
//		TRS nextTRS0 = DecomposeMatrix(next0);
//
//		TRS resultA = LerpTRS(currTRS0, nextTRS0, ratio[0]);
//
//		//////////////////////////////////////////
//		// 2) ���� �ִϸ��̼�(Anim1) ��� ����
//		//////////////////////////////////////////
//		// animIndex[1]�� ��ȿ(>= 0)�� ����
//		TRS resultB = resultA; // default �� anim1�� ������ �׳� anim0
//		if (animIndex[1] >= 0)
//		{
//			matrix curr1 = LoadBoneMatrix(boneIndex, currFrame[1], animIndex[1]);
//			matrix next1 = LoadBoneMatrix(boneIndex, nextFrame[1], animIndex[1]);
//
//			TRS currTRS1 = DecomposeMatrix(curr1);
//			TRS nextTRS1 = DecomposeMatrix(next1);
//
//			TRS anim1TRS = LerpTRS(currTRS1, nextTRS1, ratio[1]);
//
//			// Ʈ������ ����
//			resultB = LerpTRS(resultA, anim1TRS, blendFrames.blendRatio);
//		}
//
//		//////////////////////////////////////////
//		// 3) ���� ��� �ռ� & ����ġ ����
//		//////////////////////////////////////////
//		matrix finalMat = ComposeMatrix(resultB);
//		transform += boneWeight * finalMat;
//	}
//
//	return transform;
//}
//
//
//
//VS_OUTPUT VS(VS_INPUT input)
//{
//	VS_OUTPUT output;
//	matrix m;
//
//	if (blendFrames.curr.activeAnimation == 1)
//	{
//		m = GetAnimationMatrix(input); // �ִϸ��̼� ����
//	}
//	else
//	{
//		m = matrix(
//			1, 0, 0, 0,
//			0, 1, 0, 0,
//			0, 0, 1, 0,
//			0, 0, 0, 1
//			);
//	}
//	// ���� ��ȯ
//	output.position = mul(input.position, m);
//	output.position = mul(output.position, worldMatrix);
//
//	// light space ��ȯ
//	output.lightPosition = mul(output.position, light_viewMatrix);
//	output.lightPosition = mul(output.lightPosition, light_projectionMatrix);
//
//	output.worldPosition = output.position.xyz;
//	output.position = mul(output.position, viewMatrix);
//	output.position = mul(output.position, projectionMatrix);
//	output.uv = input.uv;
//
//	// ��ְ� ź��Ʈ�� �ִϸ��̼� ��� ���� (ȸ�� ���и� �����ؼ� ����)
//	float3x3 rotationMatrix = (float3x3)m; // �ִϸ��̼� ��Ʈ������ ȸ�� �κи� ���
//	output.normal = normalize(mul(input.normal, rotationMatrix));
//	output.normal = normalize(mul(output.normal, (float3x3)worldMatrix));
//
//	output.tangent = mul(input.tangent, rotationMatrix); // ź��Ʈ ��ȯ
//	output.tangent = mul(output.tangent, (float3x3)worldMatrix); // ���� ��ȯ
//
//	return output;
//}
//
//float4 PS(VS_OUTPUT input) : SV_Target
//{
//	float3 lightDirection = normalize(lightPosition);
//	float3 viewDirection = normalize(cameraPosition - input.worldPosition);
//	float3 normal = normalize(input.normal);
//	float4 textureColor = diffuseMap.Sample(sampler0, input.uv);
//
//	float3 n = normalMap.Sample(sampler0, input.uv).rgb;
//	float3 bumpedNormal = NormalSampleToWorldSpace(n, input.normal, input.tangent);
//
//
//	Material mat;
//	mat.Ambient = materialAmbient;
//	mat.Diffuse = materialDiffuse;
//	mat.Specular = materialSpecular;
//
//	DirectionalLight light;
//	light.Ambient = ambient + float4(0.3, 0.3, 0.3, 1.0);
//	light.Diffuse = diffuse + float4(0.3, 0.3, 0.3, 1.0);
//	light.Specular = specular + float4(0.3, 0.3, 0.3, 1.0);
//	light.Direction = lightDirection;
//
//	Shadow shadow;
//	shadow.lightPosition = input.lightPosition;
//	shadow.shadowSampler = shadowSampler;
//	shadow.shadowMap = shadowMap;
//	shadow.normal = normal;
//	shadow.lightDir = -lightDirection;
//	float shadowFactor = CalculateShadowFactor(shadow);
//
//	ComputeDirectionalLight(mat, light, normal, viewDirection, textureColor, shadowFactor);
//
//	return textureColor;
//}