#include "pch.h"
#include "MovePlayerWithCamera.h"

void MovePlayerWithCamera::Start()
{
    _accumulatedRotY = 180.0f;
    // 카메라의 초기 위치를 플레이어 뒤쪽으로 설정
    if (auto camera = SCENE.GetActiveScene()->Find(L"MainCamera"))
    {
        camera->transform()->SetLocalRotation(Vec3(0.0f, 180.0f, 0.0f));
    }
    _gameObject = GetGameObject();
    _animator = _gameObject->GetComponent<Animator>();
}

void MovePlayerWithCamera::Update()
{
    shared_ptr<GameObject> camera = SCENE.GetActiveScene()->Find(L"MainCamera");
    float dt = ENGINE.IsEditMode() ? TIME.GetEditorDeltaTime() : TIME.GetDeltaTime();

    // ESC 키로 마우스 제어 토글
    if (INPUT.GetButtonDown(KEY_TYPE::ESC))
    {
        _isMouseControlEnabled = !_isMouseControlEnabled;  // 상태 토글
        if (_isMouseControlEnabled)
        {
            ShowCursor(FALSE);  // 마우스 커서 숨김
            ResetMouse();
        }
        else
        {
            ShowCursor(TRUE);   // 마우스 커서 표시
        }
    }

    if (!_isAttacking)
    {
        // 키보드를 이용한 이동
        Vec3 pos = _gameObject->transform()->GetWorldPosition();

        bool isMove = false;
        if (INPUT.GetButton(KEY_TYPE::W))
        {
            pos -= GetTransform()->GetLook() * 1000.0f * dt;
            isMove = true;
        }
        if (INPUT.GetButton(KEY_TYPE::S))
        {
            pos += GetTransform()->GetLook() * 1000.0f * dt;
            isMove = true;
        }
        if (INPUT.GetButton(KEY_TYPE::A))
        {
            pos += GetTransform()->GetRight() * 1000.0f * dt;
            isMove = true;
        }
        if (INPUT.GetButton(KEY_TYPE::D))
        {
            pos -= GetTransform()->GetRight() * 1000.0f * dt;
            isMove = true;
        }
        if (INPUT.GetButton(KEY_TYPE::Q))
        {
            _animator->SetBool("isAttacking", true);
            _animator->SetBool("isMove", false);
            _isAttacking = true;
            isMove = false;
        }
        if (INPUT.GetButton(KEY_TYPE::R))
        {
            shared_ptr<GameObject> prefab = SCENE.LoadPrefabToScene(L"cylinder");
            prefab->transform()->SetLocalPosition(Vec3(GetGameObject()->transform()->GetLocalPosition() + Vec3(0, 0, 10)));
        }
        if (isMove)
            _animator->SetBool("isMove", true);
        else
            _animator->SetBool("isMove", false);

        _gameObject->transform()->SetPosition(pos);
    }

    // 마우스 회전 처리 (마우스 제어가 활성화된 경우에만)
    if (_isMouseControlEnabled)
    {
        if (_isFirstMove)
        {
            ResetMouse();
            _isFirstMove = false;
            ShowCursor(FALSE);  // 초기 상태에서 마우스 커서 숨김
            return;
        }

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
            /*GetTransform()->SetQTRotation(newRotation);
            camera->transform()->SetQTRotation(newRotation);*/
            // 플레이어는 Y축(좌우) 회전만
            Quaternion playerRotation = Quaternion::CreateFromYawPitchRoll(yaw, 0.0f, 0.0f);
            GetTransform()->SetQTRotation(playerRotation);
            
            // 카메라는 로컬 X축(상하) 회전만 추가
            Quaternion cameraRotation = Quaternion::CreateFromYawPitchRoll(0.0f, pitch, 0.0f);
            camera->transform()->SetLocalRotation(Vec3(
                XMConvertToDegrees(pitch),  // X축(상하)
                180.0f,                       // Y축(좌우)
                0.0f                        // Z축
            ));

            POINT center;
            center.x = GetSystemMetrics(SM_CXSCREEN) / 2;
            center.y = GetSystemMetrics(SM_CYSCREEN) / 2;
            SetCursorPos(center.x, center.y);
            _prevMousePos = center;
        }
    }
}

void MovePlayerWithCamera::ResetMouse()
{
    // 마우스 커서를 화면 중앙으로 설정
    POINT center;
    center.x = GetSystemMetrics(SM_CXSCREEN) / 2;
    center.y = GetSystemMetrics(SM_CYSCREEN) / 2;
    SetCursorPos(center.x, center.y);
    _prevMousePos = center;
}