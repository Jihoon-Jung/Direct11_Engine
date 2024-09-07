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
    if (INPUT.GetButton(KEY_TYPE::RBUTTON) && INPUT.GetButton(KEY_TYPE::KEY_CTRL))
    {
        if (!_isResetMouse)
        {
            ResetMouse();
            _isResetMouse = true;
        }
        float dt = TIME.GetDeltaTime();

        // Ű���带 �̿��� �̵�
        Vec3 pos = _gameObject->transform()->GetWorldPosition();

        if (INPUT.GetButton(KEY_TYPE::W))
            pos += GetTransform()->GetLook() * _speed * dt;

        if (INPUT.GetButton(KEY_TYPE::S))
            pos -= GetTransform()->GetLook() * _speed * dt;

        if (INPUT.GetButton(KEY_TYPE::A))
            pos -= GetTransform()->GetRight() * _speed * dt;

        if (INPUT.GetButton(KEY_TYPE::D))
            pos += GetTransform()->GetRight() * _speed * dt;

        _gameObject->transform()->SetPosition(pos);

        // ���콺 ȸ�� ó��
        POINT cursorPos;
        if (GetCursorPos(&cursorPos))
        {
            // ���콺 �̵� ���� ���
            float deltaX = static_cast<float>(cursorPos.x - _prevMousePos.x);
            float deltaY = static_cast<float>(cursorPos.y - _prevMousePos.y);

            if (abs(deltaX) > 1000.f || abs(deltaY) > 1000.f)
            {
                // ���� ���콺 ��ġ ������Ʈ�� �ϰ� return
                _prevMousePos = cursorPos;
                return;
            }

            // ���콺 �����ӿ� ���� ȸ�� ���� (Yaw �� Pitch)
            Vec3 rotation = GetTransform()->GetLocalRotation();

            float MOUSE_SENSITIVITY_X = 10.1f;
            float MOUSE_SENSITIVITY_Y = 10.1f;

            // Yaw ȸ�� ���� (�¿� ȸ��)
            rotation.y += deltaX * MOUSE_SENSITIVITY_X * dt;

            // Pitch ȸ�� ���� (���� ȸ��)
            rotation.x += deltaY * MOUSE_SENSITIVITY_Y * dt;

            // ��ġ �� �����Ͽ� ī�޶� �������� �ʵ��� ��
            //rotation.x = clamp(rotation.x, -XM_PIDIV2, XM_PIDIV2);

            // ȸ�� ����
            GetTransform()->SetRotation(rotation);

            // ���� ���콺 ��ġ ������Ʈ
            _prevMousePos = cursorPos;
        }
        else
        {
            char buffer[100];
            sprintf_s(buffer, "Failed to get cursor position.\n");
            OutputDebugStringA(buffer);
        }
    }
    else
        _isResetMouse = false;
}

void MoveObject::ResetMouse()
{
    POINT cursorPos;
    GetCursorPos(&cursorPos);
    _prevMousePos = cursorPos;
}




