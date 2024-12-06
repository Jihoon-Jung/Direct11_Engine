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
	float4 lightSpacePosition : LIGHT_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 worldPosition : POSITION1;
	float3 PosL : POSITION;
	float3 worldNormal : NORMAL1; // 추가된 출력
};

cbuffer CameraBuffer : register(b0)
{
	row_major matrix viewMatrix;
	row_major matrix projectionMatrix;
}

cbuffer TransformBuffer : register(b1)
{
	row_major matrix worldMatrix;
	row_major matrix worldInvTranspose;
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
cbuffer LightSpaceTransformBuffer : register(b5)
{
	row_major matrix light_viewMatrix;
	row_major matrix light_projectionMatrix;
};

SamplerState sampler0 : register(s0);
SamplerState shadowSampler : register(s1);
TextureCube texture0 : register(t0);
Texture2D normalMap : register(t1);
Texture2D specularMap : register(t2);
Texture2D diffuseMap : register(t3);
Texture2D shadowMap : register(t4);

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;

	// 월드 스페이스로 변환
	output.position = mul(float4(input.position.xyz, 1.0f), worldMatrix);
	output.worldPosition = output.position.xyz;
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	output.uv = input.uv;

	output.lightSpacePosition = mul(input.position, worldMatrix);
	output.lightSpacePosition = mul(output.lightSpacePosition, light_viewMatrix);
	output.lightSpacePosition = mul(output.lightSpacePosition, light_projectionMatrix);

	// 월드 스페이스 법선 계산
	output.normal = normalize(mul(input.normal, (float3x3)worldInvTranspose));
	output.tangent = mul(input.tangent, (float3x3)worldMatrix);
	output.PosL = input.position.xyz;

	// 추가: 월드 스페이스 법선 벡터 전달
	output.worldNormal = normalize(mul(input.normal, (float3x3)worldInvTranspose));

	return output;
}


void ComputeNormalMapping(inout float3 normal, float3 tangent, float2 uv)
{
	// [0,255] 범위에서 [0,1]로 변환
	float4 map = normalMap.Sample(sampler0, uv);
	if (any(map.rgb) == false)
		return;

	float3 N = normalize(normal); // z
	float3 T = normalize(tangent); // x
	float3 B = normalize(cross(N, T)); // y
	float3x3 TBN = float3x3(T, B, N); // TS -> WS

	// [0,1] 범위에서 [-1,1] 범위로 변환
	float3 tangentSpaceNormal = (map.rgb * 2.0f - 1.0f);
	float3 worldNormal = mul(tangentSpaceNormal, TBN);

	normal = worldNormal;
}
float4 PS(VS_OUTPUT input) : SV_Target
{
	float4 ambientColor = 0;
	float4 diffuseColor = 0;
	float4 specularColor = 0;
	float4 emissiveColor = 0;

	float3 lightDirection = normalize(lightPosition);
	float3 viewDirection = normalize(cameraPosition - input.worldPosition);
	float3 normal = normalize(input.normal);
	float4 textureColor;// = texture0.Sample(sampler0, input.PosL);

	/*float3 n = normalMap.Sample(sampler0, input.uv).rgb;
	float3 bumpedNormal = NormalSampleToWorldSpace(n, input.normal, input.tangent);*/

	//ComputeNormalMapping(normal, input.tangent, input.uv);

	// 카메라 위치에서 픽셀 위치로의 뷰 벡터 계산 (정규화)
	float3 viewDir = normalize(cameraPosition - input.worldPosition);

	// 반사 벡터 계산
	float3 reflectionVector = reflect(-viewDir, normal);

	// 반사 벡터 정규화 (정규화 필요할 경우)
	reflectionVector = normalize(reflectionVector);

	// 큐브 맵 텍스처 샘플링
	textureColor = texture0.Sample(sampler0, reflectionVector);

	Material mat;
	mat.Ambient = materialAmbient;
	mat.Diffuse = materialDiffuse;
	mat.Specular = materialSpecular;

	DirectionalLight light;
	light.Ambient = ambient;
	light.Diffuse = diffuse;
	light.Specular = specular;
	light.Direction = lightDirection;

	Shadow shadow;
	shadow.lightPosition = input.lightSpacePosition;
	shadow.shadowSampler = shadowSampler;
	shadow.shadowMap = shadowMap;
	shadow.normal = normal;
	shadow.lightDir = -lightDirection;
	float shadowFactor = CalculateShadowFactor(shadow);

	ComputeDirectionalLight(mat, light, normal, viewDirection, textureColor, shadowFactor);

	return textureColor;

}							 
