#pragma once

class Viewport
{
public:
	Viewport();
	Viewport(float width, float height, float x = 0, float y = 0, float minDepth = 0, float maxDepth = 1);
	~Viewport();

	void RSSetViewport();
	void Set(float width, float height, float x = 0, float y = 0, float minDepth = 0, float maxDepth = 1);

	float GetWidth() { return _vp.Width; }
	float GetHeight() { return _vp.Height; }

	Vec3 Project(const Vec3& pos, const Matrix& W, const Matrix& V, const Matrix& P);
	Vec3 Unproject(const Vec3& pos, const Matrix& W, const Matrix& V, const Matrix& P);
	Ray GetRayFromScreenPoint(float screenX, float screenY, const Matrix& W, const Matrix& V, const Matrix& P, const Vec3& cameraPosition);

private:
	D3D11_VIEWPORT _vp;
};