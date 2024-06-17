#pragma once
class RasterizerState
{
public:
	RasterizerState();
	~RasterizerState();
	void CreateRasterizerState(RasterzerStates rasterizerState);
	ComPtr<ID3D11RasterizerState> GetRasterizerState() { return _rasterizerState; }
private:
	ComPtr<ID3D11RasterizerState> _rasterizerState;
};

