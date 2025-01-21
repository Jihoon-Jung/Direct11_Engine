#pragma once
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "SamplerState.h"
#include "BlendState.h"
#include "RasterizerStateInfo.h"
#include "InstancingBuffer.h"

class MeshRenderer;
class Animator;
class Transition;
struct RasterizerStateInfo;
struct InstancedBlendDesc;

enum class Pass
{
	DEFAULT_RENDER,
	OUTLINE_RENDER,
	GAUSSIANBLUR_RENDER,
	SHADOWMAP_RENDER,
	QUAD_RENDER,
	TESSELLATION_RENDER,
	TERRAIN_RENDER,
	ENVIRONMENTMAP_RENDER,
	STATIC_MESH_RENDER,
	ANIMATED_MESH_RENDER,
	PARTICLE_RENDER,
	UI_RENDER,
	// TODO
};

class RenderPass
{
public:
	RenderPass();
	~RenderPass();

	void Render(bool isEnv);
	void DefaultRender(bool isEnv);
	void DefaultRenderInstance(bool isEnv, shared_ptr<InstancingBuffer>& instancingBuffer);
	void StaticMeshRenderInstance(bool isEnv, shared_ptr<InstancingBuffer>& instancingBuffer);
	void AnimatedMeshRenderInstance(bool isEnv, shared_ptr<InstancingBuffer>& instancingBuffer, const InstancedBlendDesc& desc);
	void EnvironmentMapRender();
	void OutlineRender(bool isEnv);
	void GaussianBlurRender(bool isEnv);
	void QuadRender(bool isEnv);
	void TessellationRender(bool isEnv);
	void TerrainRender(bool isEnv);
	void StaticMeshRencer(bool isEnv);
	void AnimatedMeshRender(bool isEnv);
	void ParticleRender(bool isEnv);
	void DebugQuadRender();
	void SetRenderTarget(int width, int height);
	void SetMesh(shared_ptr<Mesh> mesh) { _mesh = mesh; }
	void SetTexture(shared_ptr<Texture> texture) { _envTexture = texture; }
	void SetDiffuseMap(shared_ptr<Texture> diffuseMap) { _diffuseMap = diffuseMap; }
	void SetNormalMap(shared_ptr<Texture> normalMap) { _normalMap = normalMap; }
	void SetSpecularMap(shared_ptr<Texture> specularMap) { _specularMap = specularMap; }
	void SetShader(shared_ptr<Shader> shader) { _shader = shader; }
	void SetPass(Pass pass) { _pass = pass; }
	void CreateAndSetOffscreenRenderTarget(int width, int height);
	void SetInputSRV(ComPtr<ID3D11ShaderResourceView> input) { _inputSRV = input; }
	void SaveRenderTargetToFile(ID3D11RenderTargetView* renderTargetView, const std::string& filename, int width, int height);
	void SetRasterizerStates(RasterizerStateInfo states) { _rasterizerStates = states; }
	void SetMeshRenderer(shared_ptr<MeshRenderer> meshRenderer) { _meshRenderer = meshRenderer; }
	void SetTransform(shared_ptr<Transform> transform) { _transform = transform; }
	void SetDepthStencilStateType(DSState state) { _dsStateType = state; }
	void HandleTransitionBlend(shared_ptr<Animator>& animator, shared_ptr<Transition>& transition, shared_ptr<Model>& model);

	Pass GetPass() const { return _pass; }
	DSState GetDepthStencilStateType() const { return _dsStateType; }

	ComPtr<ID3D11ShaderResourceView> GetOutputSRV() { return _outputSRV; }
	ComPtr<ID3D11ShaderResourceView> GetOffscreenSRV() { return _offscreenSRV; }
	ComPtr<ID3D11RenderTargetView> GetOffscreenRTV() { return _offscreenRTV; }

public:
	ComPtr<ID3D11RenderTargetView> _renderTargetView;
	shared_ptr<Mesh> _mesh;
	shared_ptr<Texture> _envTexture;
	shared_ptr<Texture> _diffuseMap;
	shared_ptr<Texture> _normalMap;
	shared_ptr<Texture> _specularMap;
	shared_ptr<MeshRenderer> _meshRenderer;
	shared_ptr<RasterizerState> _rasterizerState;
	shared_ptr<BlendState> _blendState;
	shared_ptr<SamplerState> _samplerState;
	shared_ptr<DepthStencilState> _depthStencilState;
	shared_ptr<Shader> _shader;
	weak_ptr<Transform> _transform;
	DSState _dsStateType;
	Pass _pass;
	RasterizerStateInfo _rasterizerStates;
	ComPtr<ID3D11ShaderResourceView> _offscreenSRV;
	ComPtr<ID3D11RenderTargetView> _offscreenRTV;
	ComPtr<ID3D11ShaderResourceView> _inputSRV;
	ComPtr<ID3D11ShaderResourceView> _outputSRV;

	KeyframeDesc _keyframeDesc;
	BlendAnimDesc _blendAnimDesc;
	float animationSumTime = 0.0f;

	shared_ptr<Transform> _transformPtr;
};

