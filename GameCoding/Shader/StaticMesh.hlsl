#define MAX_MODEL_TRANSFORMS 250

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

cbuffer BoneBuffer : register(b5)
{
	row_major matrix BoneTransforms[MAX_MODEL_TRANSFORMS];
};
cbuffer BonIndex : register(b6)
{
	uint BoneIndex;
	float3 padding;
}

SamplerState sampler0 : register(s0);
Texture2D texture0 : register(t0);
Texture2D normalMap : register(t1);
Texture2D specularMap : register(t2);
Texture2D diffuseMap : register(t3);


VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;

	output.position = mul(input.position, BoneTransforms[BoneIndex]);
	output.position = mul(output.position, worldMatrix);
	output.worldPosition = output.position.xyz;
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	output.uv = input.uv;

	output.normal = mul(input.normal, (float3x3)worldMatrix);
	output.tangent = mul(input.tangent, (float3x3)worldMatrix);

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

	float3 lightDirection = normalize(lightPosition - input.worldPosition);
	float3 viewDirection = normalize(cameraPosition - input.worldPosition);
	float4 color = texture0.Sample(sampler0, input.uv);
	float3 normal = normalize(input.normal);
	float4 textureColor = diffuseMap.Sample(sampler0, input.uv);

	ComputeNormalMapping(normal, input.tangent, input.uv);

	// Ambient
	{
		float ambientStrength = 1.0;
		float4 color = ambient * materialAmbient * ambientStrength;
		ambientColor = textureColor * color;
	}
	// Diffuse
	{
		float value = dot(lightDirection, normal);
		diffuseColor = textureColor * value * diffuse * materialDiffuse;
	}
	// Specular
	{
		float specularStrength = 1.0;

		float3 reflectDir = reflect(-lightDirection, normal);
		float spec = pow(max(dot(viewDirection, reflectDir), 0.0), 10);
		specularColor = spec * specular * materialSpecular * specularStrength;
	}
	// Emissive
	{
		float value = saturate(dot(viewDirection, normal));
		float e = 1.0f - value;

		e = smoothstep(0.0f, 1.0f, e);
		e = pow(e, 5);
		emissiveColor = materialEmissive * emissive * e;
	}

	return textureColor;// float4((textureColor + diffuseColor + specularColor).xyz, 1.0);
}
