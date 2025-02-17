#pragma once
#include "Component.h"

class ParticleSystem : public Component
{
	using Super = Component;

public:
	ParticleSystem();
	~ParticleSystem();

	void ReadStreamOutData();
	void Update();
	void SetSpeed(float speed) { _speed = speed; }
	void SetFirstRunFlag(bool flag) { _firstRun = flag; }
	void SetEndParticleFlag(bool flag) { _endParticle = flag; }
	bool GetFirstRunFlag() { return _firstRun; }
	bool GetEndParticleFlag() { return _endParticle; }
	ComPtr<ID3D11Buffer> GetinitVB() { return _initVB; }
	ComPtr<ID3D11Buffer>& GetDrawVB() { return _drawVB; }
	ComPtr<ID3D11Buffer>& GetStreamOutVB() { return _streamOutVB; }
	float GetSpeed() { return _speed; }
	
private:
	void BuildVB();

private:
	uint32 _maxParticles = 0;
	bool _firstRun;
	bool _endParticle = false;
	float _speed = 1.0f;

	ComPtr<ID3D11Buffer> _initVB;
	ComPtr<ID3D11Buffer> _drawVB;
	ComPtr<ID3D11Buffer> _streamOutVB;

	ComPtr<ID3D11ShaderResourceView> _texArraySRV;
	ComPtr<ID3D11ShaderResourceView> _randomTexSRV;
};

