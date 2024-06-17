#include "pch.h"
#include "Light.h"

Light::Light()
	:Super(ComponentType::Light)
{
}

Light::~Light()
{
}

void Light::Update()
{
	LightDesc lightDesc;
	lightDesc.ambient = Vec4(1.f);
	lightDesc.diffuse = Vec4(1.f);
	lightDesc.specular = Vec4(1.f);
	SetLightDesc(lightDesc);
}

void Light::SetLightDesc(LightDesc lightDesc)
{
	_lightDesc = lightDesc;
	_lightBuffer = make_shared<Buffer>();
	_lightBuffer->CreateConstantBuffer<LightDesc>();
	_lightBuffer->CopyData(lightDesc);
}
