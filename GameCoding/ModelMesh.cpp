#include "pch.h"
#include "ModelMesh.h"

void ModelMesh::CreateBuffers()
{
	buffer = make_shared<Buffer>();
	buffer->CreateBuffer(BufferType::VERTEX_BUFFER, geometry->GetVertices());
	buffer->CreateBuffer(BufferType::INDEX_BUFFER, geometry->GetIndices());
}
