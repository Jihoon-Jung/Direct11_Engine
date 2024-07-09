#pragma once
#include "BaseCollider.h"

class SphereCollider : public BaseCollider
{
public:
	SphereCollider();
	virtual ~SphereCollider();

	virtual void Update() override;
	virtual bool Intersects(Ray& ray, OUT float& distance) override;
	virtual bool Intersects(shared_ptr<BaseCollider>& other) override;

	void SetRadius(float radius) { _radius = radius; }
	BoundingSphere& GetBoundingSphere() { return _boundingSphere; }
	
private:
	float _radius = 1.0f;
	BoundingSphere _boundingSphere;
};

