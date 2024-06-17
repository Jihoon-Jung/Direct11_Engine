#include "pch.h"
//#include "Engine.h"
//
//Engine::Engine()
//{
//}
//
//Engine::~Engine()
//{
//}
//
//void Engine::Init(HWND hwnd, int width, int height)
//{
//	_hwnd = hwnd;
//	rectWidth = width;
//	rectHeight = height;
//	CreateDeviceAndSwapChain();
//	CreateViewport();
//	CreateRenderTarget();
//
//	CreateRect();
//	CreateConstantBuffer();
//	CreateVertexShader();
//	CreateInputLayout();
//	CreatePixelShader();
//
//	CreateRasterizerState();
//	CreateSamplerState();
//	CreateShaderResourceView();
//	CreateBlendState();
//
//}
//
//void Engine::Render()
//{
//	SetRenderTarget();
//	SetViewport();
//
//	uint32 stride = sizeof(Vertex);
//	uint32 offset = 0;
//
//	// inputAssembler
//	_deviceContext->IASetVertexBuffers(0, 1, _vertexBuffer.GetAddressOf(), &stride, &offset);
//	_deviceContext->IASetIndexBuffer(_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
//
//	_deviceContext->IASetInputLayout(_inputLayout.Get());
//	_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	// VertexShader
//	_deviceContext->VSSetShader(_vertexShader.Get(), nullptr, 0);
//	_deviceContext->VSSetConstantBuffers(0, 1, _constantBuffer.GetAddressOf());
//
//	// Rasterizer
//	_deviceContext->RSSetState(_rasterizerState.Get());
//
//	// PixelShader
//	_deviceContext->PSSetShader(_pixelShader.Get(), nullptr, 0);
//	_deviceContext->PSSetShaderResources(0, 1, _shaderResourceView.GetAddressOf());
//	_deviceContext->PSSetSamplers(0, 1, _samplerState.GetAddressOf());
//
//	// O
//	_deviceContext->OMSetBlendState(_blendState.Get(), nullptr, 0xFFFFFFFF);
//
//	_deviceContext->DrawIndexed(_indices.size(), 0, 0);
//
//	SwapChain();
//}
//
//void Engine::Update()
//{
//	//_localPosition.x += 0.001f;
//	Matrix Scale = Matrix::CreateScale(_localScale);
//	Matrix Rotation = Matrix::CreateRotationX(_localRotation.x);
//	Rotation *= Matrix::CreateRotationY(_localRotation.y);
//	Rotation *= Matrix::CreateRotationZ(_localRotation.z);
//	Matrix Transform = Matrix::CreateTranslation(_localPosition);
//
//	Matrix matWorld = Scale * Rotation * Transform;
//	_transformData.matWorld = matWorld;
//
//	MakeViewAndProjectionMatrix();
//
//	D3D11_MAPPED_SUBRESOURCE subResource;
//	ZeroMemory(&subResource, sizeof(subResource));
//
//	_deviceContext->Map(_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
//	::memcpy(subResource.pData, &_transformData, sizeof(_transformData));
//	_deviceContext->Unmap(_constantBuffer.Get(), 0);
//}
//void Engine::MakeViewAndProjectionMatrix()
//{
//	zValue += 0.1f;
//	Vec3 cameraScale = { 1.0f, 1.0f, 1.0f };
//	Vec3 cameraPosition = { 0.0f, 0.0f, -2.0f };
//	Vec3 cameraRotation = { 0.0f, 0.0f, 0.0f };
//
//	Matrix CameraScale = Matrix::CreateScale(cameraScale);
//	Matrix CameraRotation = Matrix::CreateRotationX(cameraRotation.x);
//	CameraRotation *= Matrix::CreateRotationY(cameraRotation.y);
//	CameraRotation *= Matrix::CreateRotationZ(cameraRotation.z);
//
//	Matrix CameraTranslation = Matrix::CreateTranslation(cameraPosition);
//
//	Matrix CameraWorldMatrix = CameraScale * CameraRotation * CameraTranslation;
//
//	// set view matrix
//	Vec3 eye = cameraPosition;
//	Vec3 at = _localPosition - cameraPosition;
//	Vec3 up = CameraWorldMatrix.Up();
//
//	_transformData.matView = XMMatrixLookAtLH(eye, at, up);
//
//	//_transformData.matProjection = ::XMMatrixOrthographicLH(8, 6, 0.f, 1.f);
//	_transformData.matProjection = ::XMMatrixPerspectiveFovLH(XM_PI / 4.f, (float)rectWidth / (float)rectHeight, 1.f, 100.f);
//}
//void Engine::CreateDeviceAndSwapChain()
//{
//	DXGI_SWAP_CHAIN_DESC desc;
//	ZeroMemory(&desc, sizeof(desc));
//	{
//		int32 _width = 800;
//		int32 _height = 600;
//		desc.BufferDesc.Width = _width;
//		desc.BufferDesc.Height = _height;
//		desc.BufferDesc.RefreshRate.Numerator = 60;
//		desc.BufferDesc.RefreshRate.Denominator = 1;
//		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
//		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
//		desc.SampleDesc.Count = 1;
//		desc.SampleDesc.Quality = 0;
//		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//		desc.BufferCount = 1;
//		desc.OutputWindow = _hwnd;
//		desc.Windowed = true;
//		desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
//	}
//
//	HRESULT hr = ::D3D11CreateDeviceAndSwapChain(
//		nullptr,
//		D3D_DRIVER_TYPE_HARDWARE,
//		nullptr,
//		0,
//		nullptr,
//		0,
//		D3D11_SDK_VERSION,
//		&desc,
//		_swapChain.GetAddressOf(),
//		_device.GetAddressOf(),
//		nullptr,
//		_deviceContext.GetAddressOf()
//	);
//
//	CHECK(hr);
//}
//
//void Engine::CreateRenderTarget()
//{
//	HRESULT hr;
//
//	ComPtr<ID3D11Texture2D> backBuffer = nullptr;
//	// 후면 버퍼를 텍스처타입으로 받기. swapChain에게 후면버퍼를 backBuffer변수에 텍스처 타입으로 가지고와달라는것.
//	hr = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBuffer.GetAddressOf());
//
//	CHECK(hr);
//	// _renderTargetView를 device를 통해 ID3D11Texture2D타입으로 받도록 생성.
//	// 가저온 backbuffer변수를 통해 _renderTargetView를 발급받기. gpu에게 _renderTargetView를 쓱 건내주면 gpu가 후면버퍼에 그려줌
//	_device->CreateRenderTargetView(backBuffer.Get(), nullptr, _renderTargetView.GetAddressOf());
//}
//
//void Engine::CreateViewport()
//{
//	_viewport.TopLeftX = 0.f;
//	_viewport.TopLeftY = 0.f;
//	_viewport.Width = static_cast<float>(800);
//	_viewport.Height = static_cast<float>(600);
//	_viewport.MinDepth = 0.f;
//	_viewport.MaxDepth = 1.f;
//}
//
//void Engine::SetViewport()
//{
//	_deviceContext->RSSetViewports(1, &_viewport);
//}
//
//void Engine::SetRenderTarget()
//{
//	_deviceContext->OMSetRenderTargets(1, _renderTargetView.GetAddressOf(), nullptr);
//	_deviceContext->ClearRenderTargetView(_renderTargetView.Get(), _clearColor);
//}
//
//void Engine::SwapChain()
//{
//	HRESULT hr = _swapChain->Present(1, 0);
//	CHECK(hr);
//}
//
//void Engine::CreateRect()
//{
//	_vertices.resize(4);
//
//	_vertices[0].position = Vec3(-0.5f, -0.5f, 0.0f);
//	_vertices[0].uv = Vec2(0.0f, 1.0f);
//	_vertices[1].position = Vec3(0.5f, -0.5f, 0.0f);
//	_vertices[1].uv = Vec2(1.0f, 1.0f);
//	_vertices[2].position = Vec3(-0.5f, 0.5f, 0.0f);
//	_vertices[2].uv = Vec2(0.0f, 0.0f);
//	_vertices[3].position = Vec3(0.5f, 0.5f, 0.0f);
//	_vertices[3].uv = Vec2(1.0f, 0.0f);
//
//	{
//		D3D11_BUFFER_DESC desc;
//		ZeroMemory(&desc, sizeof(desc));
//		desc.Usage = D3D11_USAGE_IMMUTABLE;
//		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//		desc.ByteWidth = (uint32)(sizeof(Vertex) * _vertices.size());
//
//		D3D11_SUBRESOURCE_DATA data;
//		ZeroMemory(&data, sizeof(data));
//		data.pSysMem = _vertices.data();
//
//		HRESULT hr = _device->CreateBuffer(&desc, &data, _vertexBuffer.GetAddressOf());
//		CHECK(hr);
//	}
//
//	// ccw 방향으로 만들어야해서
//	_indices = { 0, 2, 1, 1, 2, 3 };
//
//	{
//		D3D11_BUFFER_DESC desc;
//		ZeroMemory(&desc, sizeof(desc));
//		desc.Usage = D3D11_USAGE_IMMUTABLE;
//		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
//		desc.ByteWidth = (uint32)(sizeof(uint32) * _indices.size());
//
//		D3D11_SUBRESOURCE_DATA data;
//		ZeroMemory(&data, sizeof(data));
//		data.pSysMem = _indices.data();
//
//		HRESULT hr = _device->CreateBuffer(&desc, &data, _indexBuffer.GetAddressOf());
//		CHECK(hr);
//	}
//	
//
//}
//
//void Engine::CreateInputLayout()
//{
//	D3D11_INPUT_ELEMENT_DESC layout[] =
//	{
//		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
//	};
//
//	const int32 count = sizeof(layout) / sizeof(D3D11_INPUT_ELEMENT_DESC);
//	_device->CreateInputLayout(layout, count, _vsBlob->GetBufferPointer(), _vsBlob->GetBufferSize(), _inputLayout.GetAddressOf());
//}
//
//void Engine::CreateVertexShader()
//{
//	LoadShaderFromFile(L"Default.hlsl", "VS", "vs_5_0", _vsBlob);
//	HRESULT hr = _device->CreateVertexShader(_vsBlob->GetBufferPointer(), _vsBlob->GetBufferSize(), nullptr, _vertexShader.GetAddressOf());
//}
//
//void Engine::CreatePixelShader()
//{
//	LoadShaderFromFile(L"Default.hlsl", "PS", "ps_5_0", _psBlob);
//	HRESULT hr = _device->CreatePixelShader(_psBlob->GetBufferPointer(), _psBlob->GetBufferSize(), nullptr, _pixelShader.GetAddressOf());
//}
//
//void Engine::CreateRasterizerState()
//{
//	D3D11_RASTERIZER_DESC desc;
//	ZeroMemory(&desc, sizeof(desc));
//	desc.FillMode = D3D11_FILL_WIREFRAME;// D3D11_FILL_SOLID;
//	desc.CullMode = D3D11_CULL_BACK;
//	desc.FrontCounterClockwise = false;
//
//	HRESULT hr = _device->CreateRasterizerState(&desc, _rasterizerState.GetAddressOf());
//	CHECK(hr);
//}
//
//void Engine::CreateSamplerState()
//{
//	D3D11_SAMPLER_DESC desc;
//	ZeroMemory(&desc, sizeof(desc));
//	desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
//	desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
//	desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
//	desc.BorderColor[0] = 1;
//	desc.BorderColor[1] = 0;
//	desc.BorderColor[2] = 0;
//	desc.BorderColor[3] = 1;
//	desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
//	desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
//	desc.MaxAnisotropy = 16;
//	desc.MaxLOD = FLT_MAX;
//	desc.MinLOD = FLT_MIN;
//	desc.MipLODBias = 0.0f;
//
//	_device->CreateSamplerState(&desc, _samplerState.GetAddressOf());
//}
//
//void Engine::CreateBlendState()
//{
//	D3D11_BLEND_DESC desc;
//	ZeroMemory(&desc, sizeof(D3D11_BLEND_DESC));
//	desc.AlphaToCoverageEnable = false;
//	desc.IndependentBlendEnable = false;
//
//	desc.RenderTarget[0].BlendEnable = true;
//	desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
//	desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
//	desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
//	desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
//	desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
//	desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
//	desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
//
//	HRESULT hr = _device->CreateBlendState(&desc, _blendState.GetAddressOf());
//	CHECK(hr);
//}
//
//void Engine::CreateShaderResourceView()
//{
//	DirectX::TexMetadata md;
//	DirectX::ScratchImage img;
//	HRESULT hr = ::LoadFromWICFile(L"Skeleton.png", WIC_FLAGS_NONE, &md, img);
//	CHECK(hr);
//
//	hr = ::CreateShaderResourceView(_device.Get(), img.GetImages(), img.GetImageCount(), md, _shaderResourceView.GetAddressOf());
//	CHECK(hr);
//}
//
//void Engine::CreateConstantBuffer()
//{
//	D3D11_BUFFER_DESC desc;
//	ZeroMemory(&desc, sizeof(desc));
//	desc.Usage = D3D11_USAGE_DYNAMIC;
//	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
//	desc.ByteWidth = sizeof(TransformData);
//	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//
//	HRESULT hr = _device->CreateBuffer(&desc, nullptr, _constantBuffer.GetAddressOf());
//}
//
//void Engine::LoadShaderFromFile(const wstring& path, const string& name, const string& version, ComPtr<ID3DBlob>& blob)
//{
//	const uint32 compileFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
//
//	HRESULT hr = ::D3DCompileFromFile(
//		path.c_str(),
//		nullptr,
//		D3D_COMPILE_STANDARD_FILE_INCLUDE,
//		name.c_str(),
//		version.c_str(),
//		compileFlag,
//		0,
//		blob.GetAddressOf(),
//		nullptr);
//
//	CHECK(hr);
//}


