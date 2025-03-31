#pragma once
#include "Buffer.h"

class Transform :public Component, public std::enable_shared_from_this<Transform>
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
	Matrix GetLocalMatrix() { return _localMat; }
	Vec3 GetWorldPosition() { return _worldPosition; }

	Quaternion GetWorldRotation(){
		if (HasParent()) {
			return _parent->GetWorldRotation() * _qtLocalRotation;
		}
		return _qtLocalRotation;
	}
	static Quaternion InverseQuaternion(const Quaternion& q)
	{
		Quaternion result;
		XMStoreFloat4(&result, XMQuaternionInverse(XMLoadFloat4(&q)));
		return result;
	}
	Vec3 GetWorldScale() { return _worldScale; }

	Vec3 GetLocalPosition() { return _localPosition; }

	Vec3 GetLocalRotation() { 
		Vec3 rotation = ToEulerAngles(_qtLocalRotation);
		// 라디안을 도(degree)로 변환
		return Vec3(
			XMConvertToDegrees(rotation.x),
			XMConvertToDegrees(rotation.y),
			XMConvertToDegrees(rotation.z)
		);
	}

	Vec3 GetLocalScale() { return _localScale; }
	Vec3 ToEulerAngles(Quaternion q);

	void SetLocalPosition(Vec3 position) { 
		_localPosition = position; UpdateTransform();
	}
	void SetLocalRotation(Vec3 rotation) { 

		float pitch = XMConvertToRadians(rotation.x);
		float yaw = XMConvertToRadians(rotation.y);
		float roll = XMConvertToRadians(rotation.z);

		_qtLocalRotation = Quaternion::CreateFromYawPitchRoll(yaw, pitch, roll);
		UpdateTransform();
	}
	void SetLocalScale(Vec3 scale) { _localScale = scale; UpdateTransform();}

	void RotateAround(const Vec3& center, const Vec3& axis, float angle);

	void SetPosition(const Vec3& position);
	void SetRotation(const Vec3& rotation);
	void SetQTLocaslRotation(const Quaternion& rotation);
	
	Quaternion GetTotalParentRotation();
	void SetScale(const Vec3& scale);

	Quaternion GetQTRotation() { return _qtLocalRotation; }
	Vec3 GetLook() { return Vec3::TransformNormal(Vec3::Backward,_worldMat); }
	Vec3 GetUp() { return Vec3::TransformNormal(Vec3::Up,_worldMat); }
	Vec3 GetRight() { return Vec3::TransformNormal(Vec3::Right,_worldMat); }

	void SetParent(shared_ptr<Transform> parent) { _parent = parent; }
	void AddChild(shared_ptr<Transform> child) {
		_children.push_back(child); 
		UpdateTransform();
	}

	shared_ptr<Transform>& GetParent() { return _parent; }
	void DetachFromParent()
	{
		if (_parent)
		{
			// 부모의 자식 Transform 목록에서 제거
			auto it = std::find(_parent->_children.begin(), _parent->_children.end(), shared_from_this());
			if (it != _parent->_children.end())
				_parent->_children.erase(it);

			// 월드 변환 유지하면서 로컬 변환 재계산
			_localPosition = GetWorldPosition();
			_qtLocalRotation = Quaternion::CreateFromRotationMatrix(GetWorldMatrix());
			_localScale = GetWorldScale();

			_parent = nullptr;
		}
		UpdateTransform();
	}

	shared_ptr<Buffer> GetTransformBuffer() { return _transformBuffer; }

	int ChildCount() { return _children.size(); }
	int ParentCount() { return _parent == nullptr ? 0 : 1; }

private:
	Vec3 _localPosition = { 0, 0, 0 };
	Quaternion _qtLocalRotation = Quaternion::Identity;
	Vec3 _localScale = { 1,1,1 };

	Matrix _localMat = Matrix::Identity;
	Matrix _worldMat = Matrix::Identity;

	Vec3 _worldPosition = { 0, 0, 0 };
	Vec3 _worldRotation = { 0, 0, 0 };
	Vec3 _worldScale = { 1, 1, 1 };
	
	shared_ptr<Transform> _parent;
	vector<shared_ptr<Transform>> _children;
	shared_ptr<Buffer> _transformBuffer;

};

