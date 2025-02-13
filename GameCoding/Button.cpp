#include "pch.h"
#include "Button.h"

Button::Button()
	: Super(ComponentType::Button)
{

}

Button::~Button()
{
}

bool Button::Picked(POINT screenPos)
{
	screenPos.x -= GP.GetProjectWidth() * (1.0f / 10.0f);
	screenPos.y -= GP.GetProjectHeight() * (3.0f / 100.0f);

	bool result = ::PtInRect(&_rect, screenPos);
	return result;
}

void Button::AddOnClickedEvent(const string& functionKey)
{
	_onClickedFunctionKey = functionKey;
}

void Button::InvokeOnClicked()
{
	if (!_onClickedFunctionKey.empty())
	{
		if (auto method = MR.getMethod(_onClickedFunctionKey))
		{
			method(nullptr);
		}
	}
}

void Button::SetTransformAndRect(Vec2 screenPos, Vec2 size)
{
	float width = GP.GetViewWidth();
	float height = GP.GetViewHeight();

	float x = screenPos.x - width / 2;
	float y = height / 2 - screenPos.y;
	_ndcPos = Vec3(x, y, 0);
	size /= 2.0f;
	_size = Vec3(size.x, size.y, 1.0f);

	_rect.left = screenPos.x - size.x;
	_rect.right = screenPos.x + size.x;
	_rect.top = screenPos.y - size.y;
	_rect.bottom = screenPos.y + size.y;

}

void Button::SetScreenTransformAndRect(Vec2 pos, Vec2 size, RECT rect)
{
	_ndcPos = Vec3(pos.x, pos.y, 0);
	_size = Vec3(size.x, size.y, 0);
	_rect = rect;
}

void Button::UpdateRect(Vec2 ndcPos, Vec2 size)
{
	float width = GP.GetViewWidth();
	float height = GP.GetViewHeight();

	float screenPosX = ndcPos.x + width / 2;
	float screenPosY = height / 2 - ndcPos.y;

	_rect.left = screenPosX - size.x;
	_rect.right = screenPosX + size.x;
	_rect.top = screenPosY - size.y;
	_rect.bottom = screenPosY + size.y;

}
