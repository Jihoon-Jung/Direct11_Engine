

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
	float3 lightDir;
	float3 normal;
	SamplerState shadowSampler;
	Texture2D shadowMap;
};

static const float2 POISSON_DISK[16] =
{
	float2(-0.94201624, -0.39906216),
	float2(0.94558609, -0.76890725),
	float2(-0.09418410, -0.92938870),
	float2(0.34495938,  0.29387760),
	float2(-0.91588581,  0.45771432),
	float2(-0.81544232, -0.87912464),
	float2(-0.38277543,  0.27676845),
	float2(0.97484398,  0.75648379),
	float2(0.44323325, -0.97511554),
	float2(0.53742981, -0.47373420),
	float2(-0.26496911, -0.41893023),
	float2(0.79197514,  0.19090188),
	float2(-0.24188840,  0.99706507),
	float2(-0.81409955,  0.91437590),
	float2(0.19984126,  0.78641367),
	float2(0.14383161, -0.14100790)
};


//---------------------------------------------------------------------------------------
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a directional light.  We need to output the terms separately because
// later we will modify the individual terms.
//---------------------------------------------------------------------------------------
void ComputeDirectionalLight(Material mat, DirectionalLight light,
	float3 normal, float3 viewDirection, inout float4 textureColor, float shadowFactor)
{
	float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

	float3 lightVec = -light.Direction;

	// Add ambient term
	ambient = mat.Ambient * light.Ambient;

	// Improved diffuse calculation
	float NdotL = dot(lightVec, normal);
	float wrappedDiffuse = (NdotL + 0.5) / 1.5;  // Wrap lighting
	float softDiffuse = smoothstep(-0.1, 1.0, wrappedDiffuse);  // Smooth transition

	[flatten]
	if (NdotL > -0.5)  // Extended range for smoother falloff
	{
		float3 v = reflect(-lightVec, normal);
		float specFactor = pow(max(dot(v, viewDirection), 0.0f), mat.Specular.w);

		diffuse = softDiffuse * mat.Diffuse * light.Diffuse * shadowFactor;
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
	// [0,255] �������� [0,1]�� ��ȯ
	float4 map = normalMap.Sample(sampler0, uv);
	if (any(map.rgb) == false)
		return;

	float3 N = normalize(normal); // z
	float3 T = normalize(tangent); // x
	float3 B = normalize(cross(N, T)); // y
	float3x3 TBN = float3x3(T, B, N); // TS -> WS

	// [0,1] �������� [-1,1] ������ ��ȯ
	float3 tangentSpaceNormal = (map.rgb * 2.0f - 1.0f);
	float3 worldNormal = mul(tangentSpaceNormal, TBN);

	normal = worldNormal;
}

float CalculateShadowFactor(Shadow info)
{
	// 1. light space ��ǥ�� [0,1] ������ ��ȯ
	float4 lightPos = info.lightPosition;
	float2 projCoords;
	projCoords.x = (lightPos.x / lightPos.w) * 0.5f + 0.5f;
	projCoords.y = -(lightPos.y / lightPos.w) * 0.5f + 0.5f;

	// ������ ����� �׸��� ����
	if (projCoords.x < 0.0f || projCoords.x > 1.0f ||
		projCoords.y < 0.0f || projCoords.y > 1.0f)
	{
		return 1.0f;
	}

	// 2. ���� �ȼ��� ���̰� (light space)
	float currentDepth = lightPos.z / lightPos.w;

	// 3. ǥ�� normal�� light direction�� ���� ���
	float3 norm = normalize(info.normal);
	float3 lightDir = normalize(info.lightDir);
	float cosTheta = saturate(dot(norm, lightDir));

	// ������ ����� ���� �׸��� ������
	if (cosTheta < 0.2f)
	{
		return 1.0f;
	}

	// 4. slope-scale bias ���
	float bias = max(0.005f * (1.0f - cosTheta), 0.001f);

	// 5. Normal offset bias
	float adjustedDepth = currentDepth - bias;

	// 6. Poisson Disk ��� PCF ���ø�
	//    (16���� ������ ���� ����Ʈ�� ���)
	float shadow = 0.0f;

	// Shadow Map�� 1920x1040�� ���
	float2 texelSize = float2(1.0f / 1920.0f, 1.0f / 1040.0f);

	// "���� �ݰ�" (���� �����ؼ� ����)
	float radius = 5.0f;

	// 16�� Poisson Disk ����
	for (int i = 0; i < 16; i++)
	{
		float2 offset = POISSON_DISK[i] * (radius * texelSize);
		float sampleDepth = info.shadowMap.Sample(info.shadowSampler, projCoords + offset).r;
		shadow += (adjustedDepth < sampleDepth) ? 1.0f : 0.0f;
	}

	// ��հ�
	shadow /= 16.0f;

	// 7. ���� ���� ���̰� near plane (��, ��ü�� �ָ� ���� ���)��� �׸��� ȿ�� ����
	if (currentDepth > 0.99f)
	{
		shadow = 1.0f;
	}

	return shadow;
}



