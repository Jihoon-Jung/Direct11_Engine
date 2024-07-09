#pragma once
#include "Component.h"
class Button : public Component
{
	using Super = Component;
public:
	Button();
	virtual ~Button();

	bool Picked(POINT screenPos);
	void AddOnClickedEvent(std::function<void(void)> func);
	void InvokeOnClicked();
	void SetTransformAndRect(Vec2 screenPos, Vec2 size);
	Vec3 GetNDCPosition() { return _ndcPos; }
	Vec3 GetSize() { return _size; }
private:
	std::function<void(void)> _onClicked;
	Vec3 _size;
	Vec3 _ndcPos;
	RECT _rect;
};

