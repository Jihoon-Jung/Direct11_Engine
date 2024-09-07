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

	ComPtr<ID3D11ShaderResourceView> _offscreenSRV;
	ComPtr<ID3D11RenderTargetView> _offscreenRTV;
	ComPtr<ID3D11Texture2D> _offScreenDepthStencilTexture;
	ComPtr<ID3D11DepthStencilView> _offScreenDepthStencilView;

	//D3D11_VIEWPORT _viewport = { 0 };
	Viewport _viewport;
	float _clearColor[4] = { 0.2f, 0.1f, 0.3f, 1.0f };


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
		CreateOffscreenRenderTarget();
		CreateDepthStencilView();
		CreateViewport(width, height, 0, 0, 0, 1);
		
	}
	int GetViewWidth() { return _viewWidth; }
	int GetViewHeight() { return _viewHeight; }
	int GetEnvironmentMapWidth() { return _environmentMapWidth; }
	int GetEnvironmentMapHeight() { return _environmentMapHeight; }
	void CreateDeviceAndSwapChain();
	void CreateRenderTarget();
	void CreateOffscreenRenderTarget();
	void CreateDepthStencilView();
	void CreateViewport(float width, float height, float x, float y, float minDepth, float maxDepth);
	void SetViewport();
	void SetRenderTarget();
	void SetOffscreenRenderTarget();
	void ClearDepthStencilView();
	void SwapChain();
	void RestoreRenderTarget();
	void RenderQuad();
	ComPtr<ID3D11Device> GetDevice() { return _device; }
	ComPtr<ID3D11DeviceContext> GetDeviceContext() { return _deviceContext; }
	int _viewWidth = 0;
	int _viewHeight = 0;

	int _environmentMapWidth = 256;
	int _environmentMapHeight = 256;

	uint32    _4xMsaaQuality = 0;
};

