#include "pch.h"
#include "Billboard.h"

Billboard::Billboard()
	: Super(ComponentType::Billboard)
{
}

Billboard::~Billboard()
{
}

void Billboard::Update()
{

}

void Billboard::Add(Vec3 position, Vec2 scale)
{
	shared_ptr<GameObject> obj = GetGameObject();
	shared_ptr<Transform> transform = obj->transform();
	
	Matrix matWorld = GetGameObject()->transform()->GetWorldMatrix();

	for (int i = 0; i < 4; i++)
	{
		_vertices[_drawCount * 4 + i].position = Vec3::Transform(position, matWorld);
	}
	_vertices[_drawCount * 4 + 0].uv = Vec2(0, 1);
	_vertices[_drawCount * 4 + 1].uv = Vec2(0, 0);
	_vertices[_drawCount * 4 + 2].uv = Vec2(1, 1);
	_vertices[_drawCount * 4 + 3].uv = Vec2(1, 0);

	_vertices[_drawCount * 4 + 0].scale = scale;
	_vertices[_drawCount * 4 + 1].scale = scale;
	_vertices[_drawCount * 4 + 2].scale = scale;
	_vertices[_drawCount * 4 + 3].scale = scale;


	_drawCount++;
}

void Billboard::DrawBillboard()
{
	if (_drawCount != _prevCount)
	{
		_prevCount = _drawCount;

		D3D11_MAPPED_SUBRESOURCE subResource;
		ZeroMemory(&subResource, sizeof(subResource));
		HRESULT hr = DEVICECONTEXT->Map(_buffer->GetVertexBuffer().Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &subResource);
		CHECK(hr);
		{
			memcpy(subResource.pData, _vertices.data(), sizeof(VertexBillboard) * _vertices.size());
		}
		DEVICECONTEXT->Unmap(_buffer->GetVertexBuffer().Get(), 0);

	}

	uint32 stride = _buffer->GetStride();
	uint32 offset = 0;

	shared_ptr<Texture> texture = RESOURCE.GetResource<Texture>(L"Grass");
	shared_ptr<Shader> shader = RESOURCE.GetResource<Shader>(L"Billboard_Shader");
	shared_ptr<RasterizerState> rasterizerState = make_shared<RasterizerState>();
	shared_ptr<BlendState> blendState = make_shared<BlendState>();
	shared_ptr<SamplerState> samplerState = make_shared<SamplerState>();

	shared_ptr<GameObject> cameraObject = SCENE.GetActiveScene()->Find(L"MainCamera");

	CameraPos cameraPos;
	cameraPos.cameraPosition = SCENE.GetActiveScene()->GetCameraPos();
	shared_ptr<Buffer> cpBuffer = make_shared<Buffer>();
	cpBuffer->CreateConstantBuffer<CameraPos>();
	cpBuffer->CopyData(cameraPos);

	shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"CameraPos", 1, cpBuffer);
	shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"TransformBuffer", 1, GetGameObject()->GetTransformBuffer());
	shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"CameraBuffer", 1, cameraObject->GetCameraBuffer());


	// inputAssembler
	DEVICECONTEXT->IASetVertexBuffers(0, 1, _buffer->GetVertexBuffer().GetAddressOf(), &stride, &offset);
	DEVICECONTEXT->IASetIndexBuffer(_buffer->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

	DEVICECONTEXT->IASetInputLayout(shader->GetInputLayout()->GetInputLayout().Get());
	DEVICECONTEXT->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// VertexShader
	DEVICECONTEXT->VSSetShader(shader->GetVertexShader().Get(), nullptr, 0);

	// Rasterizer
	DEVICECONTEXT->RSSetState(rasterizerState->GetRasterizerState().Get());

	// PixelShader
	DEVICECONTEXT->PSSetShader(shader->GetPixelShader().Get(), nullptr, 0);


	shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"texture0", 1, texture->GetShaderResourceView());

	DEVICECONTEXT->PSSetSamplers(0, 1, samplerState->GetSamplerState().GetAddressOf());

	// OutputMerger
	DEVICECONTEXT->OMSetBlendState(blendState->GetBlendState().Get(), nullptr, 0xFFFFFFFF);

	DEVICECONTEXT->DrawIndexed(_drawCount * 6, 0, 0);
}

void Billboard::SetBillboardBuffer(int count)
{
	_billboardCount += count;

	int32 vertexCount = _billboardCount * 4;
	int32 indexCount = _billboardCount * 6;

	_vertices.resize(vertexCount);

	_indices.resize(indexCount);

	for (int32 i = 0; i < _billboardCount; i++)
	{
		_indices[i * 6 + 0] = i * 4 + 0;
		_indices[i * 6 + 1] = i * 4 + 1;
		_indices[i * 6 + 2] = i * 4 + 2;
		_indices[i * 6 + 3] = i * 4 + 2;
		_indices[i * 6 + 4] = i * 4 + 1;
		_indices[i * 6 + 5] = i * 4 + 3;
	}

	_buffer = make_shared<Buffer>();
	_buffer->CreateBuffer(BufferType::VERTEX_BUFFER, _vertices, true, false);
	_buffer->CreateBuffer(BufferType::INDEX_BUFFER, _indices);
}
