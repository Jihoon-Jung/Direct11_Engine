#include "pch.h"
#include "Material.h"

Material::Material()
	:Super(ResourceType::Material)
{
}

Material::~Material()
{
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


