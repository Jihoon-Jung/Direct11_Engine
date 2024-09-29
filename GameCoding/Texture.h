#pragma once
#include "ResourceBase.h"

class Texture : public ResourceBase
{
	using Super = ResourceBase;
public:
	Texture();
	~Texture();

	ComPtr<ID3D11Texture2D> GetTexture2D();
	void SetShaderResourceView(ComPtr<ID3D11ShaderResourceView> shaderResourceView);
	void CreateTexture(const wstring& path);
	ComPtr<ID3D11ShaderResourceView> LoadTextureFromDDS(const wstring& path);
	ComPtr<ID3D11ShaderResourceView> CreateTexture2DArraySRV(std::vector<std::wstring>& filenames);
	ComPtr<ID3D11ShaderResourceView> CreateRandomTexture1DSRV();
	ComPtr<ID3D11ShaderResourceView>GetShaderResourceView() { return _shaderResourceView; }
	Vec2 GetSize() { return _size; }
private:
	ComPtr<ID3D11ShaderResourceView> _shaderResourceView;
	Vec2 _size{ 0.f, 0.f };

};

