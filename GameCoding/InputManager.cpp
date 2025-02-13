#include "pch.h"
#include "InputManager.h"

void InputManager::Init(HWND hwnd)
{
	_hwnd = hwnd;
	_states.resize(KEY_TYPE_COUNT, KEY_STATE::NONE);
}

void InputManager::Update()
{
    // 키보드 상태 업데이트
    BYTE asciiKeys[KEY_TYPE_COUNT] = {};
    if (::GetKeyboardState(asciiKeys))
    {
        for (uint32 key = 0; key < KEY_TYPE_COUNT; key++)
        {
            KEY_STATE& state = _states[key];

            if (asciiKeys[key] & 0x80) // 키가 눌려있음
            {
                if (state == KEY_STATE::NONE || state == KEY_STATE::UP)
                    state = KEY_STATE::DOWN;
                else if (state == KEY_STATE::DOWN)
                    state = KEY_STATE::PRESS;
            }
            else // 키가 눌려있지 않음
            {
                if (state == KEY_STATE::PRESS || state == KEY_STATE::DOWN)
                    state = KEY_STATE::UP;
                else
                    state = KEY_STATE::NONE;
            }
        }
    }

    // 이전 마우스 위치 저장
    POINT prevMousePos = _mousePos;

    // 현재 마우스 위치 업데이트
    ::GetCursorPos(&_mousePos);
    ::ScreenToClient(_hwnd, &_mousePos);

    // 클라이언트 영역 체크
    RECT clientRect;
    GetClientRect(_hwnd, &clientRect);

    bool prevMouseOutside = isMouseOutsideWindow;
    isMouseOutsideWindow = !PtInRect(&clientRect, _mousePos);

    // 마우스가 창 밖으로 나갔을 때만 마지막 위치 저장
    if (!prevMouseOutside && isMouseOutsideWindow)
    {
        savedMousePos = prevMousePos;
    }
}

// Scene View 전용 입력 함수들
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

// 누르고 있을 때
bool InputManager::GetButton(KEY_TYPE key) { return !GUI.isSceneView() && GetState(key) == KEY_STATE::PRESS; }
// 맨 처음 눌렀을 때
bool InputManager::GetButtonDown(KEY_TYPE key) { return !GUI.isSceneView() && GetState(key) == KEY_STATE::DOWN; }
// 맨 처음 눌렀다 뗐을 때
bool InputManager::GetButtonUp(KEY_TYPE key) { return !GUI.isSceneView() && GetState(key) == KEY_STATE::UP; }

const POINT& InputManager::GetMousePos()
{
    return !GUI.isSceneView() ? _mousePos : POINT{ 0, 0 };
}

//const POINT& GetMousePos()
//{
//    return !GUI.isSceneView() ? _mousePos : POINT{ 0, 0 };
//}