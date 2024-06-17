#include "pch.h"
#include "Camera.h"
#include "ConstantBufferType.h"
#include "Buffer.h"

Camera::Camera()
	:Super(ComponentType::Camera)
{
}

Camera::~Camera()
{
}

void Camera::Update()
{
}

void Camera::Start()
{
}

void Camera::LateUpdate()
{
	SetViewProjectionMatrix();
}

void Camera::SetViewProjectionMatrix()
{
	Vec3 eye = GetTransform()->GetWorldPosition();
	Vec3 at = eye + GetTransform()->GetLook();
	Vec3 up = GetTransform()->GetUp();
	_matView = ::XMMatrixLookAtLH(eye, at, up);

	float aspectRatio = (float)(Graphics::GetInstance().GetViewWidth()) / (float)(Graphics::GetInstance().GetViewHeight());

	float fovAngleY = XM_PI / 4.f;
	float nearZ = 0.1f;
	float farZ = 1000.f;
	_matProjcetion = ::XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearZ, farZ);

	_cameraBuffer = make_shared<Buffer>();
	CameraBuffer _cameraBufferData;
	_cameraBufferData.viewMatrix = _matView;
	_cameraBufferData.projectionMatrix = _matProjcetion;
	_cameraBuffer->CreateConstantBuffer<CameraBuffer>();
	_cameraBuffer->CopyData(_cameraBufferData);
}
