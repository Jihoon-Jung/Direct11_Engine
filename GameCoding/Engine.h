#pragma once

struct Vertex
{
	Vec3 position;
	Vec2 uv;

};
struct TransformData
{
	Matrix matWorld = Matrix::Identity;
	Matrix matView = Matrix::Identity;
	Matrix matProjection = Matrix::Identity;

};

class Engine
{
public:
	Engine();
	~Engine();

	void Init(HWND hwnd, int width, int height);
	void Render();
	void Update();

	void CreateDeviceAndSwapChain();
	void CreateRenderTarget();
	void CreateViewport();
	void SetViewport();

	void SetRenderTarget();
	void SwapChain();

	void CreateRect();
	void CreateInputLayout();
	void CreateVertexShader();
	void CreatePixelShader();

	void CreateRasterizerState();
	void CreateSamplerState();
	void CreateBlendState();
	void CreateShaderResourceView();

	void CreateConstantBuffer();
	void LoadShaderFromFile(const wstring& path, const string& name, const string& version, ComPtr<ID3DBlob>& blob);

	void MakeViewAndProjectionMatrix();

private:
	HWND _hwnd;
	ComPtr<ID3D11Device> _device = nullptr;
	ComPtr<ID3D11DeviceContext> _deviceContext = nullptr;
	ComPtr<IDXGISwapChain> _swapChain = nullptr;

	ComPtr<ID3D11RenderTargetView> _renderTargetView;
	D3D11_VIEWPORT _viewport = { 0 };
	float _clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	vector<Vertex> _vertices;
	ComPtr<ID3D11Buffer> _vertexBuffer;
	vector<uint32> _indices;
	ComPtr<ID3D11Buffer> _indexBuffer;

	ComPtr<ID3D11Buffer> _constantBuffer;
	TransformData _transformData;

	ComPtr<ID3D11VertexShader> _vertexShader;
	ComPtr<ID3DBlob> _vsBlob = nullptr;

	ComPtr<ID3D11InputLayout> _inputLayout = nullptr;

	ComPtr<ID3D11RasterizerState> _rasterizerState;

	ComPtr<ID3D11PixelShader> _pixelShader;
	ComPtr<ID3DBlob> _psBlob;

	ComPtr<ID3D11ShaderResourceView> _shaderResourceView;

	ComPtr<ID3D11SamplerState> _samplerState;
	ComPtr<ID3D11BlendState> _blendState;

	Vec3 _localPosition = { 0.f, 0.f, 0.f };
	Vec3 _localRotation = { 0.f, 0.f, 0.f };
	Vec3 _localScale = { 1.f, 1.f, 1.f };

	Matrix viewMatrix = Matrix::Identity;
	Matrix projectionMatrix = Matrix::Identity;

	float zValue = -10.0f;

	int rectWidth = 0;
	int rectHeight = 0;
};

