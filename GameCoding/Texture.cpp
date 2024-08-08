#include "pch.h"
#include "Texture.h"

Texture::Texture()
	: Super(ResourceType::Texture)
{
}

Texture::~Texture()
{
}

void Texture::CreateTexture(const wstring& path)
{
	DirectX::TexMetadata md;
	DirectX::ScratchImage img;
	HRESULT hr = ::LoadFromWICFile(path.c_str(), WIC_FLAGS_NONE, &md, img);
	CHECK(hr);

	hr = ::CreateShaderResourceView(DEVICE.Get(), img.GetImages(), img.GetImageCount(), md, _shaderResourceView.GetAddressOf());
	CHECK(hr);

	// 텍스처 포맷 확인
	DXGI_FORMAT format = md.format;

	_size.x = md.width;
	_size.y = md.height;

	
}
void Texture::SetShaderResourceView(ComPtr<ID3D11ShaderResourceView> shaderResourceView)
{
	_shaderResourceView = shaderResourceView;
}
Microsoft::WRL::ComPtr<ID3D11Texture2D> Texture::GetTexture2D()
{
	ComPtr<ID3D11Texture2D> texture;
	_shaderResourceView->GetResource((ID3D11Resource**)texture.GetAddressOf());
	return texture;
}
