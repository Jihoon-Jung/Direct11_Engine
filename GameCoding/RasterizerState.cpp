#include "pch.h"
#include "RasterizerState.h"

RasterizerState::RasterizerState()
{
}

RasterizerState::~RasterizerState()
{
}

void RasterizerState::CreateRasterizerState(RasterzerStates rasterizerState)
{
	D3D11_RASTERIZER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.FillMode = rasterizerState.fillMode;
	desc.CullMode = rasterizerState.cullMode;
	desc.FrontCounterClockwise = rasterizerState.frontCouterClockWise;

	HRESULT hr = DEVICE->CreateRasterizerState(&desc, _rasterizerState.GetAddressOf());
	CHECK(hr);
}
