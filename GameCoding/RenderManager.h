#pragma once
#include "ConstantBufferType.h"
#include "Pipeline.h"
#include "Buffer.h"
#include "GameObject.h"

enum class FilterType
{
	NORMAL,
	GAUSSIAN_BLUR
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
	void RenderRenderableObject(bool isEnv);
	void RenderEnvironmentMappedObjects(shared_ptr<GameObject> gameObject, shared_ptr<Texture> envTexture);
	void Render();
	void ClearRenderObject();

private:
	shared_ptr<Pipeline> _pipeline;
	PipelineInfo _pipelineInfo;
	vector<shared_ptr<GameObject>> _renderObjects;
	vector<shared_ptr<GameObject>> _envMappedObjects;
	vector<shared_ptr<GameObject>> _billboardObjs;
	shared_ptr<GameObject> _terrainObject;
	KeyframeDesc _keyframeDesc;
	BlendAnimDesc _blendAnimDesc;
	float animationSumTime = 0.0f;
	FilterType _filterType = FilterType::NORMAL;
};

