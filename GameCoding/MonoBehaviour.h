#pragma once
#include "MethodRegisterMacro.h" // 매크로 헤더

class MonoBehaviour : public Component
{
	using Super = Component;
public:
	MonoBehaviour();
	virtual ~MonoBehaviour();
};

