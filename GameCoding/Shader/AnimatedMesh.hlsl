
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

		// 다음 애니메이션
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
		m = GetAnimationMatrix(input); // 애니메이션 적용
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
	// 정점 변환
	output.position = mul(input.position, m);
	output.position = mul(output.position, worldMatrix);

	// light space 변환
	output.lightPosition = mul(output.position, light_viewMatrix);
	output.lightPosition = mul(output.lightPosition, light_projectionMatrix);

	output.worldPosition = output.position.xyz;
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	output.uv = input.uv;

	// 노멀과 탄젠트에 애니메이션 행렬 적용 (회전 성분만 추출해서 곱함)
	float3x3 rotationMatrix = (float3x3)m; // 애니메이션 매트릭스의 회전 부분만 사용
	output.normal = normalize(mul(input.normal, rotationMatrix));
	output.normal = normalize(mul(output.normal, (float3x3)worldMatrix));

	output.tangent = mul(input.tangent, rotationMatrix); // 탄젠트 변환
	output.tangent = mul(output.tangent, (float3x3)worldMatrix); // 월드 변환

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
//// 간단히 (T, R, S) 구조체 정의
/////////////////////////////////////////
//struct TRS
//{
//	float3 T;       // Translation
//	float4 Q;       // Rotation(Quaternion)
//	float3 S;       // Scale
//};
//
//// 회전 행렬 -> 쿼터니언 변환 
//// (스탠더드 참고용, 정확도/성능 최적화 가능)
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
//		// 축 중 가장 큰 대각원소를 기준으로
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
//// 행렬(4x4) -> (T, R, S) 분해
/////////////////////////////////////////
//TRS DecomposeMatrix(matrix m)
//{
//	TRS outTRS;
//
//	// 1) Translation 추출
//	// m[3].xyz (HLSL에서 row_major 기준일 때 주의)
//	float4 c3 = float4(m[3][0], m[3][1], m[3][2], m[3][3]);
//	outTRS.T = c3.xyz;
//
//	// 2) 스케일 추출
//	// 상단 3x3에서 각각의 축 길이를 구함
//	float3x3 rot3x3 = (float3x3)m;
//	float3 scale;
//	scale.x = length(rot3x3[0]);
//	scale.y = length(rot3x3[1]);
//	scale.z = length(rot3x3[2]);
//	outTRS.S = scale;
//
//	// 3) 회전 행렬 추출 (스케일 제거)
//	// rot3x3[i] /= scale; 로 나눠서 순수 회전만 남김
//	if (scale.x != 0) rot3x3[0] /= scale.x;
//	if (scale.y != 0) rot3x3[1] /= scale.y;
//	if (scale.z != 0) rot3x3[2] /= scale.z;
//
//	// 4) 회전 행렬 -> 쿼터니언 변환
//	outTRS.Q = QuaternionFromRotationMatrix(rot3x3);
//
//	return outTRS;
//}
//
//// 쿼터니언 -> 회전 행렬
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
//// (T, R, S) -> 행렬(4x4) 합성
////////////////////////////////////////////
//matrix ComposeMatrix(TRS t)
//{
//	// 회전 쿼터니언 -> 3x3
//	float3x3 r = RotationMatrixFromQuaternion(t.Q);
//
//	// 스케일 적용
//	r[0] *= t.S.x;
//	r[1] *= t.S.y;
//	r[2] *= t.S.z;
//
//	// 4x4 행렬에 삽입
//	matrix M = (matrix)0;
//	// row_major라고 가정
//	M[0] = float4(r[0].x, r[0].y, r[0].z, 0);
//	M[1] = float4(r[1].x, r[1].y, r[1].z, 0);
//	M[2] = float4(r[2].x, r[2].y, r[2].z, 0);
//	M[3] = float4(t.T.x, t.T.y, t.T.z, 1);
//
//	return M;
//}
//
///////////////////////////////////////////
//// 쿼터니언 slerp (간단 버전)
///////////////////////////////////////////
//float4 QuaternionSlerp(float4 q1, float4 q2, float t)
//{
//	// (상세 구현은 생략 또는 간단화)
//	// 보통 slerp 구현 시 dot 체크, 역으로 돌려주기 등 처리 필요
//	float dotVal = dot(q1, q2);
//
//	// 음수 dot이면 뒤집어줌(더 짧은 쪽으로 회전)
//	float4 q2Fix = (dotVal < 0) ? -q2 : q2;
//	if (dotVal < 0) dotVal = -dotVal;
//
//	// 임계값
//	const float THRESHOLD = 0.9995;
//	if (dotVal > THRESHOLD)
//	{
//		// 선형보간(Fast path)
//		float4 result = lerp(q1, q2Fix, t);
//		return normalize(result);
//	}
//
//	// 진짜 slerp
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
//// TRS끼리 보간
////  - T,S 는 선형
////  - R 은 쿼터니언 slerp
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
//// 본 행렬을 읽어오는 부분 (기존 코드에서 Load)
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
//		// 1) 현재 애니메이션(Anim0) 행렬 보간
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
//		// 2) 다음 애니메이션(Anim1) 행렬 보간
//		//////////////////////////////////////////
//		// animIndex[1]가 유효(>= 0)일 때만
//		TRS resultB = resultA; // default 는 anim1이 없으면 그냥 anim0
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
//			// 트랜지션 블렌딩
//			resultB = LerpTRS(resultA, anim1TRS, blendFrames.blendRatio);
//		}
//
//		//////////////////////////////////////////
//		// 3) 최종 행렬 합성 & 가중치 누적
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
//		m = GetAnimationMatrix(input); // 애니메이션 적용
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
//	// 정점 변환
//	output.position = mul(input.position, m);
//	output.position = mul(output.position, worldMatrix);
//
//	// light space 변환
//	output.lightPosition = mul(output.position, light_viewMatrix);
//	output.lightPosition = mul(output.lightPosition, light_projectionMatrix);
//
//	output.worldPosition = output.position.xyz;
//	output.position = mul(output.position, viewMatrix);
//	output.position = mul(output.position, projectionMatrix);
//	output.uv = input.uv;
//
//	// 노멀과 탄젠트에 애니메이션 행렬 적용 (회전 성분만 추출해서 곱함)
//	float3x3 rotationMatrix = (float3x3)m; // 애니메이션 매트릭스의 회전 부분만 사용
//	output.normal = normalize(mul(input.normal, rotationMatrix));
//	output.normal = normalize(mul(output.normal, (float3x3)worldMatrix));
//
//	output.tangent = mul(input.tangent, rotationMatrix); // 탄젠트 변환
//	output.tangent = mul(output.tangent, (float3x3)worldMatrix); // 월드 변환
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