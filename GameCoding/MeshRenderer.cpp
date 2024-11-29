#include "pch.h"
#include "MeshRenderer.h"

MeshRenderer::MeshRenderer()
	:Super(ComponentType::MeshRenderer)
{
}

MeshRenderer::~MeshRenderer()
{
}

void MeshRenderer::Update()
{
	if (_material != nullptr)
		_material->PushMaterialDesc();
}

void MeshRenderer::SetMaterial(shared_ptr<Material> material)
{
	_material = material;
	if (material != nullptr)
	{
		wstring name = RESOURCE.GetResourceName<Material>(material);
		_material->SetMaterialName(name);
	}
}

void MeshRenderer::SetMesh(shared_ptr<Mesh> mesh)
{
	_mesh = mesh;
	if (mesh != nullptr)
	{
		wstring name = RESOURCE.GetResourceName<Mesh>(mesh);
		_mesh->SetMeshName(name);
	}
}

void MeshRenderer::SetModel(shared_ptr<Model> model)
{
	_model = model;
	if (model != nullptr)
	{
		wstring name = RESOURCE.GetResourceName<Model>(model);
		_model->SetModelName(name);
	}
}

void MeshRenderer::SetRasterzierState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, bool frontCounterClockwise)
{
	_rasterzerStates.fillMode = fillMode;
	_rasterzerStates.cullMode = cullMode;
	_rasterzerStates.frontCounterClockwise = frontCounterClockwise;
}

void MeshRenderer::AddRenderPass()
{
	shared_ptr<RenderPass> pass = make_shared<RenderPass>();
	_renderPasses.push_back(pass);
}

shared_ptr<Buffer> MeshRenderer::GetMaterialBuffer()
{
	return _material->GetMaterialBuffer();
}
