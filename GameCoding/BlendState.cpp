#include "pch.h"
#include "BlendState.h"

BlendState::BlendState()
{

}

BlendState::~BlendState()
{
}

void BlendState::CreateBlendState()
{
	D3D11_BLEND_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BLEND_DESC));
	desc.AlphaToCoverageEnable = false;
	desc.IndependentBlendEnable = false;

	desc.RenderTarget[0].BlendEnable = TRUE;
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	HRESULT hr = DEVICE->CreateBlendState(&desc, _blendState.GetAddressOf());
	CHECK(hr);
}
void BlendState::CreateAdditiveBlendState()
{
	D3D11_BLEND_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_BLEND_DESC));
	desc.AlphaToCoverageEnable = FALSE;
	desc.IndependentBlendEnable = FALSE;

	desc.RenderTarget[0].BlendEnable = TRUE;
	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;       // SrcBlend = SRC_ALPHA
	desc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;            // DestBlend = ONE
	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;           // BlendOp = ADD

	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;       // SrcBlendAlpha = ZERO
	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;      // DestBlendAlpha = ZERO
	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;      // BlendOpAlpha = ADD

	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL; // RenderTargetWriteMask[0] = 0x0F

	HRESULT hr = DEVICE->CreateBlendState(&desc, _blendState.GetAddressOf());
	CHECK(hr);
}