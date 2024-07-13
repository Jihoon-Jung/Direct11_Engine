

struct VS_INPUT
{
	float4 position : POSITION;
	float2 uv : TEXCOORD;
	float2 scale : SCALE;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
    float4 pos : POS;
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
cbuffer CameraPos : register(b2)
{
	float3 cameraPosition;
	float padding;
}

SamplerState sampler0 : register(s0);
Texture2D texture0 : register(t0);

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;

	float4 position = mul(input.position, worldMatrix);

	float3 up = float3(0, 1, 0);
	float3 forward = position.xyz - cameraPosition; // BillBoard
	float3 right = normalize(cross(up, forward));

	position.xyz += (input.uv.x - 0.5f) * right * input.scale.x;
	position.xyz += (1.0f - input.uv.y - 0.5f) * up * input.scale.y;
	position.w = 1.0f;

	output.position = mul(mul(position, viewMatrix), projectionMatrix);

	output.uv = input.uv;

	return output;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
	float4 textureColor = texture0.Sample(sampler0, input.uv);

	return textureColor;
}							 
