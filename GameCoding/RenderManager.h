#pragma once
#include "ConstantBufferType.h"
#include "Pipeline.h"
#include "Buffer.h"
#include "GameObject.h"

enum class FilterType
{
	NORMAL,
	SHADOW_MAP,
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
	void DrawRenderableObject(bool isEnv);
	void RenderEnvironmentMappedObjects(shared_ptr<GameObject> gameObject, shared_ptr<Texture> envTexture);
	void Render();
	void RenderAllGameObject();
	void ClearRenderObject();
	bool GetShadowMapFlag() { return _drawShadowMapFlag; }
private:
	shared_ptr<Pipeline> _pipeline;
	PipelineInfo _pipelineInfo;
	vector<shared_ptr<GameObject>> _renderObjects;
	vector<shared_ptr<GameObject>> _envMappedObjects;
	vector<shared_ptr<GameObject>> _billboardObjs;
	shared_ptr<GameObject> _terrainObject;
	shared_ptr<GameObject> _lightObject;
	float _lightRotationAngle;
	float tmp;
	KeyframeDesc _keyframeDesc;
	BlendAnimDesc _blendAnimDesc;
	float animationSumTime = 0.0f;
	FilterType _filterType = FilterType::SHADOW_MAP;
	bool _drawShadowMapFlag = false;
};

