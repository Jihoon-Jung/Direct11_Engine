#pragma once
#include "Component.h"

class ParticleSystem : public Component
{
	using Super = Component;

public:
	ParticleSystem();
	~ParticleSystem();

	void Init(ComPtr<ID3D11ShaderResourceView> texArraySRV,
		ComPtr<ID3D11ShaderResourceView> randomTexSRV,
		uint32 maxParticles);

	void ReadStreamOutData();
	void Reset();
	void Update(float dt, float gameTime, bool isEnv);
	void Draw();

private:
	void BuildVB();

private:
	uint32 _maxParticles = 0;
	bool _firstRun;
	bool _isEnv;

	float _gameTime;
	float _timeStep;
	float _age;

	Vec3 _eyePosW;
	Vec3 _emitPosW;
	Vec3 _emitDirW;

	ComPtr<ID3D11Buffer> _initVB;
	ComPtr<ID3D11Buffer> _drawVB;
	ComPtr<ID3D11Buffer> _streamOutVB;

	ComPtr<ID3D11ShaderResourceView> _texArraySRV;
	ComPtr<ID3D11ShaderResourceView> _randomTexSRV;
};

