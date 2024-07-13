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
	SetViewProjectionMatrix();
}

void Camera::Start()
{
}

void Camera::LateUpdate()
{
	
}

void Camera::SetViewProjectionMatrix()
{
	Vec3 eye = GetTransform()->GetWorldPosition();
	Vec3 at = eye + GetTransform()->GetLook();
	Vec3 up = GetTransform()->GetUp();
	_matView = ::XMMatrixLookAtLH(eye, at, up);

	int width = Graphics::GetInstance().GetViewWidth();
	int height = Graphics::GetInstance().GetViewHeight();

	float aspectRatio = (float)(width) / (float)(height);

	float fovAngleY = XM_PI / 4.f;
	float nearZ = 0.1f;
	float farZ = 1000.f;
	if (_type == ProjectionType::Perspective)
		_matProjcetion = ::XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearZ, farZ);
	else
	{
		nearZ = 1.f;
		farZ = 100.f;
		_matProjcetion = ::XMMatrixOrthographicLH(width, height, nearZ, farZ);
	}
		

	_cameraBuffer = make_shared<Buffer>();
	CameraBuffer _cameraBufferData;
	_cameraBufferData.viewMatrix = _matView;
	_cameraBufferData.projectionMatrix = _matProjcetion;
	_cameraBuffer->CreateConstantBuffer<CameraBuffer>();
	_cameraBuffer->CopyData(_cameraBufferData);
}
