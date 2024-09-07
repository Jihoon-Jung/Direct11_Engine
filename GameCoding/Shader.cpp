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
	ComPtr<ID3DBlob> errorBlob;

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
		&errorBlob);

	
	if (FAILED(hr)) {
		if (errorBlob) {
			// 에러 메시지를 받아오고, OutputDebugStringA를 사용해 디버그 출력
			std::string errorMessage(static_cast<char*>(errorBlob->GetBufferPointer()), errorBlob->GetBufferSize());
			OutputDebugStringA(errorMessage.c_str()); // 디버그 콘솔로 메시지 출력
		}
		CHECK(hr); // 에러 체크 (예: 프로그램 종료 또는 에러 처리)
	}
}

void Shader::CreateShader(ShaderType type, const wstring& shaderPath, InputLayoutType inputType)
{
	if (type == ShaderType::VERTEX_SHADER)
	{
		LoadShaderFromFile(shaderPath, "VS", "vs_5_0", _vsBlob);
		HRESULT hr = DEVICE->CreateVertexShader(_vsBlob->GetBufferPointer(), _vsBlob->GetBufferSize(), nullptr, _vertexShader.GetAddressOf());
		CHECK(hr);
		_inputLayout = make_shared<InputLayout>();
		if (inputType == InputLayoutType::VertexTextureNormalTangentBlendData)
		{
			_inputLayout->CreateInputLayout(VertexTextureNormalTangentBlendData::descs, _vsBlob);
		}
		else if (inputType == InputLayoutType::VertexTextureNormalBillboard)
		{
			_inputLayout->CreateInputLayout(VertexBillboard::descs, _vsBlob);
		}
		else if (inputType == InputLayoutType::VertexBillboard_Geometry)
		{
			_inputLayout->CreateInputLayout(VertexBillboard_GeometryShader::descs, _vsBlob);
		}
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
	else if (type == ShaderType::GEOMETRY_SHADER)
	{
		LoadShaderFromFile(shaderPath, "GS", "gs_5_0", _gsBlob);
		HRESULT hr = DEVICE->CreateGeometryShader(_gsBlob->GetBufferPointer(), _gsBlob->GetBufferSize(), nullptr, &_geometryShader);
		CHECK(hr);
	}
	else if (type == ShaderType::HULL_SHADER)
	{
		LoadShaderFromFile(shaderPath, "HS", "hs_5_0", _hsBlob);
		HRESULT hr = DEVICE->CreateHullShader(_hsBlob->GetBufferPointer(), _hsBlob->GetBufferSize(), nullptr, &_hullShader);
		CHECK(hr);
	}
	else if (type == ShaderType::DOMAIN_SHADER)
	{
		LoadShaderFromFile(shaderPath, "DS", "ds_5_0", _dsBlob);
		HRESULT hr = DEVICE->CreateDomainShader(_dsBlob->GetBufferPointer(), _dsBlob->GetBufferSize(), nullptr, &_domainShader);
		CHECK(hr);
	}
}

void Shader::PushConstantBufferToShader(ShaderType type, const wstring& name, UINT numBuffers, shared_ptr<Buffer> buffer)
{
	UINT slot = _shaderSlot->GetSlotNumber(name);
	if (type == ShaderType::VERTEX_SHADER)
		DEVICECONTEXT->VSSetConstantBuffers(slot, numBuffers, buffer->GetConstantBuffer().GetAddressOf());
	if (type == ShaderType::GEOMETRY_SHADER)
		DEVICECONTEXT->GSSetConstantBuffers(slot, numBuffers, buffer->GetConstantBuffer().GetAddressOf());
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