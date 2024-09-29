

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

// SkyBox는 하늘과 카메라가 같이 움직이고 회전시 하늘과 카메라가 반대방향으로 회전.
VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output;

	// world 변환을 하지 않으면 원점을 기준으로 한 좌표가 되고 여기에 view변환을 하면 카메라와 오브젝트 모두 원점에 있게 됨.
	// 대신 View행렬의 이동은 적용하면 안되고 회전만 적용되어야 하기 떄문에 정점의 w를 0으로 하여 곱해주면 이동은 적용되지 않음.
	float4 viewPos = mul(float4(input.position.xyz, 0), viewMatrix);
	float4 clipSpacePos = mul(viewPos, projectionMatrix);
	output.position = clipSpacePos.xyzw;
	// z를 w로 넣으면 Rasterizer단계에서 w로 나누기때문에 z(깊이)는 1로됨.
	// 근데 깊이가 1이면 뭔가 비교해서 1도 그리도록? 하는 작업이 필요하니까
	// 1에 아주 가까운 값으로 해주는게 편함.
	output.position.z = output.position.w * 0.999999f;

	output.PosL = input.position.xyz;
	return output;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
	
	float4 textureColor = texture0.Sample(sampler0, input.PosL);

	return textureColor;
}							 
