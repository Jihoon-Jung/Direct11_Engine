#include "pch.h"
#include "SamplerState.h"

SamplerState::SamplerState()
{
}

SamplerState::~SamplerState()
{
}

void SamplerState::CreateSamplerState()
{
	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.BorderColor[0] = 1;
	desc.BorderColor[1] = 0;
	desc.BorderColor[2] = 0;
	desc.BorderColor[3] = 1;
	desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	desc.MaxAnisotropy = 16;
	desc.MaxLOD = FLT_MAX;
	desc.MinLOD = FLT_MIN;
	desc.MipLODBias = 0.0f;

	DEVICE->CreateSamplerState(&desc, _samplerState.GetAddressOf());
}

void SamplerState::CreateSamplerComparisonState()
{
	D3D11_SAMPLER_DESC desc = {};
	desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT; // 필터링 방식 설정
	desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER; // U 좌표에 대한 Border 모드 설정
	desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER; // V 좌표에 대한 Border 모드 설정
	desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER; // W 좌표에 대한 Border 모드 설정
	desc.BorderColor[0] = 0.0f; // Border 색상 (R = 0.0)
	desc.BorderColor[1] = 0.0f; // Border 색상 (G = 0.0)
	desc.BorderColor[2] = 0.0f; // Border 색상 (B = 0.0)
	desc.BorderColor[3] = 0.0f; // Border 색상 (A = 0.0)
	desc.ComparisonFunc = D3D11_COMPARISON_LESS; // 비교 함수 설정
	desc.MinLOD = 0.0f; // 최소 LOD 설정
	desc.MaxLOD = D3D11_FLOAT32_MAX; // 최대 LOD 설정

	DEVICE->CreateSamplerState(&desc, _sampleCmpState.GetAddressOf());
}

void SamplerState::CreateShadowSamplerState()
{
	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	desc.BorderColor[0] = 1.0f;
	desc.BorderColor[1] = 1.0f;
	desc.BorderColor[2] = 1.0f;
	desc.BorderColor[3] = 1.0f;
	desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
	desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
	desc.MinLOD = 0;
	desc.MaxLOD = D3D11_FLOAT32_MAX;

	DEVICE->CreateSamplerState(&desc, _sampleCmpState.GetAddressOf());
}

void SamplerState::CreateHeightMapSamplerState()
{
	D3D11_SAMPLER_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	desc.BorderColor[0] = 1;
	desc.BorderColor[1] = 0;
	desc.BorderColor[2] = 0;
	desc.BorderColor[3] = 1;
	desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	desc.Filter = D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
	desc.MaxAnisotropy = 16;
	desc.MaxLOD = FLT_MAX;
	desc.MinLOD = FLT_MIN;
	desc.MipLODBias = 0.0f;

	DEVICE->CreateSamplerState(&desc, _samplerState.GetAddressOf());
}
