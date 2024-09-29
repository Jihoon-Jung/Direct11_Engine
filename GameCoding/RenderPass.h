#pragma once
#include "RasterizerState.h"
#include "DepthStencilState.h"
#include "SamplerState.h"
#include "BlendState.h"
#include "RasterizerStateInfo.h"

class MeshRenderer;
struct RasterizerStateInfo;

enum class Pass
{
	DEFAULT_RENDER,
	OUTLINE_RENDER,
	GAUSSIANBLUR_RENDER,
	SHADOWMAP_RENDER,
	QUAD_RENDER,
	TERRAIN_RENDER,
	// TODO
};

class RenderPass
{
public:
	RenderPass();
	~RenderPass();

	void Render();
	void DefaultRender();
	void OutlineRender();
	void GaussianBlurRender();
	void QuadRender();
	void TerrainRender();
	void SetRenderTarget(int width, int height);
	void SetMesh(shared_ptr<Mesh> mesh) { _mesh = mesh; }
	void SetTexture(shared_ptr<Texture> texture) { _texture = texture; }
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
	void SetTessellationFlag(bool flag) { isUseTessellation = flag; }
	void SetEnvironmentMapFlag(bool flag) { isEnvironmentMap = flag; }
	void SetTerrainFlag(bool flag) { isTerrain = flag; }
	bool GetTessellationFlag() { return isUseTessellation; }
	bool GetTerrainFlag() { return isTerrain; }
	void SetTransform(shared_ptr<Transform> transform) { _transform = transform; }
	void SetDepthStencilStateType(DSState state) { _dsStateType = state; }
	ComPtr<ID3D11ShaderResourceView> GetOutputSRV() { return _outputSRV; }
	ComPtr<ID3D11ShaderResourceView> GetOffscreenSRV() { return _offscreenSRV; }
	ComPtr<ID3D11RenderTargetView> GetOffscreenRTV() { return _offscreenRTV; }

public:
	ComPtr<ID3D11RenderTargetView> _renderTargetView;
	shared_ptr<Mesh> _mesh;
	shared_ptr<Texture> _texture;
	shared_ptr<Texture> _diffuseMap;
	shared_ptr<Texture> _normalMap;
	shared_ptr<Texture> _specularMap;
	shared_ptr<MeshRenderer> _meshRenderer;
	shared_ptr<RasterizerState> _rasterizerState;
	shared_ptr<BlendState> _blendState;
	shared_ptr<SamplerState> _samplerState;
	shared_ptr<DepthStencilState> _depthStencilState;
	shared_ptr<Shader> _shader;
	shared_ptr<Transform> _transform;
	bool isUseTessellation = false;
	bool isTerrain = false;
	bool isEnvironmentMap = false;
	DSState _dsStateType;
	Pass _pass;
	RasterizerStateInfo _rasterizerStates;
	ComPtr<ID3D11ShaderResourceView> _offscreenSRV;
	ComPtr<ID3D11RenderTargetView> _offscreenRTV;
	ComPtr<ID3D11ShaderResourceView> _inputSRV;
	ComPtr<ID3D11ShaderResourceView> _outputSRV;
};

