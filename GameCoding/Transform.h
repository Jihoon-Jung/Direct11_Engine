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
	Vec3 GetLocalRotation() { return _localRotation; }
	Vec3 GetLocalScale() { return _localScale; }

	void SetLocalPosition(Vec3 position) { _localPosition = position; }
	void SetLocalRotation(Vec3 rotation) { _localRotation = rotation; }
	void SetLocalScale(Vec3 scale) { _localScale = scale; }

	void SetPosition(const Vec3& position);
	void SetRotation(const Vec3& rotation);
	void SetScale(const Vec3& scale);

	Vec3 GetLook() { return Vec3::TransformNormal(Vec3::Backward,_worldMat); }
	Vec3 GetUp() { return Vec3::TransformNormal(Vec3::Up,_worldMat); }
	Vec3 GetRight() { return Vec3::TransformNormal(Vec3::Right,_worldMat); }

	void SetParent(shared_ptr<Transform> parent) { _parent = parent; }
	void AddChild(shared_ptr<Transform> child) { _children.push_back(child); }

	shared_ptr<Buffer> GetTransformBuffer() { return _transformBuffer; }

	//test
	int ChildCount() { return _children.size(); }
	int ParentCount() { return _parent == nullptr ? 0 : 1; }
private:
	Vec3 _localPosition = { 0, 0, 0 };
	Vec3 _localRotation = { 0, 0, 0 };
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

