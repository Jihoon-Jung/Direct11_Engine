#pragma once
#include "Component.h"
#include "MonoBehaviour.h"

class TestEvent : public MonoBehaviour
{
	using Supder = MonoBehaviour;
public:
	TestEvent() {};
	~TestEvent() {};

	virtual void Start() {};
	virtual void Update() {};

	void TestLog() {
		printf("Test Event");
	}
};
// 여기에 등록 매크로 추가
REGISTER_MONOBEHAVIOR_METHOD(TestEvent, TestLog);
