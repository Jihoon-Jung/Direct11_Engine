#pragma once
class SamplerState
{
public:
	SamplerState();
	~SamplerState();
	void CreateSamplerState();
	void CreateSamplerComparisonState();
	void CreateShadowSamplerState();
	void CreateHeightMapSamplerState();
	ComPtr<ID3D11SamplerState> GetSamplerState() { return _samplerState; }
	ComPtr<ID3D11SamplerState> GetSampleCmpState() { return _sampleCmpState; }
private:
	ComPtr<ID3D11SamplerState> _samplerState;
	ComPtr<ID3D11SamplerState> _sampleCmpState;
};

