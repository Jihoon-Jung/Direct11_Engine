

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


SamplerState sampler0 : register(s0);
Texture2D texture0 : register(t0);

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;

	// Local -> World -> View -> Projection
	// ����� ���������� 0���� ����ϸ� translation�� ��������ʰ�
	// ȸ��, scale�� �����. ī�޶� �����̸� skyBox�� (�ݴ��)ȸ���ؾ��ϴ�
	float4 viewPos = mul(float4(input.position.xyz, 0), viewMatrix);
	float4 clipSpacePos = mul(viewPos, projectionMatrix);
	output.position = clipSpacePos.xyzw;
	// z�� w�� ������ Rasterizer�ܰ迡�� w�� �����⶧���� z(����)�� 1�ε�.
	// �ٵ� ���̰� 1�̸� ���� ���ؼ� 1�� �׸�����? �ϴ� �۾��� �ʿ��ϴϱ�
	// 1�� ���� ����� ������ ���ִ°� ����.
	output.position.z = output.position.w * 0.999999f;

	output.uv = input.uv;

	return output;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
	
	float4 textureColor = texture0.Sample(sampler0, input.uv);

	return textureColor;
}							 
