#pragma once
#include "Component.h"

class ParticleSystem : public Component
{
	using Super = Component;

public:
	ParticleSystem();
	~ParticleSystem();


	void SetTransform(shared_ptr<Transform> transform) { _transform = transform; }
	void ReadStreamOutData();
	void Reset();
	void Update();

	void SetFirstRunFlag(bool flag) { _firstRun = flag; }
	bool GetFirstRunFlag() { return _firstRun; }
	ComPtr<ID3D11Buffer> GetinitVB() { return _initVB; }
	ComPtr<ID3D11Buffer>& GetDrawVB() { return _drawVB; }
	ComPtr<ID3D11Buffer>& GetStreamOutVB() { return _streamOutVB; }
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

	shared_ptr<Transform> _transform;

	ComPtr<ID3D11ShaderResourceView> _texArraySRV;
	ComPtr<ID3D11ShaderResourceView> _randomTexSRV;
};

