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

        // 키보드를 이용한 이동
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

        // 마우스 회전 처리
        POINT cursorPos;
        if (GetCursorPos(&cursorPos))
        {
            // 마우스 이동 차이 계산
            float deltaX = static_cast<float>(cursorPos.x - _prevMousePos.x);
            float deltaY = static_cast<float>(cursorPos.y - _prevMousePos.y);

            if (abs(deltaX) > 1000.f || abs(deltaY) > 1000.f)
            {
                // 이전 마우스 위치 업데이트만 하고 return
                _prevMousePos = cursorPos;
                return;
            }

            // 마우스 움직임에 따른 회전 적용 (Yaw 및 Pitch)
            Vec3 rotation = GetTransform()->GetLocalRotation();

            float MOUSE_SENSITIVITY_X = 10.1f;
            float MOUSE_SENSITIVITY_Y = 10.1f;

            // Yaw 회전 적용 (좌우 회전)
            rotation.y += deltaX * MOUSE_SENSITIVITY_X * dt;

            // Pitch 회전 적용 (상하 회전)
            rotation.x += deltaY * MOUSE_SENSITIVITY_Y * dt;

            // 피치 값 제한하여 카메라가 뒤집히지 않도록 함
            //rotation.x = clamp(rotation.x, -XM_PIDIV2, XM_PIDIV2);

            // 회전 적용
            GetTransform()->SetRotation(rotation);

            // 이전 마우스 위치 업데이트
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




