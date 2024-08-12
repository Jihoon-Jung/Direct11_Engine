#include "pch.h"
#include "Material.h"

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


        /*uint32 x = max(1, (width + 31) / 32);
        uint32 y = max(1, (height + 31) / 32);
        uint32_t z = arraySize;

        DEVICECONTEXT->Dispatch(x, y, z);*/
    }
    


    //ComPtr<ID3D11Texture2D> converted_texture2;
    //ComPtr<ID3D11ShaderResourceView> horizontalResult = textureBuffer->GetOutputSRV();
    //horizontalResult->GetResource((ID3D11Resource**)converted_texture2.GetAddressOf());

    //shared_ptr<TextureBuffer> textureBuffer2 = make_shared<TextureBuffer>(converted_texture2);
    //
    //// Vertical
    //{
    //    DEVICECONTEXT->CSSetShader(verticalBlurShader->GetComputeShader().Get(), nullptr, 0);
    //    ComPtr<ID3D11ShaderResourceView> input = textureBuffer2->GetSRV();
    //    ComPtr<ID3D11UnorderedAccessView> output = textureBuffer2->GetUAV();

    //    // 텍스처 포맷 확인
    //    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    //    input->GetDesc(&srvDesc);
    //    srvDesc.Format;

    //    D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
    //    output->GetDesc(&uavDesc);
    //    uavDesc.Format;

    //    DEVICECONTEXT->CSSetShaderResources(0, 1, input.GetAddressOf()); // t0 slot
    //    DEVICECONTEXT->CSSetUnorderedAccessViews(0, 1, output.GetAddressOf(), nullptr); // u0 slot

    //    uint32 width = textureBuffer->GetWidth();
    //    uint32 height = textureBuffer->GetHeight();
    //    uint32 arraySize = textureBuffer->GetArraySize();

    //    uint32 x = max(1, (width + 31) / 32);
    //    uint32 y = max(1, (height + 31) / 32);
    //    uint32_t z = arraySize;

    //    DEVICECONTEXT->Dispatch(x, y, z);
    //}
    

    

    // Unbind resources
    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    DEVICECONTEXT->CSSetShaderResources(0, 1, nullSRV);
    ID3D11UnorderedAccessView* nullUAV[1] = { nullptr };
    DEVICECONTEXT->CSSetUnorderedAccessViews(0, 1, nullUAV, nullptr);
    DEVICECONTEXT->CSSetShader(nullptr, nullptr, 0);

    return textureBuffer->GetOutputSRV();
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


