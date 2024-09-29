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

struct VS_INPUT
{
    float4 position : POSITION;
};

struct VS_OUTPUT
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct GS_OUTPUT
{
    float4 Pos : SV_POSITION;
    float3 Normal : NORMAL;
    float2 UV : TEXCOORD;
};

SamplerState sampler0 : register(s0);
Texture2D texture0 : register(t0);

VS_OUTPUT VS(VS_INPUT input)
{
    VS_OUTPUT output;

    float4 worldPosition = input.position;
    output.position = worldPosition;

    // 기본 노멀 벡터 (위쪽 방향으로 설정)
    output.normal = float3(0.0f, 1.0f, 0.0f);

    // 기본 텍스처 좌표 설정
    output.uv = float2(0.0f, 0.0f);

    return output;
}

[maxvertexcount(4)]
void GS(point VS_OUTPUT input[1], inout TriangleStream<GS_OUTPUT> triStream)
{
    float size = 1.0f;

    float3 up = float3(0.0f, 1.0f, 0.0f);
    float3 look = cameraPosition - input[0].position.xyz;
    look.y = 0.0f; // y-axis aligned, so project to xz-plane
    look = normalize(look);
    float3 right = cross(up, look);

    float halfWidth = 0.5f * size;
    float halfHeight = 0.5f * size;

    float4 v[4];
    v[0] = float4(input[0].position.xyz + halfWidth * right - halfHeight * up, 1.0f);
    v[1] = float4(input[0].position.xyz + halfWidth * right + halfHeight * up, 1.0f);
    v[2] = float4(input[0].position.xyz - halfWidth * right - halfHeight * up, 1.0f);
    v[3] = float4(input[0].position.xyz - halfWidth * right + halfHeight * up, 1.0f);

    float2 uv[4] = {
        float2(1.0f, 1.0f),
        float2(1.0f, 0.0f),
        float2(0.0f, 1.0f),
        float2(0.0f, 0.0f)
    };

    GS_OUTPUT output;
    for (int i = 0; i < 4; ++i)
    {
        float4 viewPos = mul(v[i], viewMatrix);
        output.Pos = mul(viewPos, projectionMatrix);
        output.Normal = input[0].normal;
        output.UV = uv[i];
        triStream.Append(output);
    }
}

float4 PS(GS_OUTPUT input) : SV_TARGET
{
    float4 textureColor = texture0.Sample(sampler0, input.UV);
    clip(textureColor.a - 0.05f);
    return textureColor;
}
