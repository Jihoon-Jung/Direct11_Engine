#pragma once
#include "Component.h"

#define MAX_BILLBOARD_COUNT 16

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
	vector<VertexBillboard> _vertices;
	vector<uint32> _indices;
	shared_ptr<Buffer> _buffer;

	int32 _drawCount = 0;
	int32 _prevCount = 0;

	shared_ptr<Material> _material;
};

