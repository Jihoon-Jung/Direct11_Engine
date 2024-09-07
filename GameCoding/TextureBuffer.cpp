#include "pch.h"
#include "TextureBuffer.h"

TextureBuffer::TextureBuffer(ComPtr<ID3D11Texture2D> src)
{
	CreateInput(src);
	CreateBuffer();
}

TextureBuffer::~TextureBuffer()
{
}

void TextureBuffer::CreateBuffer()
{
	CreateSRV();
	CreateOutput();
	CreateUAV();
	CreateResult();
}

void TextureBuffer::CreateInput(ComPtr<ID3D11Texture2D> src)
{
	// copy input texture for shader resource view

	D3D11_TEXTURE2D_DESC srcDesc;
	src->GetDesc(&srcDesc);

	_width = srcDesc.Width;
	_height = srcDesc.Height;
	_arraySize = srcDesc.ArraySize;
	_format = srcDesc.Format;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Width = _width;
	desc.Height = _height;
	desc.ArraySize = _arraySize;
	desc.Format = _format;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;

	CHECK(DEVICE->CreateTexture2D(&desc, NULL, _input.GetAddressOf()));

	DEVICECONTEXT->CopyResource(_input.Get(), src.Get()); // 텍스처 복사? 그런느낌인듯?
}

void TextureBuffer::CreateSRV()
{
	// make input texture shader resource view

	D3D11_TEXTURE2D_DESC desc;
	_input->GetDesc(&desc);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.MipLevels = 1;
	srvDesc.Texture2DArray.ArraySize = _arraySize;

	CHECK(DEVICE->CreateShaderResourceView(_input.Get(), &srvDesc, _srv.GetAddressOf()));

}

void TextureBuffer::CreateOutput()
{
	// create output texture

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Width = _width;
	desc.Height = _height;
	desc.ArraySize = _arraySize;
	desc.Format = _format;
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;

	CHECK(DEVICE->CreateTexture2D(&desc, nullptr, _output.GetAddressOf()));
}

void TextureBuffer::CreateUAV()
{
	// create UAV using output texture
	D3D11_TEXTURE2D_DESC desc;
	_output->GetDesc(&desc);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
	uavDesc.Format = desc.Format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2DARRAY;
	uavDesc.Texture2DArray.ArraySize = _arraySize;

	CHECK(DEVICE->CreateUnorderedAccessView(_output.Get(), &uavDesc, _uav.GetAddressOf()));
}

void TextureBuffer::CreateResult()
{
	D3D11_TEXTURE2D_DESC desc;
	_output->GetDesc(&desc);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvDesc.Texture2DArray.MipLevels = 1;
	srvDesc.Texture2DArray.ArraySize = _arraySize;

	CHECK(DEVICE->CreateShaderResourceView(_output.Get(), &srvDesc, _outputSRV.GetAddressOf()));
}
