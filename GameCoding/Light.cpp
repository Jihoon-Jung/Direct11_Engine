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
	lightDesc.ambient = Vec4(0.3f, 0.3f, 0.3f, 1.0f);
	lightDesc.diffuse = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	lightDesc.specular = Vec4(0.8f, 0.8f, 0.7f, 1.0f);
	SetLightDesc(lightDesc);
}

void Light::SetLightDesc(LightDesc lightDesc)
{
	_lightDesc = lightDesc;
	_lightBuffer = make_shared<Buffer>();
	_lightBuffer->CreateConstantBuffer<LightDesc>();
	_lightBuffer->CopyData(lightDesc);
}
