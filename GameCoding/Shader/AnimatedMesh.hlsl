
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
	float2 padding3;
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
	matrix m = GetAnimationMatrix(input); // 애니메이션 행렬

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