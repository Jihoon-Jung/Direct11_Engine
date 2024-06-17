#pragma once
class SamplerState
{
public:
	SamplerState();
	~SamplerState();
	void CreateSamplerState();
	ComPtr<ID3D11SamplerState> GetSamplerState() { return _samplerState; }

private:
	ComPtr<ID3D11SamplerState> _samplerState;
};

