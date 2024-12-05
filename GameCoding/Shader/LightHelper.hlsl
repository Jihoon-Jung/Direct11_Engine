

struct DirectionalLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;
	float3 Direction;
	float pad;
};

struct PointLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 Position;
	float Range;

	float3 Att;
	float pad;
};

struct SpotLight
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular;

	float3 Position;
	float Range;

	float3 Direction;
	float Spot;

	float3 Att;
	float pad;
};

struct Material
{
	float4 Ambient;
	float4 Diffuse;
	float4 Specular; // w = SpecPower
};
struct Shadow
{
	float4 lightPosition;
	SamplerState shadowSampler;
	Texture2D shadowMap;
};
//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a directional light.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------
void ComputeDirectionalLight(Material mat, DirectionalLight light,
	float3 normal, float3 viewDirection, inout float4 textureColor, float shadowFactor)
{
	// Initialize outputs.
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The light vector aims opposite the direction the light rays travel.
	float3 lightVec = -light.Direction;

	// Add ambient term.
	ambient = mat.Ambient * light.Ambient;

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.

	float diffuseFactor = dot(lightVec, normal);
	
	// Flatten to avoid dynamic branching.
	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, viewDirection), 0.0f), mat.Specular.w);

		diffuse = diffuseFactor * mat.Diffuse * light.Diffuse * shadowFactor;
		spec = specFactor * mat.Specular * light.Specular * shadowFactor;
	}

	float4 lightColor = textureColor * (ambient + diffuse) + spec;
	textureColor = lightColor;
	
}

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a point light.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------
void ComputePointLight(Material mat, PointLight L, float3 pos, float3 normal, float3 toEye,
	out float4 ambient, out float4 diffuse, out float4 spec)
{
	// Initialize outputs.
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The vector from the surface to the light.
	float3 lightVec = L.Position - pos;

	// The distance from surface to light.
	float d = length(lightVec);

	// Range test.
	if (d > L.Range)
		return;

	// Normalize the light vector.
	lightVec /= d;

	// Ambient term.
	ambient = mat.Ambient * L.Ambient;

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.

	float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.
	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec = specFactor * mat.Specular * L.Specular;
	}

	// Attenuate
	float att = 1.0f / dot(L.Att, float3(1.0f, d, d * d));

	diffuse *= att;
	spec *= att;
}

//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a spotlight.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------
void ComputeSpotLight(Material mat, SpotLight L, float3 pos, float3 normal, float3 toEye,
	out float4 ambient, out float4 diffuse, out float4 spec)
{
	// Initialize outputs.
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The vector from the surface to the light.
	float3 lightVec = L.Position - pos;

	// The distance from surface to light.
	float d = length(lightVec);

	// Range test.
	if (d > L.Range)
		return;

	// Normalize the light vector.
	lightVec /= d;

	// Ambient term.
	ambient = mat.Ambient * L.Ambient;

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.

	float diffuseFactor = dot(lightVec, normal);

	// Flatten to avoid dynamic branching.
	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, toEye), 0.0f), mat.Specular.w);

		diffuse = diffuseFactor * mat.Diffuse * L.Diffuse;
		spec = specFactor * mat.Specular * L.Specular;
	}

	// Scale by spotlight factor and attenuate.
	float spot = pow(max(dot(-lightVec, L.Direction), 0.0f), L.Spot);

	// Scale by spotlight factor and attenuate.
	float att = spot / dot(L.Att, float3(1.0f, d, d * d));

	ambient *= spot;
	diffuse *= att;
	spec *= att;
}


//---------------------------------------------------------------------------------------
// Transforms a normal map sample to world space.
//---------------------------------------------------------------------------------------
float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW)
{
	// Uncompress each component from [0,1] to [-1,1].
	float3 normalT = 2.0f * normalMapSample - 1.0f;

	// Build orthonormal basis.
	float3 N = unitNormalW;
	float3 T = normalize(tangentW - dot(tangentW, N) * N);
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
	float3 bumpedNormalW = mul(normalT, TBN);

	return bumpedNormalW;
}

void ComputeNormalMapping(inout float3 normal, float3 tangent, float2 uv, Texture2D normalMap, SamplerState sampler0)
{
	// [0,255] 범위에서 [0,1]로 변환
	float4 map = normalMap.Sample(sampler0, uv);
	if (any(map.rgb) == false)
		return;

	float3 N = normalize(normal); // z
	float3 T = normalize(tangent); // x
	float3 B = normalize(cross(N, T)); // y
	float3x3 TBN = float3x3(T, B, N); // TS -> WS

	// [0,1] 범위에서 [-1,1] 범위로 변환
	float3 tangentSpaceNormal = (map.rgb * 2.0f - 1.0f);
	float3 worldNormal = mul(tangentSpaceNormal, TBN);

	normal = worldNormal;
}

float CalculateShadowFactor(Shadow info)
{
	float shadow = 1.0;

	float2 projCoords;
	projCoords.x = (info.lightPosition.x / info.lightPosition.w) * 0.5f + 0.5f;
	projCoords.y = -(info.lightPosition.y / info.lightPosition.w) * 0.5f + 0.5f;

	if (projCoords.x < 0.0f || projCoords.x > 1.0f || projCoords.y < 0.0f || projCoords.y > 1.0f)
	{
		// 그림자 계산을 하지 않음
		return shadow;
	}

	float currentDepth = info.lightPosition.z / info.lightPosition.w;
	float shadowBias = 0.005; // 바이어스 값


	float shadowSum = 0.0f;
	float texelSize = 1.0f / 2048.0f; // 해상도를 높임
	int pcfCount = 5; // PCF 커널 크기를 증가
	int halfCount = pcfCount / 2;

	[unroll]
	for (int x = -halfCount; x <= halfCount; x++)
	{
		[unroll]
		for (int y = -halfCount; y <= halfCount; y++)
		{
			float2 offset = float2(x * texelSize, y * texelSize);
			float closestDepth = info.shadowMap.Sample(info.shadowSampler, projCoords + offset).r;
			float currentDepthCheck = (currentDepth - shadowBias) < closestDepth ? 1.0 : 0.0;
			shadowSum += currentDepthCheck;
		}
	}

	shadow = shadowSum / (pcfCount * pcfCount);
	
	// // 표면 기울기에 따른 동적 bias 계산
	//float bias = max(0.005 * (1.0 - dot(normal, lightDir)), 0.001);

	//// PCF 강화
	//shadow = 0.0;
	//float2 texelSize = 1.0 / 2048.0; // shadowmap 해상도에 맞게 조정

	//// PCF 커널 크기를 7x7로 증가
	//[unroll]
	//for (int x = -3; x <= 3; ++x)
	//{
	//	[unroll]
	//	for (int y = -3; y <= 3; ++y)
	//	{
	//		// 가우시안 가중치 적용
	//		float weight = 1.0 / 49.0; // 7x7 커널
	//		float2 offset = float2(x, y) * texelSize;

	//		// 약간의 랜덤 오프셋 추가로 엘리어싱 감소
	//		float noise = frac(sin(dot(projCoords.xy, float2(12.9898, 78.233))) * 43758.5453);
	//		offset += texelSize * noise * 0.5;

	//		float pcfDepth = info.shadowMap.Sample(info.shadowSampler, projCoords + offset).r;
	//		shadow += ((currentDepth - bias) < pcfDepth ? 1.0 : 0.0) * weight;
	//	}
	//}

	//return shadow;
	return shadow;
}