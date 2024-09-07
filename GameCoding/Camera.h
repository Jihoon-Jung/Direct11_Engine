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

	void SetProjectionType(ProjectionType type) { _type = type; }
	Matrix GetViewMatrix() { return _matView; }
	Matrix GetProjectionMatrix() { return _matProjcetion; }
	shared_ptr<Buffer> GetCameraBuffer() { return _cameraBuffer; }
	shared_ptr<Buffer> GetEnvironmentCameraBuffer() { return _environmentCameraBuffer; }
private:

	Matrix _matView;
	Matrix _matProjcetion;
	Matrix _envMatView;
	Matrix _envMatProjection;

	shared_ptr<Buffer> _cameraBuffer;
	shared_ptr<Buffer> _environmentCameraBuffer;
	ProjectionType _type;
};

