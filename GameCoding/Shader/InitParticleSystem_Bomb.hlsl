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
    float  gEndParticle; // 0���� ũ�� ���� �ݺ�
    float  padding4;
    float4x4 gView;
    float4x4 gProj;
};

// ����ü ����
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
// VS: �״�� ���
// -------------------------
Particle VS(Particle vin)
{
    return vin;
}

#define PT_EMITTER 0
#define PT_FLARE   1

// -------------------------
// GS: ���� �ֱ������� �ݺ�
// -------------------------

// �� �� ���� �� �ִ� 30�� �� ��ƼŬ + Emitter ���� 1�� = 31
[maxvertexcount(31)]
void GS(point Particle gin[1], inout PointStream<Particle> ptStream)
{
    // �� ������ ���� ����
    gin[0].Age += gTimeStep;

    if (gin[0].Type == PT_EMITTER)
    {
        // gEndParticle > 0�̸� ���� ���� ����
        if (gEndParticle > 0.0f)
        {
            // "���� ���� �ΰ�" �ݺ� �� Emitter Age�� 1�� ���� ������ ���� �߻�
            //   -> ���� �� Age�� 0���� ���� = ���� 1�� �ڿ� ������
            if (gin[0].Age > 1.0f)
            {
                // ��: ��ƼŬ 30���� �ѹ��� �Ѹ�
                for (int i = 0; i < 30; i++)
                {
                    // ������ ���� ����
                    float3 vRandom = RandUnitVec3(i * 10.17f);

                    // �ӵ� ���� ����: �⺻ 4~6 ����
                    // vRandom.y (Ȥ�� �ٸ� ����) �̿��ؼ� ���� ����
                    float speedRand = 4.0f + 2.0f * abs(vRandom.y);

                    Particle p;
                    p.InitialPosW = gEmitPosW; // ���� �߽�
                    p.InitialVelW = speedRand * vRandom;
                    p.SizeW = float2(3.0f, 3.0f);
                    p.Age = 0.0f;
                    p.Type = PT_FLARE;

                    ptStream.Append(p);
                }

                // ���� �� Emitter Age �ʱ�ȭ
                gin[0].Age = 0.0f;
            }
        }

        // Emitter�� ��� ����(ȭ�鿡�� �� �׸����� �ý��� ���������� ����)
        ptStream.Append(gin[0]);
    }
    else
    {
        // Flare ��ƼŬ: ���� �ڵ�ó�� 1�ʱ����� ����
        // (���ϸ� 2��, 3�� �� ���� ����)
        if (gin[0].Age <= 1.0f)
        {
            ptStream.Append(gin[0]);
        }
    }
}
