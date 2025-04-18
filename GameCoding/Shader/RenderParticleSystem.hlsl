#define PT_EMITTER 0
#define PT_FLARE 1

cbuffer ParticleBuffer : register(b0)
{
    float3 gEyePosW; // GS
    float padding1;        // x
    float3 gEmitPosW;      // x
    float padding2;        // x
    float3 gEmitDirW;      // x
    float padding3;        // x
    float gGameTime;       // x
    float gTimeStep;       // x
    float gEndParticle;
    float padding4;
    row_major float4x4 gView;    // x
    row_major float4x4 gProj;
}

cbuffer TransformBuffer : register(b1)
{
    row_major matrix worldMatrix;
    row_major matrix worldInvTranspose;
}

//cbuffer CameraBuffer : register(b0)
//{
//    row_major matrix viewMatrix;
//    row_major matrix projectionMatrix;
//}

cbuffer cbFixed
{
    // Net constant acceleration used to accelerate the particles.
    static const float3 gAccelW = { 0.0f, 7.8f, 0.0f };

    // Texture coordinates used to stretch texture over quad 
    // when we expand point particle into a quad.
    static const float2 gQuadTexC[4] =
    {
        float2(0.0f, 1.0f),
        float2(1.0f, 1.0f),
        float2(0.0f, 0.0f),
        float2(1.0f, 0.0f)
    };
}

// 구조체 정의
struct Particle
{
    float3 InitialPosW : POSITION;
    float3 InitialVelW : VELOCITY;
    float2 SizeW       : SIZE;
    float  Age         : AGE;
    uint   Type        : TYPE;
};

struct VertexOut
{
    float3 PosW  : POSITION;
    float2 SizeW : SIZE;
    float4 Color : COLOR;
    uint   Type  : TYPE;
};

// Vertex Shader
VertexOut VS(Particle vin)
{
    VertexOut vout;

    float t = vin.Age;
    vout.PosW = 0.5f * t * t * gAccelW + t * vin.InitialVelW + vin.InitialPosW;

    float opacity = 1.0f - smoothstep(0.0f, 1.0f, t / 1.0f);
    vout.Color = float4(1.0f, 1.0f, 1.0f, opacity);

    vout.SizeW = vin.SizeW;
    vout.Type = vin.Type;

    return vout;
}

struct GeoOut
{
    float4 PosH  : SV_Position;
    float4 Color : COLOR;
    float2 Tex   : TEXCOORD;
};

// Geometry Shader
[maxvertexcount(4)]
void GS(point VertexOut gin[1], inout TriangleStream<GeoOut> triStream)
{
    // do not draw emitter particles.
    if (gin[0].Type != PT_EMITTER)
    {
        float4 worldPosW = mul(float4(gin[0].PosW, 1.0f), worldMatrix);
        //
        // Compute world matrix so that billboard faces the camera.
        //
        float3 look = normalize(gEyePosW - worldPosW.xyz);
        float3 right = normalize(cross(float3(0, 1, 0), look));
        float3 up = cross(look, right);

        //
        // Compute triangle strip vertices (quad) in world space.
        //
        float halfWidth = 0.5f * gin[0].SizeW.x;
        float halfHeight = 0.5f * gin[0].SizeW.y;

        float4 v[4];
        v[0] = float4(worldPosW.xyz + halfWidth * right - halfHeight * up, 1.0f);
        v[1] = float4(worldPosW.xyz + halfWidth * right + halfHeight * up, 1.0f);
        v[2] = float4(worldPosW.xyz - halfWidth * right - halfHeight * up, 1.0f);
        v[3] = float4(worldPosW.xyz - halfWidth * right + halfHeight * up, 1.0f);

        //
        // Transform quad vertices to world space and output 
        // them as a triangle strip.
        //
        GeoOut gout;
        [unroll]
        for (int i = 0; i < 4; ++i)
        {
            float4 viewPos = mul(v[i], gView);
            gout.PosH = mul(viewPos, gProj);// mul(viewPos, projectionMatrix);
            gout.Tex = gQuadTexC[i];
            gout.Color = gin[0].Color;
            triStream.Append(gout);
        }
    }
}


// Pixel Shader
Texture2D gTexArray : register(t0);
SamplerState gSampler : register(s0);

float4 PS(GeoOut  pin) : SV_TARGET
{
    float4 texColor = gTexArray.Sample(gSampler, pin.Tex) * pin.Color;
    return texColor * pin.Color;
}
