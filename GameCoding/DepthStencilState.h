#pragma once

enum class DSState
{
	NORMAL,
	CUSTOM1,
	CUStom2
};

class DepthStencilState
{
public:
	DepthStencilState();
	~DepthStencilState();
	void SetDepthStencilState(DSState state);
	ComPtr<ID3D11DepthStencilState> GetDepthStecilState() { return _depthStencilState; }
private:

	ComPtr<ID3D11DepthStencilState> _depthStencilState;
};

