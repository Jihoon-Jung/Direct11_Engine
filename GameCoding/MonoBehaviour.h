#pragma once
#include "MethodRegisterMacro.h" // ��ũ�� ���

class MonoBehaviour : public Component
{
	using Super = Component;
public:
	MonoBehaviour();
	virtual ~MonoBehaviour();
};

