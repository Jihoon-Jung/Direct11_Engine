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
}
Vec3 Transform::ToEulerAngles(Quaternion q)
{
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
	// 1. ���� ��ȯ ���
    Matrix scale = Matrix::CreateScale(_localScale);
    Matrix rotation = Matrix::CreateFromQuaternion(_qtLocalRotation);
    Matrix localTranslation = Matrix::CreateTranslation(_localPosition);
    Matrix localTransform = scale * rotation * localTranslation;
    _localMat = localTransform;

    // 2. �⺻ ���� ��ȯ ���
    if (HasParent())
    {
        _worldMat = _localMat * _parent->GetWorldMatrix();
    }
    else
    {
        _worldMat = _localMat;
    }


    // 3. ���� ���� ��ȯ�� ����
    Quaternion worldQuat;
    _worldMat.Decompose(_worldScale, worldQuat, _worldPosition);
    _worldRotation = ToEulerAngles(worldQuat);

    // �ڽ� ������Ʈ
    for (const shared_ptr<Transform>& child : _children)
    {
        child->UpdateTransform();
    }

	// 4. ���̴��� ������ ��� ���� ������Ʈ
    _transformBuffer = make_shared<Buffer>();
    TransformBuffer _transformBufferData;

    // 5. ���� ����� ����İ� ��ġ ��� ���
    Matrix inverseWorld = _worldMat.Invert();
    _transformBufferData.worldMatrix = _worldMat;
    _transformBufferData.inverseTransposeWorldMatrix = inverseWorld.Transpose();

    // 6. ��� ���� ���� �� ������ ����
    _transformBuffer->CreateConstantBuffer<TransformBuffer>();
    _transformBuffer->CopyData(_transformBufferData);

    //SCENE.CheckCollision();
}


bool Transform::HasParent()
{
	if (_parent != nullptr)
		return true;
	return false;
}

void Transform::RotateAround(const Vec3& center, const Vec3& axis, float angle)
{
    // 1. ���� ���� ��ġ ����
    Vec3 worldPos = GetWorldPosition();

    // 2. �߽��� ���� ȸ�� ��� ����
    Matrix toOrigin = Matrix::CreateTranslation(-center);
    Matrix rotation = Matrix::CreateFromAxisAngle(axis, XMConvertToRadians(angle));
    Matrix fromOrigin = Matrix::CreateTranslation(center);

    // 3. ��ġ�� ȸ�� ����
    Matrix transform = toOrigin * rotation * fromOrigin;
    Vec3 newWorldPos = Vec3::Transform(worldPos, transform);

    // 4. ���ο� ��ġ�� ���� �������� ��ȯ
    if (HasParent())
    {
        Matrix parentInverse = _parent->GetWorldMatrix().Invert();
        _localPosition = Vec3::Transform(newWorldPos, parentInverse);
    }
    else
    {
        _localPosition = newWorldPos;  // �θ� ������ ���� ��ġ�� �� ���� ��ġ
    }

    // 5. Transform ������Ʈ
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

		// ��ȯ�� ���Ϸ� ������ ���ʹϾ����� ��ȯ
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
void Transform::SetQTLocaslRotation(const Quaternion& rotation)
{
	_qtLocalRotation = rotation;
	UpdateTransform();
}

Quaternion Transform::GetTotalParentRotation()
{
    if (!HasParent())
        return Quaternion::Identity;

    // ��� �θ��� ȸ���� ����
    Quaternion totalRotation = Quaternion::Identity;
    shared_ptr<Transform> current = _parent;

    while (current != nullptr)
    {
        totalRotation = current->GetQTRotation() * totalRotation;
        current = current->GetParent();
    }

    return totalRotation;
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

