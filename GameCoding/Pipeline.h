#pragma once
#include "InputLayout.h"
#include "Shader.h"
#include "RasterizerState.h"
#include "BlendState.h"
#include "SamplerState.h"
#include "Buffer.h"
#include "Texture.h"

struct PipelineInfo
{
	shared_ptr<InputLayout> inputLayout;
	shared_ptr<RasterizerState> rasterizerState;
	shared_ptr<BlendState> blendState;
	shared_ptr<SamplerState> samplerState;
	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
};

class Pipeline
{
public:
	Pipeline();
	~Pipeline();
	bool isAnimation = false;
	void SetTexture(shared_ptr<Texture> texture) { _texture = texture; }
	void SetNormalMap(shared_ptr<Texture> normal) { _normalMap = normal; }
	void SetSpecularMap(shared_ptr<Texture> specular) { _specularMap = specular; }
	void SetDiffuseMap(shared_ptr<Texture> diffuse) { _diffuseMap = diffuse; }
	void UpdatePipeline(PipelineInfo pipelineInfo);
	void SetBuffer(shared_ptr<Buffer> buffer) { _buffer = buffer; }
	void SetIndicesSize(UINT size) { _indicesSize = size; }
	void SetShader(shared_ptr<Shader> vertexShader, shared_ptr<Shader> pixelShader)
	{
		_vertexShader = vertexShader;
		_pixelShader = pixelShader;
	}
	PipelineInfo GetPipelineInfo() { return _pipelineInfo; }
private:
	PipelineInfo _pipelineInfo;
	shared_ptr<Buffer> _buffer;
	shared_ptr<Texture> _texture;
	shared_ptr<Texture> _normalMap;
	shared_ptr<Texture> _specularMap;
	shared_ptr<Texture> _diffuseMap;
	shared_ptr<Shader> _vertexShader;
	shared_ptr<Shader> _pixelShader;
	UINT _indicesSize = 0;
	
};

