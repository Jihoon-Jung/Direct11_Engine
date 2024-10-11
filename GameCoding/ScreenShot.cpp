#include "pch.h"
#include "ScreenShot.h"
#include <fstream>
#include "MathHelper.h"

void ScreenShot::SaveRenderTargetToFile(ID3D11RenderTargetView* renderTargetView, const std::string& filename, int width, int height)
{
	// 1. Render Target에서 사용 중인 텍스처를 가져옴
	ComPtr<ID3D11Resource> renderTargetResource;
	renderTargetView->GetResource(&renderTargetResource);

	ComPtr<ID3D11Texture2D> renderTargetTexture;
	renderTargetResource.As(&renderTargetTexture);

	// 2. Staging 텍스처 생성 (CPU에서 읽기 가능하도록 설정)
	D3D11_TEXTURE2D_DESC stagingDesc = {};
	renderTargetTexture->GetDesc(&stagingDesc);
	stagingDesc.Usage = D3D11_USAGE_STAGING;
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	stagingDesc.BindFlags = 0;
	stagingDesc.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> stagingTexture;
	HRESULT hr = DEVICE->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);
	CHECK(hr);

	// 3. Render Target 텍스처를 Staging 텍스처로 복사
	DEVICECONTEXT->CopyResource(stagingTexture.Get(), renderTargetTexture.Get());

	// 4. Staging 텍스처에서 데이터를 맵핑하여 가져옴
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	hr = DEVICECONTEXT->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mappedResource);
	CHECK(hr);

	// 5. 픽셀 데이터를 파일로 저장
	std::ofstream outFile(filename, std::ios::binary);
	if (outFile.is_open())
	{
		// 한 행마다의 바이트 수 (행의 너비 * 4)
		auto rowPitch = width * 4;

		// 각 행의 데이터를 순차적으로 파일에 기록
		for (int y = 0; y < height; ++y)
		{
			outFile.write(static_cast<const char*>(mappedResource.pData) + y * mappedResource.RowPitch, rowPitch);
		}
		outFile.close();
	}

	// 6. 맵핑 해제
	DEVICECONTEXT->Unmap(stagingTexture.Get(), 0);
}

void ScreenShot::SaveShaderResourceViewToFile(ID3D11ShaderResourceView* shaderResourceView, const std::string& filename, int width, int height)
{
    // 1. Shader Resource에서 사용 중인 텍스처를 가져옴
    ComPtr<ID3D11Resource> resource;
    shaderResourceView->GetResource(&resource);

    ComPtr<ID3D11Texture2D> texture;
    resource.As(&texture);

    // 2. 원본 텍스처의 설명자를 가져와서 Staging 텍스처 생성 (CPU에서 읽기 가능하도록 설정)
    D3D11_TEXTURE2D_DESC textureDesc = {};
    texture->GetDesc(&textureDesc);
    textureDesc.Usage = D3D11_USAGE_STAGING;
    textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    textureDesc.BindFlags = 0;
    textureDesc.MiscFlags = 0;

    ComPtr<ID3D11Texture2D> stagingTexture;
    HRESULT hr = DEVICE->CreateTexture2D(&textureDesc, nullptr, &stagingTexture);
    CHECK(hr);

    // 3. 원본 텍스처를 Staging 텍스처로 복사
    DEVICECONTEXT->CopyResource(stagingTexture.Get(), texture.Get());

    // 4. Staging 텍스처에서 데이터를 맵핑하여 가져옴
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    hr = DEVICECONTEXT->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mappedResource);
    CHECK(hr);

    // 5. 픽셀 데이터를 파일로 저장
    std::ofstream outFile(filename, std::ios::binary);
    if (outFile.is_open())
    {
        // 한 행마다의 바이트 수 (픽셀 포맷에 따라 변경 필요)
        UINT bytesPerPixel = 4; // 예: RGBA8의 경우
        auto rowPitch = width * bytesPerPixel;

        // 각 행의 데이터를 순차적으로 파일에 기록
        for (int y = 0; y < height; ++y)
        {
            outFile.write(
                static_cast<const char*>(mappedResource.pData) + y * mappedResource.RowPitch,
                rowPitch
            );
        }
        outFile.close();
    }

    // 6. 맵핑 해제
    DEVICECONTEXT->Unmap(stagingTexture.Get(), 0);
}
