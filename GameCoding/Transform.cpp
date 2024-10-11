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
//void Transform::UpdateTransform()
//{
//	// 스케일 행렬 생성
//	Matrix scale = Matrix::CreateScale(_localScale);
//
//	// 쿼터니언을 사용한 회전 행렬 생성
//	Quaternion rotationQuat = Quaternion::CreateFromYawPitchRoll(
//		DirectX::XMConvertToRadians(_localRotation.y),  // Yaw
//		DirectX::XMConvertToRadians(_localRotation.x),  // Pitch
//		DirectX::XMConvertToRadians(_localRotation.z)   // Roll
//	);
//	Matrix rotation = Matrix::CreateFromQuaternion(rotationQuat);  // 쿼터니언을 회전 행렬로 변환
//
//	// 위치 행렬 생성
//	Matrix position = Matrix::CreateTranslation(_localPosition);
//
//	// 최종 변환 행렬
//	_localMat = scale * rotation * position;
//
//	// 부모가 있으면 월드 행렬을 부모의 월드 행렬과 곱함
//	if (HasParent())
//	{
//		_worldMat = _localMat * _parent->GetWorldMatrix();
//	}
//	else
//	{
//		_worldMat = _localMat;
//	}
//
//	// 월드 행렬에서 스케일, 회전(쿼터니언), 위치 추출
//	Quaternion worldQuat;
//	_worldMat.Decompose(_worldScale, worldQuat, _worldPosition);  // world matrix로부터 s, r, t 분리
//
//	// 월드 회전 각도를 오일러 각도로 변환 (필요한 경우)
//	_worldRotation = ToEulerAngles(worldQuat);  // 필요한 경우, 오일러 각으로 변환
//
//	// 자식들에 대해 재귀적으로 변환 적용
//	for (const shared_ptr<Transform>& child : _children)
//	{
//		child->UpdateTransform();
//	}
//
//	// 셰이더에 전달할 상수 버퍼 업데이트
//	_transformBuffer = make_shared<Buffer>();
//	TransformBuffer _transformBufferData;
//
//	// 월드 행렬의 역행렬과 전치 행렬 계산
//	Matrix inverseWorld = _worldMat.Invert();
//	_transformBufferData.worldMatrix = _worldMat;
//	_transformBufferData.inverseTransposeWorldMatrix = inverseWorld.Transpose();
//
//	// 상수 버퍼 생성 및 데이터 복사
//	_transformBuffer->CreateConstantBuffer<TransformBuffer>();
//	_transformBuffer->CopyData(_transformBufferData);
//}
void Transform::UpdateTransform()
{
	// 스케일 행렬 생성
	Matrix scale = Matrix::CreateScale(_localScale);

	// 자전 회전 행렬 생성
	Quaternion rotationQuat = Quaternion::CreateFromYawPitchRoll(
		XMConvertToRadians(_localRotation.y),
		XMConvertToRadians(_localRotation.x),
		XMConvertToRadians(_localRotation.z)
	);
	Matrix rotation = Matrix::CreateFromQuaternion(rotationQuat);

	// 공전 변환 계산
	Matrix translationToOrigin = Matrix::CreateTranslation(-_revolutionCenter);
	Quaternion revolutionQuat = Quaternion::CreateFromYawPitchRoll(
		XMConvertToRadians(_revolutionRotation.y),
		XMConvertToRadians(_revolutionRotation.x),
		XMConvertToRadians(_revolutionRotation.z)
	);
	Matrix revolutionRotation = Matrix::CreateFromQuaternion(revolutionQuat);
	Matrix translationBack = Matrix::CreateTranslation(_revolutionCenter);

	// 공전 변환 행렬 계산
	Matrix revolutionMatrix = translationToOrigin * revolutionRotation * translationBack;

	// 로컬 위치 이동 행렬
	Matrix translationWithLocalPosition = Matrix::CreateTranslation(_localPosition);

	// 전체 변환 행렬 계산
	_localMat = scale * rotation * translationWithLocalPosition * revolutionMatrix;

	// 부모 변환 적용
	if (HasParent())
	{
		_worldMat = _localMat * _parent->GetWorldMatrix();
	}
	else
	{
		_worldMat = _localMat;
	}

	// 월드 변환에서 스케일, 회전, 위치 추출
	Quaternion worldQuat;
	_worldMat.Decompose(_worldScale, worldQuat, _worldPosition);

	// 월드 회전 정보 저장
	_worldRotation = ToEulerAngles(worldQuat);

	// 자식 노드들 업데이트
	for (const shared_ptr<Transform>& child : _children)
	{
		child->UpdateTransform();
	}

    // 12. 셰이더에 전달할 상수 버퍼 업데이트
    _transformBuffer = make_shared<Buffer>();
    TransformBuffer _transformBufferData;

    // 월드 행렬의 역행렬과 전치 행렬 계산
    Matrix inverseWorld = _worldMat.Invert();
    _transformBufferData.worldMatrix = _worldMat;
    _transformBufferData.inverseTransposeWorldMatrix = inverseWorld.Transpose();

    // 상수 버퍼 생성 및 데이터 복사
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

