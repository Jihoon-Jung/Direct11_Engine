#pragma once
#include "Model.h"

struct RasterizerStates
{
	D3D11_FILL_MODE fillMode;
	D3D11_CULL_MODE cullMode;
	bool frontCouterClockWise;
};
class MeshRenderer : public Component
{
	using Super = Component;

public:
	MeshRenderer();
	virtual ~MeshRenderer();
	virtual void Update() override;
	void SetMaterial(shared_ptr<Material> material) { _material = material; }
	void SetMesh(shared_ptr<Mesh> mesh) { _mesh = mesh; }
	void SetTexture(shared_ptr<Texture> texture) { _material->SetTexture(texture); };
	void SetModel(shared_ptr<Model> model) { _model = model; }
	void SetRasterzierState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, bool frontCouterClockWise);

	auto GetMaterial() { return _material; }
	auto GetShader() { return GetMaterial()->GetShader(); }
	auto GetInputLayout() { return GetShader()->GetInputLayout(); }
	auto GetRasterzerStates() { return _rasterzerStates; }

	shared_ptr<Mesh> GetMesh() { return _mesh; }
	shared_ptr<Model> GetModel() { return _model; }
	shared_ptr<Texture> GetTexture() { return GetMaterial()->GetTexture(); }
	shared_ptr<Buffer> GetMaterialBuffer();
private:
	ComPtr<ID3D11Device> _device;

	friend class RenderManager;

	// Mesh
	shared_ptr<Mesh> _mesh;
	shared_ptr<Material> _material;
	shared_ptr<Model> _model;

	RasterizerStates _rasterzerStates;

};

