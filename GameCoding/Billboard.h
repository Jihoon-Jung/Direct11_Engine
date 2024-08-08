#pragma once
#include "Component.h"

class Billboard : public Component
{
	using Super = Component;

public:
	Billboard();
	virtual ~Billboard();

	void Update();
	void Add(Vec3 position, Vec2 scale);

	void SetMaterial(shared_ptr<Material> material) { _material = material; }
	void DrawBillboard();
private:
	vector<VertexBillboard_GeometryShader> _vertices;
	shared_ptr<Buffer> _buffer;


	shared_ptr<Material> _material;
};

