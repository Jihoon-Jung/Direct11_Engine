#pragma once
#include "Component.h"
#include "MonoBehaviour.h"

class MoveObject : public MonoBehaviour
{
	using Supder = MonoBehaviour;
public:
	MoveObject();
	~MoveObject();

	virtual void Start();
	virtual void Update();
	void ResetMouse();
	template <typename T>
	constexpr const T& clamp(const T& v, const T& lo, const T& hi) {
		return (v < lo) ? lo : (hi < v) ? hi : v;
	}
private:
	float _speed = 10.f;
	POINT _prevMousePos;
	bool _isResetMouse = false;
};
