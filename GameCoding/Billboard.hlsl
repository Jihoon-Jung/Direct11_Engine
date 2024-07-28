//
//
//struct VS_INPUT
//{
//	float4 position : POSITION;
//	float2 uv : TEXCOORD;
//	float2 scale : SCALE;
//};
//
//struct VS_OUTPUT
//{
//	float4 position : SV_POSITION;
//	float2 uv : TEXCOORD;
//    float4 pos : POS;
//};
//
//cbuffer CameraBuffer : register(b0)
//{
//	row_major matrix viewMatrix;
//	row_major matrix projectionMatrix;
//}
//
//cbuffer TransformBuffer : register(b1)
//{
//	row_major matrix worldMatrix;
//}
//cbuffer CameraPos : register(b2)
//{
//	float3 cameraPosition;
//	float padding;
//}
//
//SamplerState sampler0 : register(s0);
//Texture2D texture0 : register(t0);
//
//VS_OUTPUT VS(VS_INPUT input)
//{
//	VS_OUTPUT output;
//
//	float4 position = input.position;// mul(input.position, worldMatrix);
//
//	float3 up = float3(0, 1, 0);
//	float3 forward = position.xyz - cameraPosition; // BillBoard
//	float3 right = normalize(cross(up, forward));
//
//	position.xyz += (input.uv.x - 0.5f) * right * input.scale.x;
//	position.xyz += (1.0f - input.uv.y - 0.5f) * up * input.scale.y;
//	position.w = 1.0f;
//
//	output.position = mul(mul(position, viewMatrix), projectionMatrix);
//
//	output.uv = input.uv;
//
//	return output;
//}
//
//float4 PS(VS_OUTPUT input) : SV_Target
//{
//	float4 textureColor = texture0.Sample(sampler0, input.uv);
//
//	return textureColor;
//}							 



struct VS_INPUT
{
	float4 position : POSITION;
	float2 scale : SCALE;
};

struct VS_OUTPUT
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD;
	float3 normal : NORMAL;
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

	position.w = 1.0f;

	output.position = mul(mul(position, viewMatrix), projectionMatrix);

	output.uv = float2(0.0f, 0.0f); // 기본 텍스처 좌표 설정
	output.normal = float3(0.0f, 0.0f, 0.0f);// mul(float4(Normal, 0.0f), worldMatrix).xyz;

	return output;
}

struct GS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float2 Tex : TEXCOORD;
    float3 Normal : NORMAL;
};

[maxvertexcount(6)]
void GS(point VS_OUTPUT input[1], inout TriangleStream<GS_OUTPUT> triStream)
{
    float size = 0.1; // 사각형 크기

    float4 corners[6] =
    {
        float4(input[0].position.x - size, input[0].position.y + size, input[0].position.z, 1.0f), // Top left
        float4(input[0].position.x + size, input[0].position.y + size, input[0].position.z, 1.0f), // Top right
        float4(input[0].position.x - size, input[0].position.y - size, input[0].position.z, 1.0f), // Bottom left
    
        float4(input[0].position.x + size, input[0].position.y + size, input[0].position.z, 1.0f), // Top right
        float4(input[0].position.x - size, input[0].position.y - size, input[0].position.z, 1.0f), // Bottom left
        float4(input[0].position.x + size, input[0].position.y - size, input[0].position.z, 1.0f)  // Bottom right
    };

    float2 texCoords[6] =
    {
        float2(0.0f, 0.0f), // Top left
        float2(1.0f, 0.0f), // Top right
        float2(0.0f, 1.0f), // Bottom left

        float2(1.0f, 0.0f), // Top right
        float2(0.0f, 1.0f), // Bottom left
        float2(1.0f, 1.0f)  // Bottom right
    };

    float3 normal = input[0].normal; // 노말 벡터

    GS_OUTPUT output;
    for (int i = 0; i < 6; ++i)
    {
        output.Pos = corners[i];
        output.Tex = texCoords[i];
        output.Normal = normal;
        triStream.Append(output);
    }
}

float4 PS(GS_OUTPUT input) : SV_Target
{
	float4 textureColor = texture0.Sample(sampler0, input.Tex);

    return float4(1.0, 0.0, 0.0, 1.0);// textureColor;
}
