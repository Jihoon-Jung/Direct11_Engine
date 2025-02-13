#pragma once
#include "Component.h"
#include "MonoBehaviour.h"


class MovePlayerWithCamera : public MonoBehaviour
{
    using Supder = MonoBehaviour;

public:

	MovePlayerWithCamera() {};
	~MovePlayerWithCamera() {};

	virtual void Start();
	virtual void Update();

    void ResetMouse();
    float Clamp(float value, float min, float max)
    {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }
    void SetAttackingFlagFalse()
    {
        _isAttacking = false;
        _animator->SetBool("isAttacking", false);
    }
private:
    bool _isFirstMove = true;
    POINT _prevMousePos;
    float _accumulatedRotX = 0.0f;
    float _accumulatedRotY = 0.0f;
    bool _isMouseControlEnabled = true;
    bool _isAttacking = false;

    shared_ptr<GameObject> _gameObject = nullptr;
    shared_ptr<Animator> _animator = nullptr;
};
REGISTER_SCRIPT(MovePlayerWithCamera, "MovePlayerWithCamera");
REGISTER_MONOBEHAVIOR_METHOD(MovePlayerWithCamera, SetAttackingFlagFalse);