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
// ���⿡ ��� ��ũ�� �߰�
REGISTER_MONOBEHAVIOR_METHOD(TestEvent, TestLog);
