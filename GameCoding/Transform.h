#pragma once
#include "Buffer.h"

class Transform :public Component
{
	using Super = Component;

public:
	Transform();
	~Transform();

	virtual void Start() override;
	virtual void Update() override;
	
	void UpdateTransform();
	bool HasParent();

	Matrix GetWorldMatrix() { return _worldMat; }
	Vec3 GetWorldPosition() { return _worldPosition; }
	Vec3 GetWorldRotation() { return _worldRotation; }
	Vec3 GetWorldScale() { return _worldScale; }

	Vec3 GetLocalPosition() { return _localPosition; }
	Vec3 GetLocalRotation() { 
		//Vec3 rotation = ToEulerAngles(_qtLocalRotation);
		//return rotation;
		Vec3 rotation = ToEulerAngles(_qtLocalRotation);
		// ������ ��(degree)�� ��ȯ
		return Vec3(
			XMConvertToDegrees(rotation.x),
			XMConvertToDegrees(rotation.y),
			XMConvertToDegrees(rotation.z)
		);
	}
	Vec3 GetLocalScale() { return _localScale; }
	Vec3 GetRevolutionRotation() { return _revolutionRotation; }
	Vec3 ToEulerAngles(Quaternion q);

	void SetLocalPosition(Vec3 position) { _localPosition = position; UpdateTransform();}
	void SetLocalRotation(Vec3 rotation) { 

		float pitch = XMConvertToRadians(rotation.x);
		float yaw = XMConvertToRadians(rotation.y);
		float roll = XMConvertToRadians(rotation.z);

		_qtLocalRotation = Quaternion::CreateFromYawPitchRoll(yaw, pitch, roll);
		UpdateTransform();
	}
	void SetLocalScale(Vec3 scale) { _localScale = scale; UpdateTransform();}

	// ���� �������� ������ �����ϴ� �Լ� �߰�
	void RotateAround(const Vec3& center, const Vec3& axis, float angle);

	void SetRevolutionRotation(Vec3 rotation)
	{
		float angle = rotation.y;  // Y�� ȸ���� ����ϴ� ���
		RotateAround(_revolutionInfo.center, Vec3::Up, angle);
	}

	void SetRevolutionCenter(Vec3 center)
	{
		_revolutionInfo.center = center;
	}
	void SetPosition(const Vec3& position);
	void SetRotation(const Vec3& rotation);
	void SetQTRotation(const Quaternion& rotation);
	void SetScale(const Vec3& scale);

	Quaternion GetQTRotation() { return _qtLocalRotation; }
	Vec3 GetLook() { return Vec3::TransformNormal(Vec3::Backward,_worldMat); }
	Vec3 GetUp() { return Vec3::TransformNormal(Vec3::Up,_worldMat); }
	Vec3 GetRight() { return Vec3::TransformNormal(Vec3::Right,_worldMat); }

	void SetParent(shared_ptr<Transform> parent) { _parent = parent; }
	void AddChild(shared_ptr<Transform> child) { _children.push_back(child); }

	shared_ptr<Transform>& GetParent() { return _parent; }
	shared_ptr<Buffer> GetTransformBuffer() { return _transformBuffer; }

	//test
	int ChildCount() { return _children.size(); }
	int ParentCount() { return _parent == nullptr ? 0 : 1; }

private:
	Vec3 _localPosition = { 0, 0, 0 };
	Quaternion _qtLocalRotation = Quaternion::Identity;
	Vec3 _localScale = { 1,1,1 };

	// ���� ȸ�� ����
	Vec3 _revolutionRotation = { 0, 0, 0 };
	// ���� ȸ�� �߽�
	Vec3 _revolutionCenter = { 0, 0, 0 };

	Matrix _localMat = Matrix::Identity;
	Matrix _worldMat = Matrix::Identity;

	Vec3 _worldPosition = { 0, 0, 0 };
	Vec3 _worldRotation = { 0, 0, 0 };
	Vec3 _worldScale = { 1, 1, 1 };
	
	shared_ptr<Transform> _parent;
	vector<shared_ptr<Transform>> _children;
	shared_ptr<Buffer> _transformBuffer;

	// ���� RevolutionInfo �����ϰ� ���ο� ������ ����
	struct RevolutionInfo
	{
		Vec3 center = Vec3::Zero;
		Vec3 axis = Vec3::Up;
		float angle = 0.0f;
		bool isActive = false;
	};
	RevolutionInfo _revolutionInfo;
};

