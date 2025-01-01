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

	float Clamp(float value, float min, float max)
	{
		if (value < min) return min;
		if (value > max) return max;
		return value;
	}

	void InitializeRotationFromTransform();
	float GetSpeed() const { return _speed; }
private:

	bool _isFirstMove = true;
	float _speed = 10.f;
	POINT _prevMousePos;
	bool _isResetMouse = false;
	float _accumulatedRotX = 0.0f;  // 누적된 X축 회전 각도
	float _accumulatedRotY = 0.0f;  // 누적된 Y축 회전 각도
};

