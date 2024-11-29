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
    if (INPUT.GetButton(KEY_TYPE::RBUTTON))
    {
        if (!_isResetMouse)
        {
            ResetMouse();
            _isResetMouse = true;
        }
        float dt = TIME.GetDeltaTime();

        if (auto gameObject = GetGameObject())
        {
            // Ű���带 �̿��� �̵�
            Vec3 pos = gameObject->transform()->GetWorldPosition();

            if (INPUT.GetButton(KEY_TYPE::W))
                pos += GetTransform()->GetLook() * _speed * dt;

            if (INPUT.GetButton(KEY_TYPE::S))
                pos -= GetTransform()->GetLook() * _speed * dt;

            if (INPUT.GetButton(KEY_TYPE::A))
                pos -= GetTransform()->GetRight() * _speed * dt;

            if (INPUT.GetButton(KEY_TYPE::D))
                pos += GetTransform()->GetRight() * _speed * dt;

            gameObject->transform()->SetPosition(pos);

            // ���콺 ȸ�� ó��
            POINT cursorPos;
            if (GetCursorPos(&cursorPos))
            {
                float deltaX = static_cast<float>(cursorPos.x - _prevMousePos.x);
                float deltaY = static_cast<float>(cursorPos.y - _prevMousePos.y);

                if (abs(deltaX) > 1000.f || abs(deltaY) > 1000.f)
                {
                    _prevMousePos = cursorPos;
                    return;
                }

                float MOUSE_SENSITIVITY = 0.3f;

                // ���� ������ ���� ������Ʈ
                _accumulatedRotX += deltaY * MOUSE_SENSITIVITY;
                _accumulatedRotY += deltaX * MOUSE_SENSITIVITY;

                // X�� ȸ��(Pitch) ����
                _accumulatedRotX = Clamp(_accumulatedRotX, -89.0f, 89.0f);

                // ȸ�� ����
                float pitch = XMConvertToRadians(_accumulatedRotX);
                float yaw = XMConvertToRadians(_accumulatedRotY);

                Quaternion newRotation = Quaternion::CreateFromYawPitchRoll(yaw, pitch, 0.0f);
                GetTransform()->SetQTRotation(newRotation);

                _prevMousePos = cursorPos;
            }
            else
            {
                char buffer[100];
                sprintf_s(buffer, "Failed to get cursor position.\n");
                OutputDebugStringA(buffer);
            }
        }
        
    }
    else
    {
        _isResetMouse = false;
        _isFirstMove = true;
    }
}

void MoveObject::ResetMouse()
{
    POINT cursorPos;
    GetCursorPos(&cursorPos);
    _prevMousePos = cursorPos;
}

void MoveObject::InitializeRotationFromTransform()
{
    // ���� Transform�� ȸ������ �����ͼ� ������ �ʱ�ȭ
    Quaternion currentRotation = GetTransform()->GetQTRotation();
    Vec3 eulerAngles = GetTransform()->ToEulerAngles(currentRotation);

    // ������ ��(degree)�� ��ȯ
    _accumulatedRotX = XMConvertToDegrees(eulerAngles.x);
    _accumulatedRotY = XMConvertToDegrees(eulerAngles.y);
}
