#include "pch.h"
#include "Billboard.h"
#include "MeshRenderer.h"

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

	/*VertexBillboard_GeometryShader vb;
	vb.position = position;
	_vertices.push_back(vb);*/

	float gap = 1.2f;
	float height = 0.5f;
	// first line
	VertexBillboard_GeometryShader vb1;
	vb1.position = position + Vec3(-gap, height, gap);
	_vertices.push_back(vb1);

	VertexBillboard_GeometryShader vb2;
	vb2.position = position + Vec3(0, height, gap);
	_vertices.push_back(vb2);

	VertexBillboard_GeometryShader vb3;
	vb3.position = position + Vec3(gap, height, gap);
	_vertices.push_back(vb3);

	// second line
	VertexBillboard_GeometryShader vb4;
	vb4.position = position + Vec3(-gap, height, 0);
	_vertices.push_back(vb4);

	VertexBillboard_GeometryShader vb5;
	vb5.position = position + Vec3(0, height, 0);
	_vertices.push_back(vb5);

	VertexBillboard_GeometryShader vb6;
	vb6.position = position + Vec3(gap, height, 0);
	_vertices.push_back(vb6);

	// thrid line
	VertexBillboard_GeometryShader vb7;
	vb7.position = position + Vec3(-gap, height, -gap);
	_vertices.push_back(vb7);

	VertexBillboard_GeometryShader vb8;
	vb8.position = position + Vec3(0, height, -gap);
	_vertices.push_back(vb8);

	VertexBillboard_GeometryShader vb9;
	vb9.position = position + Vec3(gap, height, -gap);
	_vertices.push_back(vb9);

	_buffer = make_shared<Buffer>();
	_buffer->CreateBuffer(BufferType::VERTEX_BUFFER, _vertices);
}

void Billboard::DrawBillboard()
{
	uint32 stride = _buffer->GetStride();
	uint32 offset = 0;

	shared_ptr<Texture> texture = RESOURCE.GetResource<Texture>(L"tree");
	shared_ptr<Shader> shader = RESOURCE.GetResource<Shader>(L"Billboard_Shader");

	shared_ptr<RasterizerState> rasterizerState = make_shared<RasterizerState>();
	RasterizerStateInfo states;
	states.fillMode = D3D11_FILL_SOLID;
	states.cullMode = D3D11_CULL_BACK;
	states.frontCounterClockwise = false;
	rasterizerState->CreateRasterizerState(states);

	shared_ptr<DepthStencilState> depthStencilState = make_shared<DepthStencilState>();
	depthStencilState->SetDepthStencilState(DSState::NORMAL);

	shared_ptr<BlendState> blendState = make_shared<BlendState>();
	blendState->CreateBlendState();

	shared_ptr<SamplerState> samplerState = make_shared<SamplerState>();
	samplerState->CreateSamplerState();

	shared_ptr<GameObject> cameraObject = SCENE.GetActiveScene()->GetMainCamera();

	CameraPos cameraPos;
	cameraPos.cameraPosition = SCENE.GetActiveScene()->GetCameraPos();
	shared_ptr<Buffer> cpBuffer = make_shared<Buffer>();
	cpBuffer->CreateConstantBuffer<CameraPos>();
	cpBuffer->CopyData(cameraPos);

	shader->PushConstantBufferToShader(ShaderType::GEOMETRY_SHADER, L"CameraPos", 1, cpBuffer);
	shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"TransformBuffer", 1, GetGameObject()->GetTransformBuffer());
	if (RENDER.GetShadowMapFlag())
		shader->PushConstantBufferToShader(ShaderType::GEOMETRY_SHADER, L"CameraBuffer", 1, cameraObject->GetShadowCameraBuffer());
	else
		shader->PushConstantBufferToShader(ShaderType::GEOMETRY_SHADER, L"CameraBuffer", 1, cameraObject->GetCameraBuffer());


	// inputAssembler
	DEVICECONTEXT->IASetVertexBuffers(0, 1, _buffer->GetVertexBuffer().GetAddressOf(), &stride, &offset);

	DEVICECONTEXT->IASetInputLayout(shader->GetInputLayout()->GetInputLayout().Get());
	DEVICECONTEXT->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

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

	
	DEVICECONTEXT->OMSetDepthStencilState(depthStencilState->GetDepthStecilState().Get(), 1);

	DEVICECONTEXT->Draw(_vertices.size(), 0);

	DEVICECONTEXT->GSSetShader(nullptr, nullptr, 0);

}

