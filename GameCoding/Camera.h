#pragma once

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

	Matrix GetViewMatrix() { return _matView; }
	Matrix GetProjectionMatrix() { return _matProjcetion; }
	shared_ptr<Buffer> GetCameraBuffer() { return _cameraBuffer; }
private:

	Matrix _matView;
	Matrix _matProjcetion;
	shared_ptr<Buffer> _cameraBuffer;
};

