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

	// ��ũ�� �������� ���� �� ���� �����ؼ� (0,0)�ε�
	// NDC���������� �߾��� �������� (0,0)��.
	// ���� ȭ�� ũ�Ⱑ 800x600�̴ϱ� 
	// NDC���������� x�� -400 ~ 400, y�� -300~300�� ��.
	float x = screenPos.x - width / 2;
	float y = height / 2 - screenPos.y;
	_ndcPos = Vec3(x, y, 0);
	_size = Vec3(size.x, size.y, 1.0f);

	// screen�������� �߽��� �������� left top, right bottom ���ϱ�
	_rect.left = screenPos.x - size.x / 2;
	_rect.right = screenPos.x + size.x / 2;
	_rect.top = screenPos.y - size.y / 2;
	_rect.bottom = screenPos.y + size.y / 2;
}
