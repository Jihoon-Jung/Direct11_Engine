#include "pch.h"
#include "Viewport.h"
Viewport::Viewport()
{
	Set(GWinSizeX, GWinSizeY);
}

Viewport::Viewport(float width, float height, float x, float y, float minDepth, float maxDepth)
{
	Set(width, height, x, y, minDepth, maxDepth);
}

Viewport::~Viewport()
{

}

void Viewport::RSSetViewport()
{
	DEVICECONTEXT->RSSetViewports(1, &_vp);
}

void Viewport::Set(float width, float height, float x, float y, float minDepth, float maxDepth)
{
	_vp.TopLeftX = x;
	_vp.TopLeftY = y;
	_vp.Width = width;
	_vp.Height = height;
	_vp.MinDepth = minDepth;
	_vp.MaxDepth = maxDepth;
}

Vec3 Viewport::Project(const Vec3& pos, const Matrix& W, const Matrix& V, const Matrix& P)
{
	// 3D -> 2D ( viewport 상의 위치로 변환)
	Matrix wvp = W * V * P;
	Vec3 p = Vec3::Transform(pos, wvp);

	p.x = (p.x + 1.0f) * (_vp.Width / 2) + _vp.TopLeftX;
	p.y = (-p.y + 1.0f) * (_vp.Height / 2) + _vp.TopLeftY;
	p.z = p.z * (_vp.MaxDepth - _vp.MinDepth) + _vp.MinDepth;

	return p;
}

Vec3 Viewport::Unproject(const Vec3& pos, const Matrix& W, const Matrix& V, const Matrix& P)
{
	// 2D -> 3D (깊이 좌표를 가지고있는? viewport 상의 위치에서 world공간의 위치로 변환)
	Vec3 p = pos;

	p.x = 2.f * (p.x - _vp.TopLeftX) / _vp.Width - 1.f;
	p.y = -2.f * (p.y - _vp.TopLeftY) / _vp.Height + 1.f;
	p.z = ((p.z - _vp.MinDepth) / (_vp.MaxDepth - _vp.MinDepth));

	Matrix wvp = W * V * P;
	Matrix wvpInv = wvp.Invert();

	p = Vec3::Transform(p, wvpInv);
	return p;
}

Ray Viewport::GetRayFromScreenPoint(float screenX, float screenY, const Matrix& W, const Matrix& V, const Matrix& P, const Vec3& cameraPosition)
{
	Vec3 nearPoint = Unproject(Vec3(screenX, screenY, 0.0f), Matrix::Identity, V, P);
	Vec3 farPoint = Unproject(Vec3(screenX, screenY, 1.0f), Matrix::Identity, V, P);

	Vec3 direction = farPoint - nearPoint;
	direction.Normalize();

	return Ray(cameraPosition, direction);
}
