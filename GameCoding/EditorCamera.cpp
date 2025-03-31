#include "pch.h"
#include "EditorCamera.h"

EditorCamera::EditorCamera()
{
}

EditorCamera::~EditorCamera()
{
}

void EditorCamera::Start()
{
}

void EditorCamera::Update()
{
    if (!GUI.isSceneView()/*ENGINE.GetEngineMode() == EngineMode::Play*/)
        return;

    if (INPUT.GetSceneButton(KEY_TYPE::RBUTTON))
    {
        if (!_isResetMouse)
        {
            ResetMouse();
            _isResetMouse = true;
        }

        float dt = (ENGINE.IsEditMode() || ENGINE.IsPausedMode()) ? TIME.GetEditorDeltaTime() : TIME.GetDeltaTime();

        if (auto gameObject = GetGameObject())
        {
            Vec3 pos = gameObject->transform()->GetWorldPosition();

            // Scene View ���� �Է� ���
            if (INPUT.GetSceneButton(KEY_TYPE::W))
                pos += GetTransform()->GetLook() * _speed * dt;

            if (INPUT.GetSceneButton(KEY_TYPE::S))
                pos -= GetTransform()->GetLook() * _speed * dt;

            if (INPUT.GetSceneButton(KEY_TYPE::A))
                pos -= GetTransform()->GetRight() * _speed * dt;

            if (INPUT.GetSceneButton(KEY_TYPE::D))
                pos += GetTransform()->GetRight() * _speed * dt;

            gameObject->transform()->SetPosition(pos);

            // Scene View ���� ���콺 ó��
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

                _accumulatedRotX += deltaY * MOUSE_SENSITIVITY;
                _accumulatedRotY += deltaX * MOUSE_SENSITIVITY;

                _accumulatedRotX = Clamp(_accumulatedRotX, -89.0f, 89.0f);

                float pitch = XMConvertToRadians(_accumulatedRotX);
                float yaw = XMConvertToRadians(_accumulatedRotY);

                Quaternion newRotation = Quaternion::CreateFromYawPitchRoll(yaw, pitch, 0.0f);
                GetTransform()->SetQTLocaslRotation(newRotation);

                _prevMousePos = cursorPos;
            }
        }
    }
    else
    {
        _isResetMouse = false;
        _isFirstMove = true;
    }
}

void EditorCamera::ResetMouse()
{
    POINT cursorPos;
    GetCursorPos(&cursorPos);
    _prevMousePos = cursorPos;
}

void EditorCamera::InitializeRotationFromTransform()
{
    // ���� Transform�� ȸ������ �����ͼ� ������ �ʱ�ȭ
    Quaternion currentRotation = GetTransform()->GetQTRotation();
    Vec3 eulerAngles = GetTransform()->ToEulerAngles(currentRotation);

    // ������ ��(degree)�� ��ȯ
    _accumulatedRotX = XMConvertToDegrees(eulerAngles.x);
    _accumulatedRotY = XMConvertToDegrees(eulerAngles.y);
}
