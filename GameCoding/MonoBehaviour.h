#pragma once
#include "MethodRegisterMacro.h" // ��ũ�� ���
#include "ComponentFactory.h"

class MonoBehaviour : public Component
{
	using Super = Component;
public:
	MonoBehaviour();
	virtual ~MonoBehaviour();
};

