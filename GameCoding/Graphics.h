#pragma once
#include "Viewport.h"

class Graphics
{
private:
	HWND _hwnd;
	ComPtr<ID3D11Device> _device = nullptr;
	ComPtr<ID3D11DeviceContext> _deviceContext = nullptr;
	ComPtr<IDXGISwapChain> _swapChain = nullptr;
	ComPtr<ID3D11RenderTargetView> _renderTargetView;

	ComPtr<ID3D11Texture2D> _depthStencilTexture;
	ComPtr<ID3D11DepthStencilView> _depthStencilView;
	ComPtr<ID3D11DepthStencilState> _depthStencilState;

	//D3D11_VIEWPORT _viewport = { 0 };
	Viewport _viewport;
	float _clearColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };


protected:
	Graphics() {}
	virtual ~Graphics() {}
public:
	Graphics(const Graphics&) = delete;
	static Graphics& GetInstance()
	{
		static Graphics instance;
		return instance;
	}
	Viewport& GetViewport() { return _viewport; }

	void Initialize(HWND hwnd, int width, int height)
	{
		_hwnd = hwnd;
		_viewWidth = width;
		_viewHeight = height;

		CreateDeviceAndSwapChain();
		CreateRenderTarget();
		CreateDepthStencilView();
		CreateViewport(width, height, 0, 0, 0, 1);
		
	}
	int GetViewWidth() { return _viewWidth; }
	int GetViewHeight() { return _viewHeight; }
	void CreateDeviceAndSwapChain();
	void CreateRenderTarget();
	void CreateDepthStencilView();
	void CreateViewport(float width, float height, float x, float y, float minDepth, float maxDepth);
	void SetViewport();
	void SetRenderTarget();
	void SetDepthStencilView();
	void SwapChain();
	ComPtr<ID3D11Device> GetDevice() { return _device; }
	ComPtr<ID3D11DeviceContext> GetDeviceContext() { return _deviceContext; }
	int _viewWidth = 0;
	int _viewHeight = 0;
};

