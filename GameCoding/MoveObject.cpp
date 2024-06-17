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
	//	// ȸ�� ó��
	//	Vec3 rotation = GetTransform()->GetLocalRotation();

	//	// ���콺 ��ġ�� ���� ȸ��
	//	POINT currentMousePos = INPUT.GetMousePos();
	//	POINT deltaMousePos = { currentMousePos.x - _prevMousePos.x, currentMousePos.y - _prevMousePos.y };
	//	rotation.y += deltaMousePos.x * dt * 0.1f; // ���콺 X�� �̵����� ���� Y�� ȸ��
	//	rotation.x -= deltaMousePos.y * dt * 0.1f; // ���콺 Y�� �̵����� ���� X�� ȸ��
	//	_prevMousePos = currentMousePos;

	//	// ȸ�� ���� ����
	//	rotation.x = clamp(rotation.x, -90.0f, 90.0f);
	//	rotation.y = clamp(rotation.y, -90.0f, 90.0f); // �߰�: y�� ȸ�� ������ ����

	//	GetTransform()->SetLocalRotation(rotation);
	//	_gameObject->transform()->Update();
	//}

}
