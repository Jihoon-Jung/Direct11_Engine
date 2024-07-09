#pragma once
class TextureBuffer
{
public:
	TextureBuffer(ComPtr<ID3D11Texture2D> src);
	~TextureBuffer();

public:
	void CreateBuffer();

private:
	void CreateInput(ComPtr<ID3D11Texture2D> src);
	void CreateSRV();
	void CreateOutput();
	void CreateUAV();
	void CreateResult();

public:
	uint32 GetWidth() { return _width; }
	uint32 GetHeight() { return _height; }
	uint32 GetArraySize() { return _arraySize; }

	ComPtr<ID3D11Texture2D> GetOutput() { return (ID3D11Texture2D*)_output.Get(); }
	ComPtr<ID3D11ShaderResourceView> GetOutputSRV() { return _outputSRV; }

public:
	ComPtr<ID3D11ShaderResourceView> GetSRV() { return _srv; }
	ComPtr<ID3D11UnorderedAccessView> GetUAV() { return _uav; }

private:
	ComPtr<ID3D11Texture2D> _input;
	ComPtr<ID3D11ShaderResourceView> _srv; // Input
	ComPtr<ID3D11Texture2D> _output;
	ComPtr<ID3D11UnorderedAccessView> _uav; // Output

private:
	uint32 _width = 0;
	uint32 _height = 0;
	uint32 _arraySize = 0;
	DXGI_FORMAT _format;
	ComPtr<ID3D11ShaderResourceView> _outputSRV;
};

