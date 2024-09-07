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
	_material->PushMaterialDesc();
}

void MeshRenderer::SetRasterzierState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, bool frontCouterClockWise)
{
	_rasterzerStates.fillMode = fillMode;
	_rasterzerStates.cullMode = cullMode;
	_rasterzerStates.frontCouterClockWise = frontCouterClockWise;
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
