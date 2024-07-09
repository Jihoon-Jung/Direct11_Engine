#include "pch.h"
#include "UIImage.h"

UIImage::UIImage()
	: Super(ComponentType::UIImage)
{
}

UIImage::~UIImage()
{
}

void UIImage::SetTransformAndRect(Vec2 screenPos, Vec2 size)
{
	float width = GP.GetViewWidth();
	float height = GP.GetViewHeight();

	// 스크린 공간에선 왼쪽 위 부터 시작해서 (0,0)인데
	// NDC공간에서는 중앙을 기준으로 (0,0)임.
	// 현재 화면 크기가 800x600이니까 
	// NDC공간에서는 x가 -400 ~ 400, y가 -300~300이 됨.
	float x = screenPos.x - width / 2;
	float y = height / 2 - screenPos.y;
	_ndcPos = Vec3(x, y, 0);
	_size = Vec3(size.x, size.y, 1.0f);

	// screen공간에서 중심을 기준으로 left top, right bottom 구하기
	_rect.left = screenPos.x - size.x / 2;
	_rect.right = screenPos.x + size.x / 2;
	_rect.top = screenPos.y - size.y / 2;
	_rect.bottom = screenPos.y + size.y / 2;
}
