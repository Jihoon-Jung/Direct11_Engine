#pragma once
#include "ConstantBufferType.h"
#include "Pipeline.h"
#include "Buffer.h"
#include "GameObject.h"

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

	void GetRenderObject();
	void RenderObject();
	void ClearRenderObject();
private:
	shared_ptr<Pipeline> _pipeline;
	PipelineInfo _pipelineInfo;
	vector<shared_ptr<GameObject>> _renderObjects;
	KeyframeDesc _keyframeDesc;
	BlendAnimDesc _blendAnimDesc;
	float animationSumTime = 0.0f;
};

