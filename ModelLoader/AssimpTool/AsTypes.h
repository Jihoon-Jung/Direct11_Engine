#pragma once


struct VertexTextureNormalTangentBlendData
{
	Vec3 position = { 0, 0, 0 };
	Vec2 uv = { 0, 0 };
	Vec3 normal = { 0, 0, 0 };
	Vec3 tangent = { 0, 0, 0 };
	Vec4 blendIndices = { 0, 0, 0, 0 };
	Vec4 blendWeights = { 0, 0, 0, 0 };
};

struct asBone
{
	string name;
	int32 index = -1;
	int32 parent = -1;
	Matrix transform;
};

struct asMesh
{
	string name;
	aiMesh* mesh;
	vector<VertexTextureNormalTangentBlendData> vertices;
	vector<uint32> indices;

	int32 boneIndex;
	string materialName;
};

struct asMaterial
{
	string name;
	Color ambient;
	Color diffuse;
	Color specular;
	Color emissive;
	string diffuseFile;
	string specularFile;
	string normalFile;
};
// Animation
struct asBlendWeight
{
	void Set(uint32 index, uint32 boneIndex, float weight)
	{
		float i = (float)boneIndex;
		float w = weight;

		switch (index)
		{
		case 0: indices.x = i; weights.x = w; break;
		case 1: indices.y = i; weights.y = w; break;
		case 2: indices.z = i; weights.z = w; break;
		case 3: indices.w = i; weights.w = w; break;
		}
	}

	Vec4 indices = Vec4(0, 0, 0, 0); // 4개의 뼈 인덱스
	Vec4 weights = Vec4(0, 0, 0, 0); // 4개의 뼈에 의해 영향받는 가중치
};

// 정점마다 -> (관절번호, 가중치)
struct asBoneWeights
{
	// 가중치가 높은놈들을 앞에 두는것같음
	void AddWeights(uint32 boneIndex, float weight)
	{
		if (weight <= 0.0f)
			return;

		auto findIt = std::find_if(boneWeights.begin(), boneWeights.end(),
			[weight](const Pair& p) { return weight > p.second; });

		boneWeights.insert(findIt, Pair(boneIndex, weight));
	}

	asBlendWeight GetBlendWeights()
	{
		asBlendWeight blendWeights;

		for (uint32 i = 0; i < boneWeights.size(); i++)
		{
			if (i >= 4)
				break;

			blendWeights.Set(i, boneWeights[i].first, boneWeights[i].second);
		}

		return blendWeights;
	}
	// 모든 가중치의 합이 1로 되도록 변경
	void Normalize()
	{
		if (boneWeights.size() >= 4)
			boneWeights.resize(4);

		float totalWeight = 0.f;
		for (const auto& item : boneWeights)
			totalWeight += item.second;

		float scale = 1.f / totalWeight;
		for (auto& item : boneWeights)
			item.second *= scale;
	}

	using Pair = pair<int32, float>; // bone index , weight
	vector<Pair> boneWeights;
};

struct asKeyframeData
{
	float time;
	Vec3 scale;
	Quaternion rotation;
	Vec3 translation;
};

struct asKeyframe
{
	// 관절 1개당 관절 이름과 trasnform 정보가 있다.
	string boneName;
	vector<asKeyframeData> transforms;
};

struct asAnimation
{
	string name;
	uint32 frameCount;// 몇 프레임인가
	float frameRate;
	float duration; // 지속시간
	vector<shared_ptr<asKeyframe>> keyframes; // 매 프레임마다 관절들이 어떤 정보를 가지고 있을것인가
};

// Cache
struct asAnimationNode
{
	aiString name;
	vector<asKeyframeData> keyframe;
};