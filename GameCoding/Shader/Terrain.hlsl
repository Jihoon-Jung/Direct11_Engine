#include "LightHelper.hlsl"

struct VertexIn
{
	float3 PosL     : POSITION;
	float2 Tex      : TEXCOORD;
	float2 BoundsY  : BOUNDY;
};

struct VertexOut
{
	float3 PosW     : POSITION;
	float2 Tex      : TEXCOORD0;
	float2 BoundsY  : TEXCOORD1;
};

cbuffer CameraBuffer : register(b0)
{
	row_major matrix viewMatrix;
	row_major matrix projectionMatrix;
}

cbuffer LightMaterial : register(b1)
{
	float4 materialDiffuse;
	float4 materialAmbient;
	float4 materialSpecular;
	float4 materialEmissive;
}
cbuffer LightDesc : register(b2)
{
	float4 ambient;
	float4 diffuse;
	float4 specular;
	float4 emissive;

}
cbuffer LightAndCameraPos : register(b3)
{
	float3 lightPosition;
	float padding1;
	float3 cameraPosition; // HS, PS
	float padding2;
}

cbuffer TerrainBuffer : register(b4)
{
	float gMinDist; 				// HS
	float gMaxDist;					// HS
	float gMinTess;					// HS
	float gMaxTess;					// HS
	float gTexelCellSpaceU;			// PS
	float gTexelCellSpaceV;			// PS
	float gWorldCellSpace;			// PS
	float gTexScale;				// DS
	float4 gWorldFrustumPlanes[6];	// HS
	
};

cbuffer LightSpaceTransformBuffer : register(b5)
{
	row_major matrix light_viewMatrix;
	row_major matrix light_projectionMatrix;
};

cbuffer TransformBuffer : register(b6)
{
	row_major matrix worldMatrix;
	row_major matrix worldInvTranspose;
}

SamplerState sampler0 : register(s0); // PS
SamplerState shadowSampler : register(s1); // PS
SamplerState samHeightmap : register(s2); // VS, DS, PS

Texture2DArray gLayerMapArray	: register(t0); // PS
Texture2D gBlendMap				: register(t1); // PS
Texture2D gHeightMap			: register(t2); // VS, DS, PS
Texture2D shadowMap				: register(t3);

VertexOut VS(VertexIn vin)
{
	VertexOut vout;

	// Terrain specified directly in world space.
	vout.PosW = mul(float4(vin.PosL, 1.0f), worldMatrix).xyz;

	// Displace the patch corners to world space.  This is to make 
	// the eye to patch distance calculation more accurate.
	vout.PosW.y = gHeightMap.SampleLevel(samHeightmap, vin.Tex, 0).r;

	// Output vertex attributes to next stage.
	vout.Tex = vin.Tex;
	vout.BoundsY = vin.BoundsY;

	return vout;
}
float CalcTessFactor(float3 p)
{
	float d = distance(p, cameraPosition);

	// max norm in xz plane (useful to see detail levels from a bird's eye).
	//float d = max( abs(p.x-gEyePosW.x), abs(p.z-gEyePosW.z) );

	float s = saturate((d - gMinDist) / (gMaxDist - gMinDist));

	return pow(2, (lerp(gMaxTess, gMinTess, s)));
}

// Returns true if the box is completely behind (in negative half space) of plane.
bool AabbBehindPlaneTest(float3 center, float3 extents, float4 plane)
{
	float3 n = abs(plane.xyz);

	// This is always positive.
	float r = dot(extents, n);

	// signed distance from center point to plane.
	float s = dot(float4(center, 1.0f), plane);

	// If the center point of the box is a distance of e or more behind the
	// plane (in which case s is negative since it is behind the plane),
	// then the box is completely in the negative half space of the plane.
	return (s + r) < 0.0f;
}

// Returns true if the box is completely outside the frustum.
bool AabbOutsideFrustumTest(float3 center, float3 extents, float4 frustumPlanes[6])
{
	for (int i = 0; i < 6; ++i)
	{
		// If the box is completely behind any of the frustum planes
		// then it is outside the frustum.
		if (AabbBehindPlaneTest(center, extents, frustumPlanes[i]))
		{
			return true;
		}
	}

	return false;
}

struct PatchTess
{
	float EdgeTess[4]   : SV_TessFactor;
	float InsideTess[2] : SV_InsideTessFactor;
};

PatchTess ConstantHS(InputPatch<VertexOut, 4> patch, uint patchID : SV_PrimitiveID)
{
	PatchTess pt;

	//
	// Frustum cull
	//

	// We store the patch BoundsY in the first control point.
	float minY = patch[0].BoundsY.x;
	float maxY = patch[0].BoundsY.y;

	// Build axis-aligned bounding box.  patch[2] is lower-left corner
	// and patch[1] is upper-right corner.
	float3 vMin = float3(patch[2].PosW.x, minY, patch[2].PosW.z);
	float3 vMax = float3(patch[1].PosW.x, maxY, patch[1].PosW.z);

	float3 boxCenter = 0.5f * (vMin + vMax);
	float3 boxExtents = 0.5f * (vMax - vMin);

	if (AabbOutsideFrustumTest(boxCenter, boxExtents, gWorldFrustumPlanes))
	{
		pt.EdgeTess[0] = 0.0f;
		pt.EdgeTess[1] = 0.0f;
		pt.EdgeTess[2] = 0.0f;
		pt.EdgeTess[3] = 0.0f;

		pt.InsideTess[0] = 0.0f;
		pt.InsideTess[1] = 0.0f;

		return pt;
	}
	//
	// Do normal tessellation based on distance.
	//
	else
	{
		// It is important to do the tess factor calculation based on the
		// edge properties so that edges shared by more than one patch will
		// have the same tessellation factor.  Otherwise, gaps can appear.

		// Compute midpoint on edges, and patch center
		float3 e0 = 0.5f * (patch[0].PosW + patch[2].PosW);
		float3 e1 = 0.5f * (patch[0].PosW + patch[1].PosW);
		float3 e2 = 0.5f * (patch[1].PosW + patch[3].PosW);
		float3 e3 = 0.5f * (patch[2].PosW + patch[3].PosW);
		float3  c = 0.25f * (patch[0].PosW + patch[1].PosW + patch[2].PosW + patch[3].PosW);

		pt.EdgeTess[0] = CalcTessFactor(e0);
		pt.EdgeTess[1] = CalcTessFactor(e1);
		pt.EdgeTess[2] = CalcTessFactor(e2);
		pt.EdgeTess[3] = CalcTessFactor(e3);

		pt.InsideTess[0] = CalcTessFactor(c);
		pt.InsideTess[1] = pt.InsideTess[0];

		return pt;
	}
}

struct HullOut
{
	float3 PosW     : POSITION;
	float2 Tex      : TEXCOORD0;
};

[domain("quad")]
[partitioning("fractional_even")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("ConstantHS")]
[maxtessfactor(64.0f)]
HullOut HS(InputPatch<VertexOut, 4> p,
	uint i : SV_OutputControlPointID,
	uint patchId : SV_PrimitiveID)
{
	HullOut hout;

	// Pass through shader.
	hout.PosW = p[i].PosW;
	hout.Tex = p[i].Tex;

	return hout;
}

struct DomainOut
{
	float4 PosH     : SV_POSITION;
	float3 PosW     : POSITION;
	float4 LPosH	: LIGHT_POSITION;
	float2 Tex      : TEXCOORD0;
	float2 TiledTex : TEXCOORD1;
};

// The domain shader is called for every vertex created by the tessellator.  
// It is like the vertex shader after tessellation.
[domain("quad")]
DomainOut DS(PatchTess patchTess,
	float2 uv : SV_DomainLocation,
	const OutputPatch<HullOut, 4> quad)
{
	DomainOut dout;

	// Bilinear interpolation.
	dout.PosW = lerp(
		lerp(quad[0].PosW, quad[1].PosW, uv.x),
		lerp(quad[2].PosW, quad[3].PosW, uv.x),
		uv.y);

	dout.Tex = lerp(
		lerp(quad[0].Tex, quad[1].Tex, uv.x),
		lerp(quad[2].Tex, quad[3].Tex, uv.x),
		uv.y);

	// Tile layer textures over terrain.
	dout.TiledTex = dout.Tex * float2(gTexScale, gTexScale);

	// Displacement mapping
	dout.PosW.y = gHeightMap.SampleLevel(samHeightmap, dout.Tex, 0).r;

	// NOTE: We tried computing the normal in the shader using finite difference, 
	// but the vertices move continuously with fractional_even which creates
	// noticable light shimmering artifacts as the normal changes.  Therefore,
	// we moved the calculation to the pixel shader.  

	// Project to homogeneous clip space.
	dout.PosH = mul(mul(float4(dout.PosW, 1.0f), viewMatrix), projectionMatrix);
	dout.LPosH = mul(mul(float4(dout.PosW, 1.0f), light_viewMatrix), light_projectionMatrix);
	return dout;
}

float4 PS(DomainOut pin) : SV_Target
{
	//
	// Estimate normal and tangent using central differences.
	//
	float2 leftTex = pin.Tex + float2(-gTexelCellSpaceU, 0.0f);
	float2 rightTex = pin.Tex + float2(gTexelCellSpaceU, 0.0f);
	float2 bottomTex = pin.Tex + float2(0.0f, gTexelCellSpaceV);
	float2 topTex = pin.Tex + float2(0.0f, -gTexelCellSpaceV);

	float leftY = gHeightMap.SampleLevel(samHeightmap, leftTex, 0).r;
	float rightY = gHeightMap.SampleLevel(samHeightmap, rightTex, 0).r;
	float bottomY = gHeightMap.SampleLevel(samHeightmap, bottomTex, 0).r;
	float topY = gHeightMap.SampleLevel(samHeightmap, topTex, 0).r;

	float3 tangent = normalize(float3(2.0f * gWorldCellSpace, rightY - leftY, 0.0f));
	float3 bitan = normalize(float3(0.0f, bottomY - topY, -2.0f * gWorldCellSpace));
	float3 normalW = cross(tangent, bitan);


	// The toEye vector is used in lighting.
	float3 toEye = cameraPosition - pin.PosW;

	// Cache the distance to the eye from this surface point.
	float distToEye = length(toEye);

	// Normalize.
	toEye /= distToEye;

	float3 lightDirection = normalize(lightPosition);
	float3 viewDirection = normalize(cameraPosition - pin.PosW);

	//
	// Texturing
	//

	// Sample layers in texture array.
	float4 c0 = gLayerMapArray.Sample(sampler0, float3(pin.TiledTex, 0.0f));
	float4 c1 = gLayerMapArray.Sample(sampler0, float3(pin.TiledTex, 1.0f));
	float4 c2 = gLayerMapArray.Sample(sampler0, float3(pin.TiledTex, 2.0f));
	float4 c3 = gLayerMapArray.Sample(sampler0, float3(pin.TiledTex, 3.0f));
	float4 c4 = gLayerMapArray.Sample(sampler0, float3(pin.TiledTex, 4.0f));

	// Sample the blend map.
	float4 t = gBlendMap.Sample(sampler0, pin.Tex);

	// Blend the layers on top of each other.
	float4 texColor = c0;
	texColor = lerp(texColor, c1, t.r); 
	texColor = lerp(texColor, c2, t.g);
	texColor = lerp(texColor, c3, t.b);
	texColor = lerp(texColor, c4, t.a);


	Material mat;
	mat.Ambient =  materialAmbient;
	mat.Diffuse =  materialDiffuse;
	mat.Specular =  materialSpecular;

	DirectionalLight light;
	light.Ambient = ambient;
	light.Diffuse = diffuse;
	light.Specular = specular;
	light.Direction = lightDirection;

	Shadow shadow;
	shadow.lightPosition = pin.LPosH;
	shadow.shadowSampler = shadowSampler;
	shadow.shadowMap = shadowMap;
	shadow.normal = normalW;
	shadow.lightDir = -lightDirection;

	float shadowFactor = CalculateShadowFactor(shadow);

	ComputeDirectionalLight(mat, light, normalW, viewDirection, texColor, shadowFactor);

	//// perform perspective divide
	//float2 projCoords;
	//projCoords.x = pin.LPosH.x / pin.LPosH.w / 2.0f + 0.5f;
	//projCoords.y = -pin.LPosH.y / pin.LPosH.w / 2.0f + 0.5f;

	//if (projCoords.x < 0.0f || projCoords.x > 1.0f || projCoords.y < 0.0f || projCoords.y > 1.0f)
	//{
	//	// 그림자 계산을 하지 않고 원래 조명값으로 리턴
	//	return texColor;
	//}

	//// get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
	//float closestDepth = shadowMap.Sample(shadowSampler, projCoords).r;

	//// get depth of current fragment from light's perspective
	//float currentDepth = pin.LPosH.z / pin.LPosH.w;

	//float shadowBias = 0.001; // 바이어스 값
	//float shadow = (currentDepth - shadowBias) < closestDepth ? 1.0 : 0.0;

	//if (currentDepth >= 0.99f)
	//	shadow = 1.0;
	//texColor = float4(texColor.r * (shadow), texColor.g * (shadow), texColor.b * (shadow), texColor.a);
	return texColor;

}
