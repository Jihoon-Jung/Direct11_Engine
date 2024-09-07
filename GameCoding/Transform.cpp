#include "pch.h"
#include "Transform.h"
#include "ConstantBufferType.h"

Transform::Transform()
	:Super(ComponentType::Transform)
{
}

Transform::~Transform()
{
}
void Transform::Start()
{
}
void Transform::Update()
{
	UpdateTransform();
}
Vec3 ToEulerAngles(Quaternion q)
{
	Vec3 angles;

	// roll (x-axis rotation)
	double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
	double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
	angles.x = std::atan2(sinr_cosp, cosr_cosp);

	// pitch (y-axis rotation)
	double sinp = std::sqrt(1 + 2 * (q.w * q.y - q.x * q.z));
	double cosp = std::sqrt(1 - 2 * (q.w * q.y - q.x * q.z));
	angles.y = 2 * std::atan2(sinp, cosp) - 3.14159f / 2;

	// yaw (z-axis rotation)
	double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
	double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
	angles.z = std::atan2(siny_cosp, cosy_cosp);

	return angles;
}
void Transform::UpdateTransform()
{
	//if (HasParent())
	//{
	//	_localPosition = Vec3::Transform(_worldPosition, _parent->GetWorldMatrix().Invert());
	//}
	Matrix scale = Matrix::CreateScale(_localScale);
	Matrix rotation = Matrix::CreateRotationX(DirectX::XMConvertToRadians(_localRotation.x));
	rotation *= Matrix::CreateRotationY(DirectX::XMConvertToRadians(_localRotation.y));
	rotation *= Matrix::CreateRotationZ(DirectX::XMConvertToRadians(_localRotation.z));
	Matrix position = Matrix::CreateTranslation(_localPosition);

	_localMat = scale * rotation * position;
	if (HasParent())
	{
		_worldMat = _localMat * _parent->GetWorldMatrix();
	}
	else
	{
		_worldMat = _localMat;
	}

	Quaternion quat;
	_worldMat.Decompose(_worldScale, quat, _worldPosition); // world matrix로 부터 s r t를 분리 (r은 quaternion으로 변환)
	_worldRotation = ToEulerAngles(quat);


	for (const shared_ptr<Transform>& child : _children)
		child->UpdateTransform();

	_transformBuffer = make_shared<Buffer>();
	TransformBuffer _transformBufferData;

	Matrix inverseWorld = _worldMat.Invert();
	_transformBufferData.worldMatrix = _worldMat;
	_transformBufferData.inverseTransposeWorldMatrix = inverseWorld.Transpose();
	
	_transformBuffer->CreateConstantBuffer<TransformBuffer>();
	_transformBuffer->CopyData(_transformBufferData);
}

bool Transform::HasParent()
{
	if (_parent != nullptr)
		return true;
	return false;
}

void Transform::SetPosition(const Vec3& position)
{
	if (HasParent())
	{
		Matrix inverseWorldMatrix = _parent->GetWorldMatrix().Invert();
		Vec3 convertedPosition = Vec3::Transform(position, inverseWorldMatrix);
		_localPosition = convertedPosition;
	}
	else
	{
		_localPosition = position;
	}
	UpdateTransform();
}

void Transform::SetRotation(const Vec3& rotation)
{
	if (HasParent())
	{
		Matrix inverseWorldMatrix = _parent->GetWorldMatrix().Invert();
		Vec3 convertedRotation = Vec3::TransformNormal(rotation, inverseWorldMatrix);
		_localRotation = convertedRotation;
	}
	else
	{
		_localRotation = rotation;
	}
	UpdateTransform();
}

void Transform::SetScale(const Vec3& scale)
{
	if (HasParent())
	{
		Matrix inverseWorldMatrix = _parent->GetWorldMatrix().Invert();
		Vec3 convertedScale = Vec3::TransformNormal(scale, inverseWorldMatrix);
		_localScale = convertedScale;
	}
	else
	{
		_localScale = scale;
	}
}

