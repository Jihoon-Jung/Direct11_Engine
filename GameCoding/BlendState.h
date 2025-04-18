#pragma once
class BlendState
{
public:
	BlendState();
	~BlendState();
	void CreateBlendState();
	void CreateAdditiveBlendState();
	ComPtr<ID3D11BlendState> GetBlendState() { return _blendState; }
private:
	ComPtr<ID3D11BlendState> _blendState;
};

