#pragma once
#include "ConstantBufferType.h"
#include "Pipeline.h"
#include "Buffer.h"
#include "GameObject.h"
#include "InstancingBuffer.h"

#define MAX_MODEL_INSTANCE 500

enum class FilterType
{
	NORMAL,
	SHADOW_MAP,
	GAUSSIAN_BLUR
};

struct InstancedBlendDesc
{
	BlendAnimDesc blendDesc[MAX_MODEL_INSTANCE];
};

class RenderManager
{
protected:
	RenderManager() {}
	virtual ~RenderManager() {}
public:
	RenderManager(const RenderManager&) = delete;
	static RenderManager& GetInstance()
	{
		static RenderManager instance;
		return instance;
	}

	void Reset() {
		_pipeline = nullptr;
		_renderObjects.clear();
		_envMappedObjects.clear();
		_billboardObjs.clear();
		_UIObjects.clear();
		_particleObjects.clear();
		_terrainObject = nullptr;
		_lightObject = nullptr;
		_buffers.clear();
		cache_DefaultRender.clear();
		cache_StaticMeshRender.clear();
		cache_AnimatedRender.clear();
	}

	void Init();
	void Update();

	void GetRenderableObject();
	void DrawRenderableObject(bool isEnv);
	void DrawUIObject();
	void DrawParticleObject();
	void RenderEnvironmentMappedObjects(shared_ptr<GameObject> gameObject, shared_ptr<Texture> envTexture);
	void Render();
	void RenderAllGameObject();
	void ClearRenderObject();
	bool GetShadowMapFlag() { return _drawShadowMapFlag; }

	// for instancing
	void ClearData();
	void UpdateInstancingObject();
	void GetDefaultRenderObjectsForInstancing(vector<shared_ptr<GameObject>>& gameObjects);
	void DrawInstancingDefaultRenderObject(bool isEnv);
	void GetStaticMeshObjectsForInstancing(vector<shared_ptr<GameObject>>& gameObjects);
	void DrawInstancingStaticMeshObject(bool isEnv);
	void GetAnimatedMeshObjectsForInstancing(vector<shared_ptr<GameObject>>& gameObjects);
	void DrawInstancingAnimatedMeshObject(bool isEnv);
	void AddData(InstanceID instanceId, InstancingData& data);
	shared_ptr<Texture> _envTexture;

private:
	shared_ptr<Pipeline> _pipeline;
	PipelineInfo _pipelineInfo;
	vector<shared_ptr<GameObject>> _renderObjects;
	vector<shared_ptr<GameObject>> _envMappedObjects;
	vector<shared_ptr<GameObject>> _billboardObjs;
	vector<shared_ptr<GameObject>> _UIObjects;
	vector<shared_ptr<GameObject>> _particleObjects;

	shared_ptr<GameObject> _terrainObject;
	shared_ptr<GameObject> _lightObject;
	float _totalRotationTime = 0.0f;

	KeyframeDesc _keyframeDesc;
	BlendAnimDesc _blendAnimDesc;
	float animationSumTime = 0.0f;
	FilterType _filterType = FilterType::SHADOW_MAP;
	bool _drawShadowMapFlag = false;

	map<InstanceID, shared_ptr<InstancingBuffer>> _buffers;
	map<InstanceID, vector<shared_ptr<GameObject>>> cache_DefaultRender;
	map<InstanceID, vector<shared_ptr<GameObject>>> cache_StaticMeshRender;
	map<InstanceID, vector<shared_ptr<GameObject>>> cache_AnimatedRender;
	shared_ptr<InstancedBlendDesc> _cache_blendDescs;
};

