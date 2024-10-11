#include "LightHelper.hlsl"

struct VertexIn
{
    float3 PosL     : POSITION;
    float3 NormalL  : NORMAL;
    float2 Tex      : TEXCOORD;
    float3 TangentL : TANGENT;
    float4 blendIndices : BLENDINDICES;
    float4 blendWeights : BLENDWEIGHTS;
};

struct VertexOut
{
    float3 PosW       : POSITION;
    float3 NormalW    : NORMAL;
    float3 TangentW   : TANGENT;
    float2 Tex        : TEXCOORD;
    float  TessFactor : TESS;
};

cbuffer CameraBuffer : register(b0)
{
    row_major matrix viewMatrix;
    row_major matrix projectionMatrix;
}

cbuffer TransformBuffer : register(b1)
{
    row_major matrix worldMatrix;
    row_major matrix worldInvTranspose;
}
cbuffer LightMaterial : register(b2)
{
    float4 materialDiffuse;
    float4 materialAmbient;
    float4 materialSpecular;
    float4 materialEmissive;
}
cbuffer LightDesc : register(b3)
{
    float4 ambient;
    float4 diffuse;
    float4 specular;
    float4 emissive;

}
cbuffer LightAndCameraPos : register(b4)
{
    float3 lightPosition;
    float padding1;
    float3 cameraPosition;
    float padding2;
}

SamplerState sampler0 : register(s0);
Texture2D texture0 : register(t0);
Texture2D normalMap : register(t1);
Texture2D specularMap : register(t2);
Texture2D diffuseMap : register(t3);


// Vertex Shader (VS)
VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    // Transform to world space space.
    vout.PosW = mul(float4(vin.PosL, 1.0f), worldMatrix).xyz;
    vout.NormalW = mul(vin.NormalL, (float3x3)worldInvTranspose);
    vout.TangentW = mul(vin.TangentL, (float3x3)worldMatrix);

    // Output vertex attributes for interpolation across triangle.
    float scaleU = 2.0f;
    float scaleV = 2.0f;
    float4x4 texTransform = float4x4(
        scaleU, 0.0f, 0.0f, 0.0f,
        0.0f, scaleV, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
        );
    vout.Tex = vin.Tex;// mul(float4(vin.Tex, 0.0f, 1.0f), texTransform).xy;

    float d = distance(vout.PosW, cameraPosition);

    // Normalized tessellation factor. 
    // The tessellation is 
    //   0 if d >= 25.0 and
    //   1 if d <= 1.0.  
    float tess = saturate((25.0 - d) / (25.0 - 1.0));

    // Rescale [0,1] --> [gMinTessFactor, gMaxTessFactor].
    vout.TessFactor = 1.0 + tess * (5.0 - 1.0);

    return vout;
}


struct PatchTess
{
    float EdgeTess[3] : SV_TessFactor;
    float InsideTess : SV_InsideTessFactor;
};

PatchTess PatchHS(InputPatch<VertexOut, 3> patch,
    uint patchID : SV_PrimitiveID)
{
    PatchTess pt;

    // Average tess factors along edges, and pick an edge tess factor for 
    // the interior tessellation.  It is important to do the tess factor
    // calculation based on the edge properties so that edges shared by 
    // more than one triangle will have the same tessellation factor.  
    // Otherwise, gaps can appear.
    pt.EdgeTess[0] = 0.5f * (patch[1].TessFactor + patch[2].TessFactor);
    pt.EdgeTess[1] = 0.5f * (patch[2].TessFactor + patch[0].TessFactor);
    pt.EdgeTess[2] = 0.5f * (patch[0].TessFactor + patch[1].TessFactor);
    pt.InsideTess = pt.EdgeTess[0];

    return pt;
}

struct HullOut
{
    float3 PosW     : POSITION;
    float3 NormalW  : NORMAL;
    float3 TangentW : TANGENT;
    float2 Tex      : TEXCOORD;
};

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("PatchHS")]
HullOut HS(InputPatch<VertexOut, 3> p,
    uint i : SV_OutputControlPointID,
    uint patchId : SV_PrimitiveID)
{
    HullOut hout;

    // Pass through shader.
    hout.PosW = p[i].PosW;
    hout.NormalW = p[i].NormalW;
    hout.TangentW = p[i].TangentW;
    hout.Tex = p[i].Tex;

    return hout;
}

struct DomainOut
{
    float4 PosH     : SV_POSITION;
    float3 PosW     : POSITION;
    float3 NormalW  : NORMAL;
    float3 TangentW : TANGENT;
    float2 Tex      : TEXCOORD;
    float3 tmp      : TMP_POSITION;
};

// The domain shader is called for every vertex created by the tessellator.  
// It is like the vertex shader after tessellation.
[domain("tri")]
DomainOut DS(PatchTess patchTess,
    float3 bary : SV_DomainLocation,
    const OutputPatch<HullOut, 3> tri)
{
    DomainOut dout;

    // Interpolate patch attributes to generated vertices.
    dout.PosW = bary.x * tri[0].PosW + bary.y * tri[1].PosW + bary.z * tri[2].PosW;
    dout.NormalW = bary.x * tri[0].NormalW + bary.y * tri[1].NormalW + bary.z * tri[2].NormalW;
    dout.TangentW = bary.x * tri[0].TangentW + bary.y * tri[1].TangentW + bary.z * tri[2].TangentW;
    dout.Tex = bary.x * tri[0].Tex + bary.y * tri[1].Tex + bary.z * tri[2].Tex;

    // Interpolating normal can unnormalize it, so normalize it.
    dout.NormalW = normalize(dout.NormalW);

    //
    // Displacement mapping.
    //

    // Choose the mipmap level based on distance to the eye; specifically, choose
    // the next miplevel every MipInterval units, and clamp the miplevel in [0,6].
    const float MipInterval = 20.0f;
    float mipLevel = clamp((distance(dout.PosW, cameraPosition) - MipInterval) / MipInterval, 0.0f, 6.0f);

    // Sample height map (stored in alpha channel).
    float h = normalMap.SampleLevel(sampler0, dout.Tex, mipLevel).a;

    // Offset vertex along normal.
    float heightScale = 0.07;
    float3 rst = (heightScale * (h - 1.0)) * dout.NormalW;
    dout.tmp = float3(h, h, h);
    dout.PosW += (heightScale * (h - 1.0)) * dout.NormalW;

    // Project to homogeneous clip space.
    dout.PosH = mul(mul(float4(dout.PosW, 1.0f), viewMatrix), projectionMatrix);

    return dout;
}

float4 PS(DomainOut pin) : SV_Target
{
    float4 ambientColor = 0;
    float4 diffuseColor = 0;
    float4 specularColor = 0;
    float4 emissiveColor = 0;

    float3 lightDirection = normalize(pin.PosW - lightPosition);
    float3 viewDirection = normalize(cameraPosition - pin.PosW);
    float3 normal = normalize(pin.NormalW);
    float4 textureColor = texture0.Sample(sampler0, pin.Tex);

    float3 n = normalMap.Sample(sampler0, pin.Tex).rgb;
    float3 bumpedNormal = NormalSampleToWorldSpace(n, pin.NormalW, pin.TangentW);
    //ComputeNormalMapping(normal, pin.TangentW, pin.Tex, normalMap, sampler0);

    Material mat;
    mat.Ambient = materialAmbient;
    mat.Diffuse = materialDiffuse;
    mat.Specular = materialSpecular;

    DirectionalLight light;
    light.Ambient = ambient;
    light.Diffuse = diffuse;
    light.Specular = specular;
    light.Direction = lightDirection;

    ComputeDirectionalLight(mat, light, bumpedNormal, viewDirection, textureColor);
    //// Ambient
    //{
    //    float ambientStrength = 1.0;
    //    float4 color = ambient * materialAmbient * ambientStrength;
    //    ambientColor = textureColor * color;
    //}
    //// Diffuse
    //{
    //    float value = dot(lightDirection, normal);
    //    diffuseColor = textureColor * value * diffuse * materialDiffuse;
    //}
    //// Specular
    //{
    //    float specularStrength = 1.0;

    //    float3 reflectDir = reflect(-lightDirection, normal);
    //    float spec = pow(max(dot(viewDirection, reflectDir), 0.0), 10);
    //    specularColor = spec * specular * materialSpecular * specularStrength;
    //}
    //// Emissive
    //{
    //    float value = saturate(dot(viewDirection, normal));
    //    float e = 1.0f - value;

    //    e = smoothstep(0.0f, 1.0f, e);
    //    e = pow(e, 5);
    //    emissiveColor = materialEmissive * emissive * e;
    //}

    //float4 textureColor = texture0.Sample(sampler0, pin.Tex);
    return textureColor;// float4((textureColor + diffuseColor + specularColor).xyz, 1.0);// textureColor;
}