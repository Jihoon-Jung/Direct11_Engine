#include "pch.h"
#include "RasterizerState.h"

RasterizerState::RasterizerState()
{
}

RasterizerState::~RasterizerState()
{
}

void RasterizerState::CreateRasterizerState(RasterizerStateInfo rasterizerStateInfo)
{
	D3D11_RASTERIZER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.FillMode = rasterizerStateInfo.fillMode;
	desc.CullMode = rasterizerStateInfo.cullMode;
	desc.FrontCounterClockwise = rasterizerStateInfo.frontCounterClockwise;
	HRESULT hr = DEVICE->CreateRasterizerState(&desc, _rasterizerState.GetAddressOf());
	CHECK(hr);
}
