#include "pch.h"
#include "ScreenShot.h"
#include <fstream>
#include "MathHelper.h"

void ScreenShot::SaveRenderTargetToFile(ID3D11RenderTargetView* renderTargetView, const std::string& filename, int width, int height)
{
	// 1. Render Target���� ��� ���� �ؽ�ó�� ������
	ComPtr<ID3D11Resource> renderTargetResource;
	renderTargetView->GetResource(&renderTargetResource);

	ComPtr<ID3D11Texture2D> renderTargetTexture;
	renderTargetResource.As(&renderTargetTexture);

	// 2. Staging �ؽ�ó ���� (CPU���� �б� �����ϵ��� ����)
	D3D11_TEXTURE2D_DESC stagingDesc = {};
	renderTargetTexture->GetDesc(&stagingDesc);
	stagingDesc.Usage = D3D11_USAGE_STAGING;
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	stagingDesc.BindFlags = 0;
	stagingDesc.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> stagingTexture;
	HRESULT hr = DEVICE->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);
	CHECK(hr);

	// 3. Render Target �ؽ�ó�� Staging �ؽ�ó�� ����
	DEVICECONTEXT->CopyResource(stagingTexture.Get(), renderTargetTexture.Get());

	// 4. Staging �ؽ�ó���� �����͸� �����Ͽ� ������
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	hr = DEVICECONTEXT->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mappedResource);
	CHECK(hr);

	// 5. �ȼ� �����͸� ���Ϸ� ����
	std::ofstream outFile(filename, std::ios::binary);
	if (outFile.is_open())
	{
		// �� �ึ���� ����Ʈ �� (���� �ʺ� * 4)
		auto rowPitch = width * 4;

		// �� ���� �����͸� ���������� ���Ͽ� ���
		for (int y = 0; y < height; ++y)
		{
			outFile.write(static_cast<const char*>(mappedResource.pData) + y * mappedResource.RowPitch, rowPitch);
		}
		outFile.close();
	}

	// 6. ���� ����
	DEVICECONTEXT->Unmap(stagingTexture.Get(), 0);
}

void ScreenShot::SaveShaderResourceViewToFile(ID3D11ShaderResourceView* shaderResourceView, const std::string& filename, int width, int height)
{
    // 1. Shader Resource���� ��� ���� �ؽ�ó�� ������
    ComPtr<ID3D11Resource> resource;
    shaderResourceView->GetResource(&resource);

    ComPtr<ID3D11Texture2D> texture;
    resource.As(&texture);

    // 2. ���� �ؽ�ó�� �����ڸ� �����ͼ� Staging �ؽ�ó ���� (CPU���� �б� �����ϵ��� ����)
    D3D11_TEXTURE2D_DESC textureDesc = {};
    texture->GetDesc(&textureDesc);
    textureDesc.Usage = D3D11_USAGE_STAGING;
    textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    textureDesc.BindFlags = 0;
    textureDesc.MiscFlags = 0;

    ComPtr<ID3D11Texture2D> stagingTexture;
    HRESULT hr = DEVICE->CreateTexture2D(&textureDesc, nullptr, &stagingTexture);
    CHECK(hr);

    // 3. ���� �ؽ�ó�� Staging �ؽ�ó�� ����
    DEVICECONTEXT->CopyResource(stagingTexture.Get(), texture.Get());

    // 4. Staging �ؽ�ó���� �����͸� �����Ͽ� ������
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    hr = DEVICECONTEXT->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mappedResource);
    CHECK(hr);

    // 5. �ȼ� �����͸� ���Ϸ� ����
    std::ofstream outFile(filename, std::ios::binary);
    if (outFile.is_open())
    {
        // �� �ึ���� ����Ʈ �� (�ȼ� ���˿� ���� ���� �ʿ�)
        UINT bytesPerPixel = 4; // ��: RGBA8�� ���
        auto rowPitch = width * bytesPerPixel;

        // �� ���� �����͸� ���������� ���Ͽ� ���
        for (int y = 0; y < height; ++y)
        {
            outFile.write(
                static_cast<const char*>(mappedResource.pData) + y * mappedResource.RowPitch,
                rowPitch
            );
        }
        outFile.close();
    }

    // 6. ���� ����
    DEVICECONTEXT->Unmap(stagingTexture.Get(), 0);
}
