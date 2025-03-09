#pragma once


enum class KEY_TYPE
{
	UP = VK_UP,
	DOWN = VK_DOWN,
	LEFT = VK_LEFT,
	RIGHT = VK_RIGHT,

	W = 'W',
	A = 'A',
	S = 'S',
	D = 'D',

	Q = 'Q',
	E = 'E',
	Z = 'Z',
	C = 'C',
	R = 'R',

	KEY_1 = '1',
	KEY_2 = '2',
	KEY_3 = '3',
	KEY_4 = '4',

	LBUTTON = VK_LBUTTON,
	RBUTTON = VK_RBUTTON,

	KEY_CTRL = VK_CONTROL,
	ESC = VK_ESCAPE
};

enum class KEY_STATE
{
	NONE,
	PRESS,
	DOWN,
	UP,
	END
};

enum
{
	KEY_TYPE_COUNT = static_cast<int32>(UINT8_MAX + 1),
	KEY_STATE_COUNT = static_cast<int32>(KEY_STATE::END),
};

class InputManager
{
protected:
	InputManager() {}
	virtual ~InputManager() {}
public:
	InputManager(const InputManager&) = delete;
	static InputManager& GetInstance()
	{
		static InputManager instance;
		return instance;
	}

	void Init(HWND hwnd);
	void Update();

	// Scene View ���� �Է� �Լ���
	bool GetSceneButton(KEY_TYPE key);
	bool GetSceneButtonDown(KEY_TYPE key);
	bool GetSceneButtonUp(KEY_TYPE key);
	const POINT& GetSceneMousePos();

	// ������ ���� ��
	bool GetButton(KEY_TYPE key);
	// �� ó�� ������ ��
	bool GetButtonDown(KEY_TYPE key);
	// �� ó�� ������ ���� ��
	bool GetButtonUp(KEY_TYPE key);

	bool GetPublicButton(KEY_TYPE key) { return GetState(key) == KEY_STATE::PRESS; }
	bool GetPublicButtonDown(KEY_TYPE key) { return GetState(key) == KEY_STATE::DOWN; }
	bool GetPublicButtonUp(KEY_TYPE key) { return GetState(key) == KEY_STATE::UP; }
	const POINT& GetPublicMousePos() { return _mousePos; }

	bool isMouseOut() { return isMouseOutsideWindow; }

	const POINT& GetMousePos();
	const POINT& GetSavedMousePos() { return savedMousePos; }
private:
	inline KEY_STATE GetState(KEY_TYPE key) { return _states[static_cast<uint8>(key)]; }

private:
	HWND _hwnd;
	vector<KEY_STATE> _states;
	POINT _mousePos = {};
	bool isMouseOutsideWindow = false; // â �ۿ� ���콺�� �ִ��� ���θ� �����ϴ� ����
	POINT savedMousePos; // â �ۿ� ���콺�� ���� ���� ���������� �νĵ� ���콺 ��ġ�� �����ϴ� ����
	bool isMouseEnter = false;
};

