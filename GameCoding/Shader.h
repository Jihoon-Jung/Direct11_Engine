#pragma once
#include "InputLayout.h"
#include "ResourceBase.h"
#include "ShaderSlot.h"
#include "Texture.h"
#include "ConstantBufferType.h"



enum class ShaderType
{
	VERTEX_SHADER,
	PIXEL_SHADER,
	COMPUTE_SHADER,
	GEOMETRY_SHADER,
	GEOMETRY_SHADER_WITH_STREAMOUTPUT,
	HULL_SHADER,
	DOMAIN_SHADER
};

class Shader : public ResourceBase
{
	using Super = ResourceBase;
public:
	Shader();
	~Shader();

	void LoadShaderFromFile(const wstring& path, const string& name, const string& version, ComPtr<ID3DBlob>& blob);
	void CreateShader(ShaderType type, const wstring& shaderPath, InputLayoutType inputType);
	void PushConstantBufferToShader(ShaderType type, const wstring& name, UINT numBuffers, shared_ptr<Buffer> buffer);
	void PushShaderResourceToShader(ShaderType type, const wstring& name, UINT numViews, ComPtr<ID3D11ShaderResourceView> shaderResourceViews);

	void ResetShaderResources();

	ComPtr<ID3D11VertexShader> GetVertexShader() { return _vertexShader; }
	ComPtr<ID3D11PixelShader> GetPixelShader() { return _pixelShader; }
	ComPtr<ID3D11ComputeShader> GetComputeShader() { return _computeShader; }
	ComPtr<ID3D11GeometryShader> GetGeometryShader() { return _geometryShader; }
	ComPtr<ID3D11GeometryShader> GetOutputStreamGeometryShader() { return _streamOutGS; }
	ComPtr<ID3D11HullShader> GetHullShader() { return _hullShader; }
	ComPtr<ID3D11DomainShader> GetDomainShader() { return _domainShader; }

	ComPtr<ID3DBlob> GetVertexShaderBlob() { return _vsBlob; }
	ComPtr<ID3DBlob> GetPixelShaderBlob() { return _psBlob; }
	shared_ptr<InputLayout> GetInputLayout() { return _inputLayout; }
	shared_ptr<ShaderSlot> GetShaderSlot() { return _shaderSlot; }
	

private:
	ComPtr<ID3DBlob> _vsBlob;
	ComPtr<ID3DBlob> _psBlob;
	ComPtr<ID3DBlob> _csBlob;
	ComPtr<ID3DBlob> _gsBlob;
	ComPtr<ID3DBlob> _hsBlob;
	ComPtr<ID3DBlob> _dsBlob;
	ComPtr<ID3D11VertexShader> _vertexShader;
	ComPtr<ID3D11PixelShader> _pixelShader;
	ComPtr<ID3D11ComputeShader> _computeShader;
	ComPtr<ID3D11GeometryShader> _geometryShader;
	ComPtr<ID3D11GeometryShader> _streamOutGS;
	ComPtr<ID3D11HullShader> _hullShader;
	ComPtr<ID3D11DomainShader> _domainShader;
	shared_ptr<InputLayout> _inputLayout;
	shared_ptr<ShaderSlot> _shaderSlot;

	void ResetVertexShaderResources();
	void ResetPixelShaderResources();
	void ResetGeometryShaderResources();
	void ResetDomainShaderResources();
	void ResetHullShaderResources();

};

