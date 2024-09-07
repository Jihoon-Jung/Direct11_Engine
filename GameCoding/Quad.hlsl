

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
};

Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;

	output.position = input.position;
	output.uv = input.uv;

	return output;
}


float4 PS(VS_OUTPUT input) : SV_Target
{
	float4 textureColor = texture0.Sample(sampler0, input.uv);

	return textureColor;
}							 
