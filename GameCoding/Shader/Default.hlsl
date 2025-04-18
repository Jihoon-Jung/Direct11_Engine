#include "LightHelper.hlsl"

struct VS_INPUT
{
	float4 position : POSITION;    // => Slot 0 (PER_VERTEX)
	float2 uv       : TEXCOORD;    // => Slot 0 (PER_VERTEX)
	float3 normal   : NORMAL;      // => Slot 0 (PER_VERTEX)
	float3 tangent  : TANGENT;     // => Slot 0 (PER_VERTEX)
	float4 blendIndices : BLENDINDICES; // => Slot 0
	float4 blendWeights : BLENDWEIGHTS; // => Slot 0

	// 인스턴싱 데이터 => Slot 1 (PER_INSTANCE)
	uint instanceID : SV_INSTANCEID;
	row_major matrix world : INST;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float4 lightSpacePosition : LIGHT_POSITION;
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

cbuffer LightSpaceTransformBuffer : register(b5)
{
	row_major matrix light_viewMatrix;
	row_major matrix light_projectionMatrix;
};

cbuffer CheckInstancingObject : register(b6)
{
	float isInstancing;
	float padding3;
	float padding4;
	float padding5;
}

SamplerState sampler0 : register(s0);
SamplerState shadowSampler : register(s1);
Texture2D texture0 : register(t0);
Texture2D normalMap : register(t1);
Texture2D specularMap : register(t2);
Texture2D diffuseMap : register(t3);
Texture2D shadowMap : register(t4);

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;

	matrix world;
	
	if (isInstancing > 0.0)
		world = input.world;
	else
		world = worldMatrix;

	output.position = mul(input.position, world);
	output.worldPosition = output.position.xyz;
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	output.lightSpacePosition = mul(input.position, world);
	output.lightSpacePosition = mul(output.lightSpacePosition, light_viewMatrix);
	output.lightSpacePosition = mul(output.lightSpacePosition, light_projectionMatrix);

	output.uv = input.uv;
	output.normal = mul(input.normal, (float3x3)world);
	output.tangent = mul(input.tangent, (float3x3)world);

	return output;
}


float4 PS(VS_OUTPUT input) : SV_Target
{
	float3 lightDirection = normalize(lightPosition);
	float3 viewDirection = normalize(cameraPosition - input.worldPosition);
	float3 normal = normalize(input.normal);
	float4 textureColor = texture0.Sample(sampler0, input.uv);

	//ComputeNormalMapping(normal, input.tangent, input.uv, normalMap, sampler0);
	float3 n = normalMap.Sample(sampler0, input.uv).rgb;
	float3 bumpedNormal = NormalSampleToWorldSpace(n, input.normal, input.tangent);

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

	ComputeDirectionalLight(mat, light, bumpedNormal, viewDirection, textureColor, shadowFactor);

	return textureColor;
}							 
