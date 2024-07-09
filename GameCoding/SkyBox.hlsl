

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
	// 행렬의 마지막행을 0으로 사용하면 translation이 적용되지않고
	// 회전, scale만 적용됨. 카메라가 움직이면 skyBox도 (반대로)회전해야하니
	float4 viewPos = mul(float4(input.position.xyz, 0), viewMatrix);
	float4 clipSpacePos = mul(viewPos, projectionMatrix);
	output.position = clipSpacePos.xyzw;
	// z를 w로 넣으면 Rasterizer단계에서 w로 나누기때문에 z(깊이)는 1로됨.
	// 근데 깊이가 1이면 뭔가 비교해서 1도 그리도록? 하는 작업이 필요하니까
	// 1에 아주 가까운 값으로 해주는게 편함.
	output.position.z = output.position.w * 0.999999f;

	output.uv = input.uv;

	return output;
}

float4 PS(VS_OUTPUT input) : SV_Target
{
	
	float4 textureColor = texture0.Sample(sampler0, input.uv);

	return textureColor;
}							 
