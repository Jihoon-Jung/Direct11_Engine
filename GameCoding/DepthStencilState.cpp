#include "pch.h"
#include "DepthStencilState.h"


DepthStencilState::DepthStencilState()
{
}

DepthStencilState::~DepthStencilState()
{
}

void DepthStencilState::SetDepthStencilState(DSState state)
{	
	if(state == DSState::NORMAL)
	{
		D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
		ZeroMemory(&depthStencilStateDesc, sizeof(depthStencilStateDesc));
		depthStencilStateDesc.DepthEnable = TRUE;
		depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
		depthStencilStateDesc.StencilEnable = FALSE;
		HRESULT hr = DEVICE->CreateDepthStencilState(&depthStencilStateDesc, _depthStencilState.GetAddressOf());
		CHECK(hr);
	}
	else if(state == DSState::CUSTOM1)
	{
		D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
		ZeroMemory(&depthStencilStateDesc, sizeof(depthStencilStateDesc));
		depthStencilStateDesc.DepthEnable = TRUE;
		depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
		depthStencilStateDesc.StencilEnable = TRUE;
		depthStencilStateDesc.StencilReadMask = 0xFF;
		depthStencilStateDesc.StencilWriteMask = 0xFF;

		// Stencil operations if pixel is front-facing
		depthStencilStateDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilStateDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthStencilStateDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		depthStencilStateDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Stencil operations if pixel is back-facing
		depthStencilStateDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilStateDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthStencilStateDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		depthStencilStateDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		HRESULT hr = DEVICE->CreateDepthStencilState(&depthStencilStateDesc, _depthStencilState.GetAddressOf());
		CHECK(hr);
	}
	else if (state == DSState::CUStom2)
	{
		D3D11_DEPTH_STENCIL_DESC depthStencilStateDesc;
		ZeroMemory(&depthStencilStateDesc, sizeof(depthStencilStateDesc));
		depthStencilStateDesc.DepthEnable = TRUE;
		depthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;
		depthStencilStateDesc.StencilEnable = TRUE;
		depthStencilStateDesc.StencilReadMask = 0xFF;
		depthStencilStateDesc.StencilWriteMask = 0xFF;

		// Stencil operations if pixel is front-facing
		depthStencilStateDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilStateDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthStencilStateDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilStateDesc.FrontFace.StencilFunc = D3D11_COMPARISON_NOT_EQUAL;

		// Stencil operations if pixel is back-facing
		depthStencilStateDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilStateDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthStencilStateDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_REPLACE;
		depthStencilStateDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		HRESULT hr = DEVICE->CreateDepthStencilState(&depthStencilStateDesc, _depthStencilState.GetAddressOf());
		CHECK(hr);
	}
}
