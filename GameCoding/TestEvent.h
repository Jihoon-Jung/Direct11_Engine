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
		char buffer[100];
		sprintf_s(buffer, "Event Test1.\n");
		OutputDebugStringA(buffer);
	}
	void TestLog2() {
		char buffer[100];
		sprintf_s(buffer, "Event Test2.\n");
		OutputDebugStringA(buffer);
	}
	void TestLog3() {
		char buffer[100];
		sprintf_s(buffer, "Event Test3.\n");
		OutputDebugStringA(buffer);
	}
};

// 등록 매크로 추가
REGISTER_MONOBEHAVIOR_METHOD(TestEvent, TestLog);
REGISTER_MONOBEHAVIOR_METHOD(TestEvent, TestLog2);
REGISTER_MONOBEHAVIOR_METHOD(TestEvent, TestLog3);
REGISTER_SCRIPT(TestEvent, "TestEvent");
