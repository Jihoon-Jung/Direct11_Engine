cbuffer ParticleBuffer : register(b0)
{
    float3 gEyePosW;
    float  padding1;
    float3 gEmitPosW;
    float  padding2;
    float3 gEmitDirW;
    float  padding3;
    float  gGameTime;
    float  gTimeStep;
    float  gEndParticle; // 0보다 크면 폭발 반복
    float  padding4;
    float4x4 gView;
    float4x4 gProj;
};

// 구조체 정의
struct Particle
{
    float3 InitialPosW : POSITION;
    float3 InitialVelW : VELOCITY;
    float2 SizeW       : SIZE;
    float  Age : AGE;
    uint   Type        : TYPE;
};

Texture1D gRandomTex : register(t0);
SamplerState gSampler : register(s0);

float3 RandUnitVec3(float offset)
{
    float u = (gGameTime + offset);
    float3 v = gRandomTex.SampleLevel(gSampler, u, 0).xyz;
    return normalize(v);
}

// -------------------------
// VS: 그대로 통과
// -------------------------
Particle VS(Particle vin)
{
    return vin;
}

#define PT_EMITTER 0
#define PT_FLARE   1

// -------------------------
// GS: 폭발 주기적으로 반복
// -------------------------

// 한 번 폭발 시 최대 30개 새 파티클 + Emitter 본인 1개 = 31
[maxvertexcount(31)]
void GS(point Particle gin[1], inout PointStream<Particle> ptStream)
{
    // 매 프레임 나이 증가
    gin[0].Age += gTimeStep;

    if (gin[0].Type == PT_EMITTER)
    {
        // gEndParticle > 0이면 폭발 로직 수행
        if (gEndParticle > 0.0f)
        {
            // "조금 텀을 두고" 반복 → Emitter Age가 1초 지날 때마다 폭발 발생
            //   -> 폭발 후 Age를 0으로 리셋 = 다음 1초 뒤에 재폭발
            if (gin[0].Age > 1.0f)
            {
                // 예: 파티클 30개를 한번에 뿌림
                for (int i = 0; i < 30; i++)
                {
                    // 전방위 랜덤 방향
                    float3 vRandom = RandUnitVec3(i * 10.17f);

                    // 속도 조금 랜덤: 기본 4~6 사이
                    // vRandom.y (혹은 다른 성분) 이용해서 가감 가능
                    float speedRand = 4.0f + 2.0f * abs(vRandom.y);

                    Particle p;
                    p.InitialPosW = gEmitPosW; // 폭발 중심
                    p.InitialVelW = speedRand * vRandom;
                    p.SizeW = float2(3.0f, 3.0f);
                    p.Age = 0.0f;
                    p.Type = PT_FLARE;

                    ptStream.Append(p);
                }

                // 폭발 후 Emitter Age 초기화
                gin[0].Age = 0.0f;
            }
        }

        // Emitter는 계속 유지(화면에는 안 그리지만 시스템 내부적으로 존재)
        ptStream.Append(gin[0]);
    }
    else
    {
        // Flare 파티클: 원본 코드처럼 1초까지만 유지
        // (원하면 2초, 3초 등 조절 가능)
        if (gin[0].Age <= 1.0f)
        {
            ptStream.Append(gin[0]);
        }
    }
}
