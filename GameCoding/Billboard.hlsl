

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
cbuffer LightAndCameraPos : register(b2)
{
	float3 lightPosition;
	float padding1;
	float3 cameraPosition;
	float padding2;
}

SamplerState sampler0 : register(s0);
Texture2D texture0 : register(t0);

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;

	output.position = mul(input.position, worldMatrix);

	float3 look = normalize(cameraPosition - output.position);
	float3 up = float3(0.0, 1.0, 0.0);
	float3 right = normalize(cross(up, look));
	up = normalize(cross(look, right));

	float3x3 rotationMatrix = float3x3(right, up, look);

	float3 rotatedPosition = mul(rotationMatrix, input.position);
	rotatedPosition += output.position.xyz;

	output.worldPosition = output.position.xyz;
	output.position = mul(float4(rotatedPosition, 1.0), viewMatrix);
	output.position = mul(output.position, projectionMatrix);
	output.uv = input.uv;

	output.normal = mul(input.normal, (float3x3)worldMatrix);
	output.tangent = mul(input.tangent, (float3x3)worldMatrix);

	return output;
}


float4 PS(VS_OUTPUT input) : SV_Target
{
	float4 textureColor = texture0.Sample(sampler0, input.uv);

	return textureColor;
}							 
