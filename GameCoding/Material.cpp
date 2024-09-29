#include "pch.h"
#include "Material.h"
#include <sstream>

Material::Material()
	:Super(ResourceType::Material)
{
}

Material::~Material()
{
}


ComPtr<ID3D11ShaderResourceView> Material::AdjustTexture(shared_ptr<Shader> computeShader, shared_ptr<Texture> texture)
{
    shared_ptr<TextureBuffer> textureBuffer = make_shared<TextureBuffer>(texture->GetTexture2D());
    DEVICECONTEXT->CSSetShader(computeShader->GetComputeShader().Get(), nullptr, 0);

    ComPtr<ID3D11ShaderResourceView> input = textureBuffer->GetSRV();
    ComPtr<ID3D11UnorderedAccessView> output = textureBuffer->GetUAV();

    // 텍스처 포맷 확인
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    input->GetDesc(&srvDesc);
    srvDesc.Format;

    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    output->GetDesc(&uavDesc);
    uavDesc.Format;

    DEVICECONTEXT->CSSetShaderResources(0, 1, input.GetAddressOf()); // t0 slot
    DEVICECONTEXT->CSSetUnorderedAccessViews(0, 1, output.GetAddressOf(), nullptr); // u0 slot

    uint32 width = textureBuffer->GetWidth();
    uint32 height = textureBuffer->GetHeight();
    uint32 arraySize = textureBuffer->GetArraySize();

    uint32 x = max(1, (width + 31) / 32);
    uint32 y = max(1, (height + 31) / 32);
    uint32_t z = arraySize;

    DEVICECONTEXT->Dispatch(x, y, z);

    // Unbind resources
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    DEVICECONTEXT->CSSetShaderResources(0, 1, nullSRV);
    ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
    DEVICECONTEXT->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
    DEVICECONTEXT->CSSetShader(nullptr, nullptr, 0);

    return textureBuffer->GetOutputSRV();
}

ComPtr<ID3D11ShaderResourceView> Material::GaussainBlur(shared_ptr<Shader> verticalBlurShader, shared_ptr<Shader> horizontalBlurShader, ComPtr<ID3D11ShaderResourceView> texture)
{
    ComPtr<ID3D11Texture2D> converted_texture;
    texture->GetResource((ID3D11Resource**)converted_texture.GetAddressOf());

    shared_ptr<TextureBuffer> textureBuffer = make_shared<TextureBuffer>(converted_texture);

    // Horizontal
    {
        DEVICECONTEXT->CSSetShader(horizontalBlurShader->GetComputeShader().Get(), nullptr, 0);
        ComPtr<ID3D11ShaderResourceView> input = textureBuffer->GetSRV();
        ComPtr<ID3D11UnorderedAccessView> output = textureBuffer->GetUAV();

        // 텍스처 포맷 확인
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        input->GetDesc(&srvDesc);
        srvDesc.Format;

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        output->GetDesc(&uavDesc);
        uavDesc.Format;

        DEVICECONTEXT->CSSetShaderResources(0, 1, input.GetAddressOf()); // t0 slot
        DEVICECONTEXT->CSSetUnorderedAccessViews(0, 1, output.GetAddressOf(), nullptr); // u0 slot

        uint32 width = textureBuffer->GetWidth();
        uint32 height = textureBuffer->GetHeight();
        uint32 arraySize = textureBuffer->GetArraySize();


        uint32 numGroupsX = (uint32)ceilf(width / 256.0f);
        DEVICECONTEXT->Dispatch(numGroupsX, height, 1);


    }
    


    ComPtr<ID3D11Texture2D> converted_texture2;
    ComPtr<ID3D11ShaderResourceView> horizontalResult = textureBuffer->GetOutputSRV();
    horizontalResult->GetResource((ID3D11Resource**)converted_texture2.GetAddressOf());

    shared_ptr<TextureBuffer> textureBuffer2 = make_shared<TextureBuffer>(converted_texture2);
    
    // Vertical
    {
        DEVICECONTEXT->CSSetShader(verticalBlurShader->GetComputeShader().Get(), nullptr, 0);
        ComPtr<ID3D11ShaderResourceView> input = textureBuffer2->GetSRV();
        ComPtr<ID3D11UnorderedAccessView> output = textureBuffer2->GetUAV();

        // 텍스처 포맷 확인
        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        input->GetDesc(&srvDesc);
        srvDesc.Format;

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
        output->GetDesc(&uavDesc);
        uavDesc.Format;

        DEVICECONTEXT->CSSetShaderResources(0, 1, input.GetAddressOf()); // t0 slot
        DEVICECONTEXT->CSSetUnorderedAccessViews(0, 1, output.GetAddressOf(), nullptr); // u0 slot

        uint32 width = textureBuffer->GetWidth();
        uint32 height = textureBuffer->GetHeight();
        uint32 arraySize = textureBuffer->GetArraySize();



        uint32 numGroupsY = (uint32)ceilf(height / 256.0f);
        DEVICECONTEXT->Dispatch(width, numGroupsY, 1);

    }
    

    

    // Unbind resources
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    DEVICECONTEXT->CSSetShaderResources(0, 1, nullSRV);
    ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
    DEVICECONTEXT->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
    DEVICECONTEXT->CSSetShader(nullptr, nullptr, 0);

    return textureBuffer2->GetOutputSRV();
}

void Material::PushMaterialDesc()
{
	_materialBuffer = make_shared<Buffer>();
	_materialBuffer->CreateConstantBuffer<MaterialDesc>();
	_materialBuffer->CopyData(_materialDesc);
}

void Material::SetMaterialDesc(MaterialDesc materialDesc)
{
	_materialDesc.ambient = materialDesc.ambient;
	_materialDesc.diffuse = materialDesc.diffuse;
	_materialDesc.emissive = materialDesc.emissive;
	_materialDesc.specular = materialDesc.specular;
}

void Material::CreateCubeMapTexture(shared_ptr<Texture> textureArray[6])
{
    ComPtr<ID3D11Texture2D> faceTextures[6];
    for (int i = 0; i < 6; i++)
    {
        faceTextures[i] = textureArray[i]->GetTexture2D();
    }
    // 큐브 맵 텍스처 설명자 정의
    D3D11_TEXTURE2D_DESC texDesc = {};
    faceTextures[0]->GetDesc(&texDesc);
    texDesc.ArraySize = 6; // 6개의 면
    texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

    ComPtr<ID3D11Texture2D> cubeTexture;
    HRESULT hr = DEVICE->CreateTexture2D(&texDesc, nullptr, &cubeTexture);
    CHECK(hr);

    // 각 면에 텍스처 데이터 복사
    for (int i = 0; i < 6; ++i)
    {
        for (UINT mip = 0; mip < texDesc.MipLevels; ++mip)
        {
            UINT subresourceIndex = D3D11CalcSubresource(mip, i, texDesc.MipLevels);
            DEVICECONTEXT->CopySubresourceRegion(cubeTexture.Get(), subresourceIndex, 0, 0, 0, faceTextures[i].Get(), mip, nullptr);
        }
    }

    // Shader Resource View 생성
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = texDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.TextureCube.MipLevels = texDesc.MipLevels;

    hr = DEVICE->CreateShaderResourceView(cubeTexture.Get(), &srvDesc, _cubeMapSRV.GetAddressOf());
    CHECK(hr);
}

void Material::CreateEnvironmentMapTexture(shared_ptr<GameObject> gameObject)
{
    Vec3 origin_rotation = gameObject->transform()->GetLocalRotation();
    Vec3 camera_origin_position = SCENE.GetActiveScene()->Find(L"MainCamera")->transform()->GetLocalPosition();
    shared_ptr<Camera> camera = SCENE.GetActiveScene()->Find(L"MainCamera")->GetComponent<Camera>();
    
    Vec3 worldPosition = gameObject->transform()->GetWorldPosition();
    SCENE.GetActiveScene()->Find(L"MainCamera")->transform()->SetLocalPosition(worldPosition);

    Vec3 lookVector;
    Vec3 upVector;

    shared_ptr<RenderPass> renderPass[6];
    for (auto& rp : renderPass) {
        rp = std::make_shared<RenderPass>();
    }


    for (int i = 0; i < 6; i++)
    {
        Vec3 rotation;
        switch (i)
        {
        case 0: // Positive X (+X): 오른쪽
            rotation = origin_rotation + Vec3(0.0f, 90.0f, 0.0f);
            lookVector = Vec3(1.0f, 0.0f, 0.0f);
            upVector = Vec3(0.0f, 1.0f, 0.0f);
            break;
        case 1: // Negative X (-X): 왼쪽
            rotation = origin_rotation + Vec3(0.0f, -90.0f, 0.0f);
            lookVector = Vec3(-1.0f, 0.0f, 0.0f);
            upVector = Vec3(0.0f, 1.0f, 0.0f);
            break;
        case 2: // Positive Y (+Y): 위쪽
            rotation = origin_rotation + Vec3(-90.0f, 0.0f, 0.0f);
            lookVector = Vec3(0.0f, 1.0f, 0.0f);
            upVector = Vec3(0.0f, 0.0f, -1.0f);
            break;
        case 3: // Negative Y (-Y): 아래쪽
            rotation = origin_rotation + Vec3(90.0f, 0.0f, 0.0f);
            lookVector = Vec3(0.0f, -1.0f, 0.0f);
            upVector = Vec3(0.0f, 0.0f, 1.0f);
            break;
        case 4: // Positive Z (+Z): 앞쪽
            rotation = origin_rotation;
            lookVector = Vec3(0.0f, 0.0f, 1.0f);
            upVector = Vec3(0.0f, 1.0f, 0.0f);
            break;
        case 5: // Negative Z (-Z): 뒤쪽
            rotation = origin_rotation + Vec3(0.0f, 180.0f, 0.0f);
            lookVector = Vec3(0.0f, 0.0f, -1.0f);
            upVector = Vec3(0.0f, 1.0f, 0.0f);
            break;
        }

        gameObject->transform()->SetRotation(rotation);

        //lookVector = gameObject->transform()->GetLook();
        //upVector = gameObject->transform()->GetUp();
        camera->SetEnvironmentMapViewProjectionMatrix(worldPosition, lookVector, upVector);

        gameObject->transform()->UpdateTransform();

        int envmapWidth = Graphics::GetInstance().GetEnvironmentMapWidth();
        int envmapHeight = Graphics::GetInstance().GetEnvironmentMapHeight();
        renderPass[i]->CreateAndSetOffscreenRenderTarget(envmapWidth, envmapHeight);
        RENDER.RenderRenderableObject(true);

        /*std::string filename = "environmentMap" + std::to_string(i + 1) + "_" + std::to_string(envmapWidth) + "x" + std::to_string(envmapHeight) + ".dump";
        renderPass[i]->SaveRenderTargetToFile(renderPass[i]->GetOffscreenRTV().Get(), filename, envmapWidth, envmapHeight);*/
    }

    GP.RestoreRenderTarget();
    SCENE.GetActiveScene()->Find(L"MainCamera")->transform()->SetLocalPosition(camera_origin_position);

    shared_ptr<Texture> textureArray[6];
    for (auto& ta : textureArray) {
        ta = std::make_shared<Texture>();
    }
    for (int i = 0; i < 6; i++)
    {
        textureArray[i]->SetShaderResourceView(renderPass[i]->GetOffscreenSRV());
    }
    CreateCubeMapTexture(textureArray);

    gameObject->transform()->SetLocalRotation(origin_rotation);
}




