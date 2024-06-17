#include "pch.h"
#include "MoveObject.h"

MoveObject::MoveObject()
{
}

MoveObject::~MoveObject()
{
}

void MoveObject::Start()
{

}

void MoveObject::Update()
{
	//if (_gameObject->GetName() == L"MainCamera")
	{
		float dt = TIME.GetDeltaTime();
		Vec3 pos = _gameObject->transform()->GetTransform()->GetWorldPosition();

		if (INPUT.GetButton(KEY_TYPE::W))
			pos += GetTransform()->GetLook() * _speed * dt;

		if (INPUT.GetButton(KEY_TYPE::S))
			pos -= GetTransform()->GetLook() * _speed * dt;

		if (INPUT.GetButton(KEY_TYPE::A))
			pos -= GetTransform()->GetRight() * _speed * dt;

		if (INPUT.GetButton(KEY_TYPE::D))
			pos += GetTransform()->GetRight() * _speed * dt;

		_gameObject->transform()->SetPosition(pos);

		if (INPUT.GetButton(KEY_TYPE::Q))
		{
			Vec3 rotation = GetTransform()->GetLocalRotation();
			rotation.x += dt * 0.5f;
			GetTransform()->SetLocalRotation(rotation);
		}

		if (INPUT.GetButton(KEY_TYPE::E))
		{
			Vec3 rotation = GetTransform()->GetLocalRotation();
			rotation.x -= dt * 0.5f;
			GetTransform()->SetLocalRotation(rotation);
		}

		if (INPUT.GetButton(KEY_TYPE::Z))
		{
			Vec3 rotation = GetTransform()->GetLocalRotation();
			rotation.y += dt * 0.5f;
			GetTransform()->SetLocalRotation(rotation);
		}

		if (INPUT.GetButton(KEY_TYPE::C))
		{
			Vec3 rotation = GetTransform()->GetLocalRotation();
			rotation.y -= dt * 0.5f;
			GetTransform()->SetLocalRotation(rotation);
		}
	}
	/*else if (_gameObject->GetName() == L"MainLight")
	{
		float dt = TIME.GetDeltaTime();
		Vec3 pos = _gameObject->transform()->GetTransform()->GetWorldPosition();

		if (INPUT.GetButton(KEY_TYPE::KEY_1))
			pos += GetTransform()->GetLook() * _speed * dt;

		if (INPUT.GetButton(KEY_TYPE::KEY_2))
			pos -= GetTransform()->GetLook() * _speed * dt;

		if (INPUT.GetButton(KEY_TYPE::KEY_3))
			pos -= GetTransform()->GetRight() * _speed * dt;

		if (INPUT.GetButton(KEY_TYPE::KEY_4))
			pos += GetTransform()->GetRight() * _speed * dt;

		_gameObject->transform()->SetPosition(pos);

	}*/
	
	//if (!INPUT.isMouseOut())
	//{
	//	// 회전 처리
	//	Vec3 rotation = GetTransform()->GetLocalRotation();

	//	// 마우스 위치에 따라 회전
	//	POINT currentMousePos = INPUT.GetMousePos();
	//	POINT deltaMousePos = { currentMousePos.x - _prevMousePos.x, currentMousePos.y - _prevMousePos.y };
	//	rotation.y += deltaMousePos.x * dt * 0.1f; // 마우스 X축 이동량에 따라 Y축 회전
	//	rotation.x -= deltaMousePos.y * dt * 0.1f; // 마우스 Y축 이동량에 따라 X축 회전
	//	_prevMousePos = currentMousePos;

	//	// 회전 각도 제한
	//	rotation.x = clamp(rotation.x, -90.0f, 90.0f);
	//	rotation.y = clamp(rotation.y, -90.0f, 90.0f); // 추가: y축 회전 각도도 제한

	//	GetTransform()->SetLocalRotation(rotation);
	//	_gameObject->transform()->Update();
	//}

}
