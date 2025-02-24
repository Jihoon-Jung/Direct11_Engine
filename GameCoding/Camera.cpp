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
	/*SetViewProjectionMatrix();*/
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

	int width = Graphics::GetInstance().GetViewWidth();
	int height = Graphics::GetInstance().GetViewHeight();

	float aspectRatio = (float)(width) / (float)(height);

	float fovAngleY = XM_PI / 4.f;
	float nearZ = 0.1f;
	float farZ = 1000.f;
	
	_aspectRatio = aspectRatio;
	_fov = fovAngleY;
	_nearZ = nearZ;
	_farZ = farZ;

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

	SetShadowMapViewProjectionMatrix();
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

void Camera::SetShadowMapViewProjectionMatrix()
{
	shared_ptr<GameObject> light = SCENE.GetActiveScene()->GetMainLight();
	Vec3 main_cameraPos = SCENE.GetActiveScene()->GetMainCamera()->transform()->GetWorldPosition();
	Vec3 eye = light->transform()->GetWorldPosition();
	Vec3 at = Vec3(main_cameraPos.x, 0, main_cameraPos.z);
	Vec3 up = Vec3(0.0f, 1.0f, 0.0f);
	_shadowView = ::XMMatrixLookAtLH(eye, at, up);

	int width = GP.GetProjectWidth();
	int height = GP.GetProjectHeight();
	float nearZ = 1.0f;
	float farZ = 100.0f;
	float orthoSize = 30.0f;
	float aspect = width / height;

	_shadowProjection = ::XMMatrixOrthographicLH(orthoSize * aspect, orthoSize, nearZ, farZ);

	
	_shadowCameraBuffer = make_shared<Buffer>();
	CameraBuffer _cameraBufferData;
	_cameraBufferData.viewMatrix = _shadowView;
	_cameraBufferData.projectionMatrix = _shadowProjection;
	_shadowCameraBuffer->CreateConstantBuffer<CameraBuffer>();
	_shadowCameraBuffer->CopyData(_cameraBufferData);
}
