#pragma once
#include "Component.h"

class Button : public Component
{
	using Super = Component;
public:
	Button();
	virtual ~Button();

	bool Picked(POINT screenPos);
	void AddOnClickedEvent(const string& functionKey, MonoBehaviour* script);
	void InvokeOnClicked();
	void SetTransformAndRect(Vec2 screenPos, Vec2 size);
	void SetRect(RECT rect) { _rect = rect; }
	void SetScreenTransformAndRect(Vec2 pos, Vec2 size, RECT rect);
	void UpdateRect(Vec2 ndcPos, Vec2 size);
	Vec3 GetNDCPosition() { return _ndcPos; }
	Vec3 GetSize() { return _size; }
	RECT GetRect() { return _rect; }
	string GetOnClickedFuntionKey() { return _onClickedFunctionKey; }

private:
	string _onClickedFunctionKey;
	MonoBehaviour* _onClickedScript = nullptr;
	Vec3 _size;
	Vec3 _ndcPos;
	RECT _rect;
};

