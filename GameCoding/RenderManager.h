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

	void Init();
	void Update();

	void GetRenderableObject();
	void DrawRenderableObject(bool isEnv);
	void DrawUIObject(ComPtr<ID3D11ShaderResourceView> srv);
	void RenderEnvironmentMappedObjects(shared_ptr<GameObject> gameObject, shared_ptr<Texture> envTexture);
	void Render();
	void RenderAllGameObject();
	void ClearRenderObject();
	bool GetShadowMapFlag() { return _drawShadowMapFlag; }

	// for instancing
	void ClearData();
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

