#include "pch.h"
#include "Graphics.h"

void Graphics::CreateDeviceAndSwapChain()
{
	DXGI_SWAP_CHAIN_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	{
		int32 _width = 800;
		int32 _height = 600;
		desc.BufferDesc.Width = _width;
		desc.BufferDesc.Height = _height;
		desc.BufferDesc.RefreshRate.Numerator = 60;
		desc.BufferDesc.RefreshRate.Denominator = 1;
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = 1;
		desc.OutputWindow = _hwnd;
		desc.Windowed = true;
		desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	}

	HRESULT hr = ::D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&desc,
		_swapChain.GetAddressOf(),
		_device.GetAddressOf(),
		nullptr,
		_deviceContext.GetAddressOf()
	);

	CHECK(hr);
}

void Graphics::CreateRenderTarget()
{
	HRESULT hr;

	ComPtr<ID3D11Texture2D> backBuffer = nullptr;
	// 후면 버퍼를 텍스처타입으로 받기. swapChain에게 후면버퍼를 backBuffer변수에 텍스처 타입으로 가지고와달라는것.
	hr = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBuffer.GetAddressOf());

	CHECK(hr);
	// _renderTargetView를 device를 통해 ID3D11Texture2D타입으로 받도록 생성.
	// 가저온 backbuffer변수를 통해 _renderTargetView를 발급받기. gpu에게 _renderTargetView를 쓱 건내주면 gpu가 후면버퍼에 그려줌
	_device->CreateRenderTargetView(backBuffer.Get(), nullptr, _renderTargetView.GetAddressOf());
}

void Graphics::CreateDepthStencilView()
{
	{
		D3D11_TEXTURE2D_DESC desc = { 0 };
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = static_cast<uint32>(GP.GetViewWidth());
		desc.Height = static_cast<uint32>(GP.GetViewHeight());
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		HRESULT hr = DEVICE->CreateTexture2D(&desc, nullptr, _depthStencilTexture.GetAddressOf());
		CHECK(hr);
	}

	{
		D3D11_DEPTH_STENCIL_VIEW_DESC desc;
		ZeroMemory(&desc, sizeof(desc));
		desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		desc.Texture2D.MipSlice = 0;

		HRESULT hr = DEVICE->CreateDepthStencilView(_depthStencilTexture.Get(), &desc, _depthStencilView.GetAddressOf());
		CHECK(hr);
	}
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

}

void Graphics::CreateViewport(float width, float height, float x, float y, float minDepth, float maxDepth)
{
	_viewport.Set(width, height, x, y, minDepth, maxDepth);
}

void Graphics::SetViewport()
{
	_viewport.RSSetViewport();
}

void Graphics::SetRenderTarget()
{
	_deviceContext->OMSetRenderTargets(1, _renderTargetView.GetAddressOf(), _depthStencilView.Get());
	_deviceContext->ClearRenderTargetView(_renderTargetView.Get(), _clearColor);
	SetDepthStencilView();
}

void Graphics::SetDepthStencilView()
{
	_deviceContext->OMSetDepthStencilState(_depthStencilState.Get(), 1);
	_deviceContext->ClearDepthStencilView(_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
}

void Graphics::SwapChain()
{
	HRESULT hr = _swapChain->Present(1, 0);
	CHECK(hr);
}
