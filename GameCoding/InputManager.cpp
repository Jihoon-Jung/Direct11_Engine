#include "pch.h"
#include "InputManager.h"

void InputManager::Init(HWND hwnd)
{
	_hwnd = hwnd;
	_states.resize(KEY_TYPE_COUNT, KEY_STATE::NONE);
}

void InputManager::Update()
{
    // Ű���� ���� ������Ʈ
    BYTE asciiKeys[KEY_TYPE_COUNT] = {};
    if (::GetKeyboardState(asciiKeys))
    {
        for (uint32 key = 0; key < KEY_TYPE_COUNT; key++)
        {
            KEY_STATE& state = _states[key];

            if (asciiKeys[key] & 0x80) // Ű�� ��������
            {
                if (state == KEY_STATE::NONE || state == KEY_STATE::UP)
                    state = KEY_STATE::DOWN;
                else if (state == KEY_STATE::DOWN)
                    state = KEY_STATE::PRESS;
            }
            else // Ű�� �������� ����
            {
                if (state == KEY_STATE::PRESS || state == KEY_STATE::DOWN)
                    state = KEY_STATE::UP;
                else
                    state = KEY_STATE::NONE;
            }
        }
    }

    // ���� ���콺 ��ġ ����
    POINT prevMousePos = _mousePos;

    // ���� ���콺 ��ġ ������Ʈ
    ::GetCursorPos(&_mousePos);
    ::ScreenToClient(_hwnd, &_mousePos);

    // Ŭ���̾�Ʈ ���� üũ
    RECT clientRect;
    GetClientRect(_hwnd, &clientRect);

    bool prevMouseOutside = isMouseOutsideWindow;
    isMouseOutsideWindow = !PtInRect(&clientRect, _mousePos);

    // ���콺�� â ������ ������ ���� ������ ��ġ ����
    if (!prevMouseOutside && isMouseOutsideWindow)
    {
        savedMousePos = prevMousePos;
    }
}

// Scene View ���� �Է� �Լ���
bool InputManager::GetSceneButton(KEY_TYPE key) { return GUI.isSceneView() && GetState(key) == KEY_STATE::PRESS; }
bool InputManager::GetSceneButtonDown(KEY_TYPE key) { return GUI.isSceneView() && GetState(key) == KEY_STATE::DOWN; }
bool InputManager::GetSceneButtonUp(KEY_TYPE key) { return GUI.isSceneView() && GetState(key) == KEY_STATE::UP; }
const POINT& InputManager::GetSceneMousePos()
{
    return GUI.isSceneView() ? _mousePos : POINT{ 0, 0 };
}
//const POINT::& GetSceneMousePos()
//{
//    return GUI.isSceneView() ? _mousePos : POINT{ 0, 0 };
//}

// ������ ���� ��
bool InputManager::GetButton(KEY_TYPE key) { return !GUI.isSceneView() && GetState(key) == KEY_STATE::PRESS; }
// �� ó�� ������ ��
bool InputManager::GetButtonDown(KEY_TYPE key) { return !GUI.isSceneView() && GetState(key) == KEY_STATE::DOWN; }
// �� ó�� ������ ���� ��
bool InputManager::GetButtonUp(KEY_TYPE key) { return !GUI.isSceneView() && GetState(key) == KEY_STATE::UP; }

const POINT& InputManager::GetMousePos()
{
    return !GUI.isSceneView() ? _mousePos : POINT{ 0, 0 };
}

//const POINT& GetMousePos()
//{
//    return !GUI.isSceneView() ? _mousePos : POINT{ 0, 0 };
//}