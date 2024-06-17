#pragma once
#include "ConstantBufferType.h"
#include "Buffer.h"

class Light : public Component
{
	using Super = Component;

public:
	Light();
	virtual ~Light();
	virtual void Update() override;
	void SetLightDesc(LightDesc lightDesc);
	LightDesc GetLightDesc() { return _lightDesc; }
	shared_ptr<Buffer> GetLightBuffer() { return _lightBuffer; }
private:
	LightDesc _lightDesc;
	shared_ptr<Buffer> _lightBuffer;
};

