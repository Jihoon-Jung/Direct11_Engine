#pragma once

enum class ProjectionType
{
	Perspective,
	Orthographic
};

class Camera : public Component
{
	using Super = Component;
public:
	Camera();
	virtual ~Camera();

	virtual void Update() override;
	virtual void Start() override;
	virtual void LateUpdate() override;
	void SetViewProjectionMatrix();
	void SetEnvironmentMapViewProjectionMatrix(Vec3 worldPosition, Vec3 lookVector, Vec3 upVector);
	void SetShadowMapViewProjectionMatrix();
	void SetProjectionType(ProjectionType type) { _type = type; }
	Matrix GetViewMatrix() { return _matView; }
	Matrix GetProjectionMatrix() { return _matProjcetion; }
	Matrix GetEnvViewMatrix() { return _envMatView; }
	Matrix GetEnvProjectionMatrix() { return _envMatProjection; }
	shared_ptr<Buffer> GetCameraBuffer() { return _cameraBuffer; }
	shared_ptr<Buffer> GetEnvironmentCameraBuffer() { return _environmentCameraBuffer; }
	shared_ptr<Buffer> GetShadowCameraBuffer() { return _shadowCameraBuffer; }
private:

	Matrix _matView;
	Matrix _matProjcetion;
	Matrix _envMatView;
	Matrix _envMatProjection;
	Matrix _shadowView;
	Matrix _shadowProjection;

	shared_ptr<Buffer> _cameraBuffer;
	shared_ptr<Buffer> _environmentCameraBuffer;
	shared_ptr <Buffer> _shadowCameraBuffer;

	ProjectionType _type;

	BoundingSphere _sceneBounds;
};

