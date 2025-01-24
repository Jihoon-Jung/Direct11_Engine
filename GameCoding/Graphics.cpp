#include "pch.h"
#include "Graphics.h"

void Graphics::CreateDeviceAndSwapChain()
{
	DXGI_SWAP_CHAIN_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	{
		desc.BufferDesc.Width = GetProjectWidth();
		desc.BufferDesc.Height = GetProjectHeight();
		desc.BufferDesc.RefreshRate.Numerator = 60;
		desc.BufferDesc.RefreshRate.Denominator = 1;
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		desc.SampleDesc.Count = 4; // MSAA 4x 설정
		desc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
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
	hr = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBuffer.GetAddressOf());
	CHECK(hr);

	hr = _device->CreateRenderTargetView(backBuffer.Get(), nullptr, _renderTargetView.GetAddressOf());
	CHECK(hr);
}

void Graphics::CreateOffscreenRenderTarget()
{
	D3D11_TEXTURE2D_DESC texDesc;

	texDesc.Width = GetProjectWidth();
	texDesc.Height = GetProjectHeight();
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> offscreenTex = 0;
	HRESULT hr = DEVICE->CreateTexture2D(&texDesc, 0, offscreenTex.GetAddressOf());
	CHECK(hr);

	// Null description means to create a view to all mipmap levels using 
	// the format the texture was created with.
	hr = DEVICE->CreateShaderResourceView(offscreenTex.Get(), 0, _offscreenSRV.GetAddressOf());
	CHECK(hr);
	hr = DEVICE->CreateRenderTargetView(offscreenTex.Get(), 0, _offscreenRTV.GetAddressOf());
	CHECK(hr);

	// depthStenciView 생성

	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = GetProjectWidth();  // 렌더 타겟과 동일한 너비
	depthDesc.Height = GetProjectHeight(); // 렌더 타겟과 동일한 높이
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	hr = DEVICE->CreateTexture2D(&depthDesc, nullptr, &_offScreenDepthStencilTexture);
	CHECK(hr);

	hr = DEVICE->CreateDepthStencilView(_offScreenDepthStencilTexture.Get(), nullptr, &_offScreenDepthStencilView);
	CHECK(hr);

}


void Graphics::CreateDepthStencilView()
{
	{
		D3D11_TEXTURE2D_DESC desc = { 0 };
		ZeroMemory(&desc, sizeof(desc));
		desc.Width = static_cast<uint32>(GP.GetProjectWidth());
		desc.Height = static_cast<uint32>(GP.GetProjectHeight());
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		desc.SampleDesc.Count = 4; // MSAA 4x 설정
		desc.SampleDesc.Quality = D3D11_STANDARD_MULTISAMPLE_PATTERN;
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
		desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS; // 멀티샘플러 텍스처로 설정
		desc.Texture2D.MipSlice = 0;

		HRESULT hr = DEVICE->CreateDepthStencilView(_depthStencilTexture.Get(), &desc, _depthStencilView.GetAddressOf());
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
	ClearDepthStencilView();
}

void Graphics::SetOffscreenRenderTarget()
{
	// viewport 설정
	D3D11_VIEWPORT viewport = {};
	viewport.Width = GetProjectWidth();
	viewport.Height = GetProjectHeight();
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	float _clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };


	DEVICECONTEXT->RSSetViewports(1, &viewport);
	DEVICECONTEXT->OMSetRenderTargets(1, _offscreenRTV.GetAddressOf(), _offScreenDepthStencilView.Get());
	DEVICECONTEXT->ClearRenderTargetView(_offscreenRTV.Get(), _clearColor);
	DEVICECONTEXT->ClearDepthStencilView(_offScreenDepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Graphics::SetShadowMapRenderTarget()
{
	// viewport 설정
	D3D11_VIEWPORT viewport = {};
	viewport.Width = GetProjectWidth();//GetViewWidth();
	viewport.Height = GetProjectHeight();//GetViewHeight();
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	float _clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };


	DEVICECONTEXT->RSSetViewports(1, &viewport);
	// Set null render target because we are only going to draw to depth buffer.
	// Setting a null render target will disable color writes.
	ID3D11RenderTargetView* renderTargets[1] = { 0 };

	DEVICECONTEXT->OMSetRenderTargets(1, renderTargets, _depthMapDSV.Get());
	DEVICECONTEXT->ClearDepthStencilView(_depthMapDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

}

void Graphics::CreateShadowMapRenderTarget()
{
	// Use typeless format because the DSV is going to interpret
	// the bits as DXGI_FORMAT_D24_UNORM_S8_UINT, whereas the SRV is going to interpret
	// the bits as DXGI_FORMAT_R24_UNORM_X8_TYPELESS.
	D3D11_TEXTURE2D_DESC texDesc;
	texDesc.Width = GetProjectWidth();//GetViewWidth();
	texDesc.Height = GetProjectHeight();//GetViewHeight();
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R24G8_TYPELESS;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> depthMap;
	HRESULT hr = DEVICE->CreateTexture2D(&texDesc, 0, depthMap.GetAddressOf());

	CHECK(hr);

	D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc;
	dsvDesc.Flags = 0;
	dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvDesc.Texture2D.MipSlice = 0;
	hr = DEVICE->CreateDepthStencilView(depthMap.Get(), &dsvDesc, _depthMapDSV.GetAddressOf());

	CHECK(hr);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = texDesc.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	hr = DEVICE->CreateShaderResourceView(depthMap.Get(), &srvDesc, _depthMapSRV.GetAddressOf());

	CHECK(hr);
}

void Graphics::ClearDepthStencilView()
{
	_deviceContext->ClearDepthStencilView(_depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1, 0);
}

void Graphics::SwapChain()
{
	HRESULT hr = _swapChain->Present(1, 0);
	CHECK(hr);
}

void Graphics::RestoreRenderTarget()
{
	_deviceContext->OMSetRenderTargets(1, _renderTargetView.GetAddressOf(), _depthStencilView.Get());
	SetViewport();
}

void Graphics::RenderQuad()
{
	shared_ptr<Material> material = make_shared<Material>();

	shared_ptr<Shader> computeShader_gaussianBlurHorizontal = RESOURCE.GetResource<Shader>(L"Gaussian_Horizontal");
	shared_ptr<Shader> computeShader_gaussianBlurVertical = RESOURCE.GetResource<Shader>(L"Gaussian_Vertical");
	ComPtr<ID3D11ShaderResourceView> convertedSRV = material->GaussainBlur(computeShader_gaussianBlurVertical, computeShader_gaussianBlurHorizontal, _offscreenSRV);

	RestoreRenderTarget();
	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	shared_ptr<Mesh> mesh = RESOURCE.GetResource<Mesh>(L"Quad");
	shared_ptr<Shader> shader = RESOURCE.GetResource<Shader>(L"Quad_Shader");
	shared_ptr<Buffer> buffer = mesh->GetBuffer();
	uint32 stride = buffer->GetStride();
	uint32 offset = 0;

	shared_ptr<InputLayout> inputLayout = shader->GetInputLayout();
	shared_ptr<RasterizerState> rasterizerState = make_shared<RasterizerState>();

	RasterizerStateInfo rasterzerStates;
	rasterzerStates.fillMode = D3D11_FILL_SOLID;
	rasterzerStates.cullMode = D3D11_CULL_BACK;
	rasterzerStates.frontCounterClockwise = false;
	rasterizerState->CreateRasterizerState(rasterzerStates);

	// inputAssembler
	DEVICECONTEXT->IASetVertexBuffers(0, 1, buffer->GetVertexBuffer().GetAddressOf(), &stride, &offset);
	DEVICECONTEXT->IASetIndexBuffer(buffer->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

	DEVICECONTEXT->IASetInputLayout(shader->GetInputLayout()->GetInputLayout().Get());
	DEVICECONTEXT->IASetPrimitiveTopology(topology);

	// VertexShader
	DEVICECONTEXT->VSSetShader(shader->GetVertexShader().Get(), nullptr, 0);

	// Rasterizer
	DEVICECONTEXT->RSSetState(rasterizerState->GetRasterizerState().Get());

	// PixelShader
	DEVICECONTEXT->PSSetShader(shader->GetPixelShader().Get(), nullptr, 0);


	// Set Default Texture
	shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"texture0", 1, convertedSRV);

	shared_ptr<DepthStencilState> depthStencilState = make_shared<DepthStencilState>();
	depthStencilState->SetDepthStencilState(DSState::NORMAL);

	shared_ptr<SamplerState> samplerState = make_shared<SamplerState>();
	samplerState->CreateSamplerState();

	shared_ptr<BlendState> blendState = make_shared<BlendState>();
	blendState->CreateBlendState();

	DEVICECONTEXT->PSSetSamplers(0, 1, samplerState->GetSamplerState().GetAddressOf());

	// OutputMerger
	DEVICECONTEXT->OMSetBlendState(blendState->GetBlendState().Get(), nullptr, 0xFFFFFFFF);
	DEVICECONTEXT->OMSetDepthStencilState(depthStencilState->GetDepthStecilState().Get(), 1);
	DEVICECONTEXT->DrawIndexed(mesh->GetGeometry()->GetIndices().size(), 0, 0);

}
