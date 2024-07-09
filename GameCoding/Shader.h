#pragma once
#include "InputLayout.h"
#include "ResourceBase.h"
#include "ShaderSlot.h"
#include "Texture.h"

enum class ShaderType
{
	VERTEX_SHADER,
	PIXEL_SHADER,
	COMPUTE_SHADER
};
class Shader : public ResourceBase
{
	using Super = ResourceBase;
public:
	Shader();
	~Shader();

	void LoadShaderFromFile(const wstring& path, const string& name, const string& version, ComPtr<ID3DBlob>& blob);
	void CreateShader(ShaderType type, const wstring& shaderPath);
	void PushConstantBufferToShader(ShaderType type, const wstring& name, UINT numBuffers, shared_ptr<Buffer> buffer);
	void PushShaderResourceToShader(ShaderType type, const wstring& name, UINT numViews, shared_ptr<Texture> texture);
	void PushShaderResourceToShader(ShaderType type, const wstring& name, UINT numViews, ComPtr<ID3D11ShaderResourceView> shaderResourceViews);
	ComPtr<ID3D11VertexShader> GetVertexShader() { return _vertexShader; }
	ComPtr<ID3D11PixelShader> GetPixelShader() { return _pixelShader; }
	ComPtr<ID3D11ComputeShader> GetComputeShader() { return _computeShader; }
	ComPtr<ID3DBlob> GetVertexShaderBlob() { return _vsBlob; }
	ComPtr<ID3DBlob> GetPixelShaderBlob() { return _psBlob; }
	shared_ptr<InputLayout> GetInputLayout() { return _inputLayout; }
	shared_ptr<ShaderSlot> GetShaderSlot() { return _shaderSlot; }

private:
	ComPtr<ID3DBlob> _vsBlob;
	ComPtr<ID3DBlob> _psBlob;
	ComPtr<ID3DBlob> _csBlob;
	ComPtr<ID3D11VertexShader> _vertexShader;
	ComPtr<ID3D11PixelShader> _pixelShader;
	ComPtr<ID3D11ComputeShader> _computeShader;
	shared_ptr<InputLayout> _inputLayout;
	shared_ptr<ShaderSlot> _shaderSlot;
};

