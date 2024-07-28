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

	VertexBillboard_GeometryShader vb;
	vb.position = position;
	vb.scale = scale;
	_vertices.push_back(vb);

	_buffer = make_shared<Buffer>();
	_buffer->CreateBuffer(BufferType::VERTEX_BUFFER, _vertices);
}

void Billboard::DrawBillboard()
{
	uint32 stride = _buffer->GetStride();
	uint32 offset = 0;

	shared_ptr<Texture> texture = RESOURCE.GetResource<Texture>(L"Grass");
	shared_ptr<Shader> shader = RESOURCE.GetResource<Shader>(L"Billboard_Shader");
	shared_ptr<RasterizerState> rasterizerState = make_shared<RasterizerState>();
	shared_ptr<DepthStencilState> depthStencilState = make_shared<DepthStencilState>();
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

	// GeometryShader
	DEVICECONTEXT->GSSetShader(shader->GetGeometryShader().Get(), nullptr, 0);

	// Rasterizer
	DEVICECONTEXT->RSSetState(rasterizerState->GetRasterizerState().Get());

	// PixelShader
	DEVICECONTEXT->PSSetShader(shader->GetPixelShader().Get(), nullptr, 0);


	shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"texture0", 1, texture->GetShaderResourceView());

	DEVICECONTEXT->PSSetSamplers(0, 1, samplerState->GetSamplerState().GetAddressOf());


	// OutputMerger
	DEVICECONTEXT->OMSetBlendState(blendState->GetBlendState().Get(), nullptr, 0xFFFFFFFF);

	depthStencilState->SetDepthStencilState(DSState::NORMAL);
	DEVICECONTEXT->OMSetDepthStencilState(depthStencilState->GetDepthStecilState().Get(), 1);

	DEVICECONTEXT->Draw(_vertices.size(), 0);
}

