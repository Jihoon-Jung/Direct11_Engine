#pragma once
#include "Model.h"
#include "RenderPass.h"

class MeshRenderer : public Component
{
	using Super = Component;

public:
	MeshRenderer();
	virtual ~MeshRenderer();
	virtual void Update() override;
	void SetMaterial(shared_ptr<Material> material);
	void SetMesh(shared_ptr<Mesh> mesh);
	void SetTexture(shared_ptr<Texture> texture) { _material->SetTexture(texture); };
	void SetModel(shared_ptr<Model> model);
	void SetRasterzierState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, bool frontCounterClockwise);
	void AddRenderPass();
	void SetUseEnvironmentMap(bool isUseEnvironmentMap) { _isUseEnvironmentMap = isUseEnvironmentMap; }
	bool GetUseEnvironmentMap() { return _isUseEnvironmentMap; }
	auto GetMaterial() { return _material; }
	auto GetShader() { return GetMaterial()->GetShader(); }
	auto GetInputLayout() { return GetShader()->GetInputLayout(); }
	auto GetRasterzerStates() { return _rasterzerStates; }
	auto GetRenderPasses() { return _renderPasses; }

	bool CheckUseEnvironmentMap() { return _isUseEnvironmentMap; }

	shared_ptr<Mesh> GetMesh() { return _mesh; }
	shared_ptr<Model> GetModel() { return _model; }
	shared_ptr<Texture> GetTexture() { return GetMaterial()->GetTexture(); }
	shared_ptr<Buffer> GetMaterialBuffer();

	InstanceID GetInstanceID_DefaultMesh() {
		return make_pair((uint64)_mesh.get(), (uint64)_material.get());
	}
	InstanceID GetInstanceID_ModelMesh() {
		return make_pair((uint64)_model.get(), (uint64)_material.get());
	}

private:
	ComPtr<ID3D11Device> _device;

	friend class RenderManager;

	// Mesh
	shared_ptr<Mesh> _mesh;
	shared_ptr<Material> _material;
	shared_ptr<Model> _model;

	RasterizerStateInfo _rasterzerStates;
	vector<shared_ptr<RenderPass>> _renderPasses;
	bool _isUseEnvironmentMap = false;
};

