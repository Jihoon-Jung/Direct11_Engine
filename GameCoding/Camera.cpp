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

void Camera::SetEnvironmentMapViewProjectionMatrix(Vec3 worldPosition, Vec3 lookVector, Vec3 upVector)
{
	Vec3 eye = worldPosition;
	Vec3 at = eye + lookVector;
	Vec3 up = upVector;
	_envMatView = ::XMMatrixLookAtLH(eye, at, up);

	int width = Graphics::GetInstance().GetEnvironmentMapWidth();
	int height = Graphics::GetInstance().GetEnvironmentMapHeight();

	float aspectRatio = (float)(width) / (float)(height);

	float fovAngleY = 0.5f * XM_PI;
	float nearZ = 0.1f;
	float farZ = 1000.f;
	if (_type == ProjectionType::Perspective)
		_envMatProjection = ::XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearZ, farZ);
	else
	{
		nearZ = 1.f;
		farZ = 100.f;
		_envMatProjection = ::XMMatrixOrthographicLH(width, height, nearZ, farZ);
	}


	_environmentCameraBuffer = make_shared<Buffer>();
	CameraBuffer _cameraBufferData;
	_cameraBufferData.viewMatrix = _envMatView;
	_cameraBufferData.projectionMatrix = _envMatProjection;
	_environmentCameraBuffer->CreateConstantBuffer<CameraBuffer>();
	_environmentCameraBuffer->CopyData(_cameraBufferData);
}
