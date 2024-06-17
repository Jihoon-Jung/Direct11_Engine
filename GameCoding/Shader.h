#pragma once
#include "InputLayout.h"
#include "ResourceBase.h"
enum class ShaderType
{
	VERTEX_SHADER,
	PIXEL_SHADER
};
class Shader : public ResourceBase
{
	using Super = ResourceBase;
public:
	Shader();
	~Shader();

	void LoadShaderFromFile(const wstring& path, const string& name, const string& version, ComPtr<ID3DBlob>& blob);
	void CreateShader(ShaderType type, const wstring& shaderPath);
	void SetInputLayout(shared_ptr<InputLayout> inputLayout) { _inputLayout = inputLayout; }

	ComPtr<ID3D11VertexShader> GetVertexShader() { return _vertexShader; }
	ComPtr<ID3D11PixelShader> GetPixelShader() { return _pixelShader; }
	ComPtr<ID3DBlob> GetVertexShaderBlob() { return _vsBlob; }
	ComPtr<ID3DBlob> GetPixelShaderBlob() { return _psBlob; }
	shared_ptr<InputLayout> GetInputLayout() { return _inputLayout; }
private:
	ComPtr<ID3DBlob> _vsBlob;
	ComPtr<ID3DBlob> _psBlob;
	ComPtr<ID3D11VertexShader> _vertexShader;
	ComPtr<ID3D11PixelShader> _pixelShader;
	shared_ptr<InputLayout> _inputLayout;
};

