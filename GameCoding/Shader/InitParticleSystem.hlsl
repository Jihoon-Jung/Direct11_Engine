cbuffer ParticleBuffer : register(b0)
{
    float3 gEyePosW; // x
    float padding1;
    float3 gEmitPosW; // o
    float padding2;
    float3 gEmitDirW; // x
    float padding3;
    float gGameTime; // o
    float gTimeStep; // o
    float padding4;
    float padding5;
	float4x4 gView;
	float4x4 gProj;
};

// 구조체 정의
struct Particle
{
    float3 InitialPosW : POSITION;
    float3 InitialVelW : VELOCITY;
    float2 SizeW       : SIZE;
    float  Age         : AGE;
    uint   Type        : TYPE;
};


Texture1D gRandomTex : register(t0); // GS
SamplerState gSampler : register(s0);

float3 RandUnitVec3(float offset)
{
    float u = (gGameTime + offset);
    float3 v = gRandomTex.SampleLevel(gSampler, u, 0).xyz;
    return normalize(v);
}

// Vertex Shader
Particle VS(Particle vin)
{
    return vin;
}

#define PT_EMITTER 0
#define PT_FLARE 1

// Geometry Shader
[maxvertexcount(2)]
void GS(point Particle gin[1], inout PointStream<Particle> ptStream)
{
	gin[0].Age += gTimeStep;

	if (gin[0].Type == PT_EMITTER)
	{
		// time to emit a new particle?
		if (gin[0].Age > 0.005f)
		{
			float3 vRandom = RandUnitVec3(0.0f);
			vRandom.x *= 0.5f;
			vRandom.z *= 0.5f;

			Particle p;
			p.InitialPosW = gEmitPosW.xyz;
			p.InitialVelW = 4.0f * vRandom;
			p.SizeW = float2(3.0f, 3.0f);
			p.Age = 0.0f;
			p.Type = PT_FLARE;

			ptStream.Append(p);

			// reset the time to emit
			gin[0].Age = 0.0f;
		}

		// always keep emitters
		ptStream.Append(gin[0]);
	}
	else
	{
		// Specify conditions to keep particle; this may vary from system to system.
		if (gin[0].Age <= 1.0f)
			ptStream.Append(gin[0]);
	}
}