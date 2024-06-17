#include "pch.h"
#include "Shader.h"

Shader::Shader()
	:Super(ResourceType::Shader)
{
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

	}
}
