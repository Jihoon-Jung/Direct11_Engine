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
Vec3 Transform::ToEulerAngles(Quaternion q)
{
	//Vec3 angles;

	//// roll (x-axis rotation)
	//double sinr_cosp = 2 * (q.w * q.x + q.y * q.z);
	//double cosr_cosp = 1 - 2 * (q.x * q.x + q.y * q.y);
	//angles.x = std::atan2(sinr_cosp, cosr_cosp);

	//// pitch (y-axis rotation)
	//double sinp = std::sqrt(1 + 2 * (q.w * q.y - q.x * q.z));
	//double cosp = std::sqrt(1 - 2 * (q.w * q.y - q.x * q.z));
	//angles.y = 2 * std::atan2(sinp, cosp) - 3.14159f / 2;

	//// yaw (z-axis rotation)
	//double siny_cosp = 2 * (q.w * q.z + q.x * q.y);
	//double cosy_cosp = 1 - 2 * (q.y * q.y + q.z * q.z);
	//angles.z = std::atan2(siny_cosp, cosy_cosp);

	//return angles;

    Vec3 angles;

    // pitch (x-axis rotation)
    float pitch = asin(2 * (q.w * q.x - q.y * q.z));

    // yaw (y-axis rotation)
    float yaw = atan2(2 * (q.w * q.y + q.x * q.z), 1 - 2 * (q.x * q.x + q.y * q.y));

    // roll (z-axis rotation)
    float roll = atan2(2 * (q.w * q.z + q.x * q.y), 1 - 2 * (q.x * q.x + q.z * q.z));

    angles.x = pitch;
    angles.y = yaw;
    angles.z = roll;

    return angles;
    
}

void Transform::UpdateTransform()
{
	// 1. 로컬 변환 계산
    Matrix scale = Matrix::CreateScale(_localScale);
    Matrix rotation = Matrix::CreateFromQuaternion(_qtLocalRotation);
    Matrix localTranslation = Matrix::CreateTranslation(_localPosition);
    Matrix localTransform = scale * rotation * localTranslation;
    _localMat = localTransform;

    // 2. 기본 월드 변환 계산
    if (HasParent())
    {
        _worldMat = _localMat * _parent->GetWorldMatrix();
    }
    else
    {
        _worldMat = _localMat;
    }

    // 3. 공전이 있다면 월드 공간에서 적용
    if (_revolutionInfo.isActive)
    {
        Matrix toOrigin = Matrix::CreateTranslation(-_revolutionInfo.center);
        Matrix rotation = Matrix::CreateFromAxisAngle(
            _revolutionInfo.axis,
            XMConvertToRadians(_revolutionInfo.angle)
        );
        Matrix fromOrigin = Matrix::CreateTranslation(_revolutionInfo.center);

        // 공전은 월드 공간에서 적용
        _worldMat = _worldMat * toOrigin * rotation * fromOrigin;
    }

    // 4. 최종 월드 변환값 추출
    Quaternion worldQuat;
    _worldMat.Decompose(_worldScale, worldQuat, _worldPosition);
    _worldRotation = ToEulerAngles(worldQuat);

    // 자식 업데이트
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

void Transform::RotateAround(const Vec3& center, const Vec3& axis, float angle)
{
    // 1. 현재 월드 위치 저장
    Vec3 worldPos = GetWorldPosition();

    // 2. 중심점 기준 회전 행렬 생성
    Matrix toOrigin = Matrix::CreateTranslation(-center);
    Matrix rotation = Matrix::CreateFromAxisAngle(axis, XMConvertToRadians(angle));
    Matrix fromOrigin = Matrix::CreateTranslation(center);

    // 3. 위치에 회전 적용
    Matrix transform = toOrigin * rotation * fromOrigin;
    Vec3 newWorldPos = Vec3::Transform(worldPos, transform);

    // 4. 새로운 위치를 로컬 공간으로 변환
    if (HasParent())
    {
        Matrix parentInverse = _parent->GetWorldMatrix().Invert();
        _localPosition = Vec3::Transform(newWorldPos, parentInverse);
    }
    else
    {
        _localPosition = newWorldPos;  // 부모가 없으면 월드 위치가 곧 로컬 위치
    }

    // 5. 회전도 적용
    Quaternion currentRotation = _qtLocalRotation;
    Quaternion additionalRotation = Quaternion::CreateFromAxisAngle(axis, XMConvertToRadians(angle));
    _qtLocalRotation = currentRotation * additionalRotation;

    // 6. Transform 업데이트
    UpdateTransform();
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

		// 변환된 오일러 각도를 쿼터니언으로 변환
		float pitch = XMConvertToRadians(convertedRotation.x);
		float yaw = XMConvertToRadians(convertedRotation.y);
		float roll = XMConvertToRadians(convertedRotation.z);

		_qtLocalRotation = Quaternion::CreateFromYawPitchRoll(yaw, pitch, roll);
	}
	else
	{
		float pitch = XMConvertToRadians(rotation.x);
		float yaw = XMConvertToRadians(rotation.y);
		float roll = XMConvertToRadians(rotation.z);

		_qtLocalRotation = Quaternion::CreateFromYawPitchRoll(yaw, pitch, roll);
	}
	UpdateTransform();
}
void Transform::SetQTRotation(const Quaternion& rotation)
{
	_qtLocalRotation = rotation;
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
    UpdateTransform();
}

