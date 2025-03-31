#pragma once
#include "Component.h"
class UIImage : public Component
{
	using Super = Component;
public:

	UIImage();
	virtual ~UIImage();
	bool Picked(POINT screenPos);
	void SetRect(RECT rect) { _rect = rect; }
	void SetTransformAndRect(Vec2 screenPos, Vec2 size);
	void SetScreenTransformAndRect(Vec2 pos, Vec2 size, RECT rect);
	void UpdateRect(Vec2 ndcPos, Vec2 size);
	Vec3 GetNDCPosition() { return _ndcPos; }
	Vec3 GetSize() { return _size; }
	RECT GetRect() { return _rect; }
private:
	Vec3 _size;
	Vec3 _ndcPos;
	RECT _rect;

};

