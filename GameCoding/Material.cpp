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


