#pragma once
#include "Component.h"
#include "MonoBehaviour.h"

class TestEvent : public MonoBehaviour
{
	using Supder = MonoBehaviour;
public:
	TestEvent() {};
	~TestEvent() {};

	virtual void Start();
	virtual void Update();

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
	void MoveRight() {
		shared_ptr<Transform> transform = SCENE.GetActiveScene()->Find(L"cube")->transform();
		Vec3 position = transform->GetLocalPosition();
		transform->SetLocalPosition(Vec3(position.x + 5.0f, position.y, position.z));
	}
	void MoveLeft() {
		shared_ptr<Transform> transform = SCENE.GetActiveScene()->Find(L"cube")->transform();
		Vec3 position = transform->GetLocalPosition();
		transform->SetLocalPosition(Vec3(position.x - 5.0f, position.y, position.z));
	}
	void MoveForward() {
		shared_ptr<Transform> transform = SCENE.GetActiveScene()->Find(L"cube")->transform();
		Vec3 position = transform->GetLocalPosition();
		transform->SetLocalPosition(Vec3(position.x, position.y, position.z + 5.0f));
	}
	void MoveBackward() {
		shared_ptr<Transform> transform = SCENE.GetActiveScene()->Find(L"cube")->transform();
		Vec3 position = transform->GetLocalPosition();
		transform->SetLocalPosition(Vec3(position.x, position.y, position.z - 5.0f));
	}

private:
	shared_ptr<GameObject> _gameObject = nullptr;
	int a = 10;
};


// 등록 매크로 추가
REGISTER_SCRIPT(TestEvent, "TestEvent");
REGISTER_MONOBEHAVIOR_METHOD(TestEvent, TestLog);
REGISTER_MONOBEHAVIOR_METHOD(TestEvent, TestLog2);
REGISTER_MONOBEHAVIOR_METHOD(TestEvent, TestLog3);
REGISTER_MONOBEHAVIOR_METHOD(TestEvent, MoveRight);
REGISTER_MONOBEHAVIOR_METHOD(TestEvent, MoveLeft);
REGISTER_MONOBEHAVIOR_METHOD(TestEvent, MoveForward);
REGISTER_MONOBEHAVIOR_METHOD(TestEvent, MoveBackward);

