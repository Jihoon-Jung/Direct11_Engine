#include "pch.h"
#include "Shader.h"

Shader::Shader()
	:Super(ResourceType::Shader)
{
	_shaderSlot = make_shared<ShaderSlot>();
}

Shader::~Shader()
{
}

void Shader::LoadShaderFromFile(const wstring& path, const string& name, const string& version, ComPtr<ID3DBlob>& blob)
{
	const uint32 compileFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

	HRESULT hr = ::D3DCompileFromFile(
		path.c_str(),
		nullptr,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		name.c_str(),
		version.c_str(),
		compileFlag,
		0,
		blob.GetAddressOf(),
		nullptr);

	CHECK(hr);
}

void Shader::CreateShader(ShaderType type, const wstring& shaderPath)
{
	if (type == ShaderType::VERTEX_SHADER)
	{
		LoadShaderFromFile(shaderPath, "VS", "vs_5_0", _vsBlob);
		HRESULT hr = DEVICE->CreateVertexShader(_vsBlob->GetBufferPointer(), _vsBlob->GetBufferSize(), nullptr, _vertexShader.GetAddressOf());
		CHECK(hr);
		_inputLayout = make_shared<InputLayout>();
		_inputLayout->CreateInputLayout(VertexTextureNormalTangentBlendData::descs, _vsBlob);
	}
	else if (type == ShaderType::PIXEL_SHADER)
	{
		LoadShaderFromFile(shaderPath, "PS", "ps_5_0", _psBlob);
		HRESULT hr = DEVICE->CreatePixelShader(_psBlob->GetBufferPointer(), _psBlob->GetBufferSize(), nullptr, _pixelShader.GetAddressOf());
		CHECK(hr);
	}
	else if (type == ShaderType::COMPUTE_SHADER)
	{
		LoadShaderFromFile(shaderPath, "CS", "cs_5_0", _csBlob);
		HRESULT hr = DEVICE->CreateComputeShader(_csBlob->GetBufferPointer(), _csBlob->GetBufferSize(), nullptr, &_computeShader);
		CHECK(hr);
	}
}

void Shader::PushConstantBufferToShader(ShaderType type, const wstring& name, UINT numBuffers, shared_ptr<Buffer> buffer)
{
	UINT slot = _shaderSlot->GetSlotNumber(name);
	if (type == ShaderType::VERTEX_SHADER)
		DEVICECONTEXT->VSSetConstantBuffers(slot, numBuffers, buffer->GetConstantBuffer().GetAddressOf());
	else
		DEVICECONTEXT->PSSetConstantBuffers(slot, numBuffers, buffer->GetConstantBuffer().GetAddressOf());
}

void Shader::PushShaderResourceToShader(ShaderType type, const wstring& name, UINT numViews, ComPtr<ID3D11ShaderResourceView> shaderResourceViews)
{
	UINT slot = _shaderSlot->GetSlotNumber(name);
	if (type == ShaderType::VERTEX_SHADER)
		DEVICECONTEXT->VSSetShaderResources(slot, numViews, shaderResourceViews.GetAddressOf());
	else
		DEVICECONTEXT->PSSetShaderResources(slot, numViews, shaderResourceViews.GetAddressOf());
}
void Shader::PushShaderResourceToShader(ShaderType type, const wstring& name, UINT numViews, shared_ptr<Texture> texture)
{
	UINT slot = _shaderSlot->GetSlotNumber(name);
	if (type == ShaderType::VERTEX_SHADER)
		DEVICECONTEXT->VSSetShaderResources(slot, numViews, texture->GetShaderResourceView().GetAddressOf());
	else
		DEVICECONTEXT->PSSetShaderResources(slot, numViews, texture->GetShaderResourceView().GetAddressOf());
}