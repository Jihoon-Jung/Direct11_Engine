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
};

cbuffer CameraBuffer : register(b0)
{
	row_major matrix viewMatrix;
	row_major matrix projectionMatrix;
}

cbuffer TransformBuffer : register(b1)
{
	row_major matrix worldMatrix;
	row_major matrix inverseTransposeWorldMatrix;
}

cbuffer CheckInstancingObject : register(b2)
{
	float isInstancing;
	float padding3;
	float padding4;
	float padding5;
}

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;

	matrix world;

	if (isInstancing > 0.0)
		world = input.world;
	else
		world = worldMatrix;

	output.position = mul(input.position, world);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	output.uv = input.uv;


	return output;
}

SamplerState sampler0 : register(s0);
Texture2D texture0 : register(t0);

float4 PS(VS_OUTPUT input) : SV_Target
{
	return texture0.Sample(sampler0, input.uv);

}