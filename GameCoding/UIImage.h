#pragma once
#include "Component.h"
class UIImage : public Component
{
	using Super = Component;
public:

	UIImage();
	virtual ~UIImage();
	void SetTransformAndRect(Vec2 screenPos, Vec2 size);
	Vec3 GetNDCPosition() { return _ndcPos; }
	Vec3 GetSize() { return _size; }
private:
	Vec3 _size;
	Vec3 _ndcPos;
	RECT _rect;
};

