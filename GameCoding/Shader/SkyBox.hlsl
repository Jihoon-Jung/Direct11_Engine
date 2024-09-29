

struct VS_INPUT
{
	float4 position : POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float4 blendIndices : BLENDINDICES;
	float4 blendWeights : BLENDWEIGHTS;
};

//struct VS_OUTPUT
//{
//	float4 position : SV_POSITION;
//	float2 uv : TEXCOORD;
//	float3 normal : NORMAL;
//	float3 tangent : TANGENT;
//	float3 worldPosition : POSITION1;
//};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float3 PosL : POSITION;
};

cbuffer CameraBuffer : register(b0)
{
	row_major matrix viewMatrix;
	row_major matrix projectionMatrix;
}


SamplerState sampler0 : register(s0);
//Texture2D texture0 : register(t0);
TextureCube texture0 : register(t0);

// SkyBox�� �ϴð� ī�޶� ���� �����̰� ȸ���� �ϴð� ī�޶� �ݴ�������� ȸ��.
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;

	// world ��ȯ�� ���� ������ ������ �������� �� ��ǥ�� �ǰ� ���⿡ view��ȯ�� �ϸ� ī�޶�� ������Ʈ ��� ������ �ְ� ��.
	// ��� View����� �̵��� �����ϸ� �ȵǰ� ȸ���� ����Ǿ�� �ϱ� ������ ������ w�� 0���� �Ͽ� �����ָ� �̵��� ������� ����.
	float4 viewPos = mul(float4(input.position.xyz, 0), viewMatrix);
	float4 clipSpacePos = mul(viewPos, projectionMatrix);
	output.position = clipSpacePos.xyzw;
	// z�� w�� ������ Rasterizer�ܰ迡�� w�� �����⶧���� z(����)�� 1�ε�.
	// �ٵ� ���̰� 1�̸� ���� ���ؼ� 1�� �׸�����? �ϴ� �۾��� �ʿ��ϴϱ�
	// 1�� ���� ����� ������ ���ִ°� ����.
	output.position.z = output.position.w * 0.999999f;

	output.PosL = input.position.xyz;
	return output;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
	
	float4 textureColor = texture0.Sample(sampler0, input.PosL);

	return textureColor;
}							 
