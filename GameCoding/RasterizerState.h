#pragma once
struct RasterizerStateInfo;

class RasterizerState
{
public:
	RasterizerState();
	~RasterizerState();
	void CreateRasterizerState(RasterizerStateInfo rasterizerStateInfo);
	ComPtr<ID3D11RasterizerState> GetRasterizerState() { return _rasterizerState; }
private:
	ComPtr<ID3D11RasterizerState> _rasterizerState;
};

