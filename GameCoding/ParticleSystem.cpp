#include "pch.h"
#include "ParticleSystem.h"

ParticleSystem::ParticleSystem()
	: Super(ComponentType::Particle)
{
	_firstRun = true;
	_gameTime = 0.0f;
	_timeStep = 0.0f;
	_age = 0.0f;

	_eyePosW = XMFLOAT3(0.0f, 0.0f, 0.0f);
	_emitPosW = XMFLOAT3(0.0f, 1.0f, 120.0f);
	_emitDirW = XMFLOAT3(0.0f, 1.0f, 0.0f);
}

ParticleSystem::~ParticleSystem()
{
}

void ParticleSystem::Init(ComPtr<ID3D11ShaderResourceView> texArraySRV, ComPtr<ID3D11ShaderResourceView> randomTexSRV, uint32 maxParticles)
{
	_maxParticles = maxParticles;

	_texArraySRV = texArraySRV;
	_randomTexSRV = randomTexSRV;

	BuildVB();
}

#include <iostream>
using namespace std;
void ParticleSystem::ReadStreamOutData()
{
	// 1. ������¡ ���� ����
	ID3D11Buffer* stagingBuffer = nullptr;
	D3D11_BUFFER_DESC bufferDesc;
	_streamOutVB->GetDesc(&bufferDesc);

	bufferDesc.Usage = D3D11_USAGE_STAGING;
	bufferDesc.BindFlags = 0;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	bufferDesc.MiscFlags = 0;

	HRESULT hr = DEVICE->CreateBuffer(&bufferDesc, nullptr, &stagingBuffer);
	if (FAILED(hr))
	{
		// ���� ó��
		return;
	}

	// 2. ���� ����
	DEVICECONTEXT->CopyResource(stagingBuffer, _streamOutVB.Get());

	// 3. ������¡ ���� ���� �� ������ �б�
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	hr = DEVICECONTEXT->Map(stagingBuffer, 0, D3D11_MAP_READ, 0, &mappedResource);
	if (FAILED(hr))
	{
		// ���� ó��
		stagingBuffer->Release();
		return;
	}

	// �����Ϳ� ����
	VertexParticle* particles = reinterpret_cast<VertexParticle*>(mappedResource.pData);
	UINT vertexCount = bufferDesc.ByteWidth / sizeof(VertexParticle);

	char outputBuffer[512]; // ����� ū ���� Ȯ��
	for (UINT i = 0; i < min(vertexCount, 10u); ++i)
	{
		// ��ƼŬ ������ ������
		int bytesWritten = sprintf_s(
			outputBuffer,
			sizeof(outputBuffer),
			"Particle %u: Position(%.2f, %.2f, %.2f), Velocity(%.2f, %.2f, %.2f), Size(%.2f, %.2f), Age(%.2f), Type(%u)\n",
			i,
			particles[i].position.x,
			particles[i].position.y,
			particles[i].position.z,
			particles[i].velocity.x,
			particles[i].velocity.y,
			particles[i].velocity.z,
			particles[i].size.x,
			particles[i].size.y,
			particles[i].age,
			particles[i].type
		);

		if (bytesWritten > 0)
		{
			OutputDebugStringA(outputBuffer);
		}
	}

	// 4. ���� ���� �� ������¡ ���� ����
	DEVICECONTEXT->Unmap(stagingBuffer, 0);
	stagingBuffer->Release();
}

void ParticleSystem::Reset()
{
	_firstRun = true;
	_age = 0.0f;
}

void ParticleSystem::Update(float dt, float gameTime, bool isEnv)
{
	_gameTime = gameTime;
	_timeStep = dt;

	_age += dt;
	_isEnv = isEnv;
	Draw();
}

void ParticleSystem::Draw()
{
	shared_ptr<Shader> initParticleShader = RESOURCE.GetResource<Shader>(L"InitParticle_Shader");
	shared_ptr<Shader> renderParticleShader = RESOURCE.GetResource<Shader>(L"RenderParticle_Shader");

	Matrix viewMat;
	Matrix projMat;
	if (_isEnv)
	{
		viewMat = SCENE.GetActiveScene()->Find(L"MainCamera")->GetComponent<Camera>()->GetEnvViewMatrix();
		projMat = SCENE.GetActiveScene()->Find(L"MainCamera")->GetComponent<Camera>()->GetEnvProjectionMatrix();
	}
	else
	{
		viewMat = SCENE.GetActiveScene()->Find(L"MainCamera")->GetComponent<Camera>()->GetViewMatrix();
		projMat = SCENE.GetActiveScene()->Find(L"MainCamera")->GetComponent<Camera>()->GetProjectionMatrix();
	}
	
	Matrix viewProj = viewMat * projMat;

	ParticleBuffer pBuffer;
	pBuffer.gView = viewMat;
	pBuffer.gProj = projMat;
	pBuffer.gGameTime = _gameTime;
	pBuffer.gTimeStep = _timeStep;
	pBuffer.gEyePosW = SCENE.GetActiveScene()->FindWithComponent(ComponentType::Camera)->transform()->GetWorldPosition();
	pBuffer.gEmitPosW = _emitPosW;
	pBuffer.gEmitDirW = _emitDirW;

	shared_ptr<Buffer> particleBuffer = make_shared<Buffer>();
	particleBuffer->CreateConstantBuffer<ParticleBuffer>();
	particleBuffer->CopyData(pBuffer);

	static_assert(sizeof(ParticleBuffer) % 16 == 0, "Constant buffer size must be a multiple of 16 bytes.");

	initParticleShader->PushConstantBufferToShader(ShaderType::GEOMETRY_SHADER, L"ParticleBuffer", 1, particleBuffer);
	initParticleShader->PushShaderResourceToShader(ShaderType::GEOMETRY_SHADER, L"gRandomTex", 1, _randomTexSRV);

	shared_ptr<SamplerState> samplerState = make_shared<SamplerState>();
	samplerState->CreateSamplerState();

	// make particle vertex
	DEVICECONTEXT->IASetInputLayout(initParticleShader->GetInputLayout()->GetInputLayout().Get());
	DEVICECONTEXT->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	uint32 stride = sizeof(VertexParticle);
	uint32 offset = 0;

	if (_firstRun)
		DEVICECONTEXT->IASetVertexBuffers(0, 1, _initVB.GetAddressOf(), &stride, &offset);
	else
		DEVICECONTEXT->IASetVertexBuffers(0, 1, _drawVB.GetAddressOf(), &stride, &offset);

	// VertexShader
	DEVICECONTEXT->VSSetShader(initParticleShader->GetVertexShader().Get(), nullptr, 0);

	// GeometryShader
	DEVICECONTEXT->GSSetShader(initParticleShader->GetOutputStreamGeometryShader().Get(), nullptr, 0);

	DEVICECONTEXT->GSSetSamplers(0, 1, samplerState->GetSamplerState().GetAddressOf());
	// PixelShader (not use)
	DEVICECONTEXT->PSSetShader(nullptr, nullptr, 0);

	DEVICECONTEXT->SOSetTargets(1, _streamOutVB.GetAddressOf(), &offset);

	if (_firstRun)
	{
		DEVICECONTEXT->Draw(1, 0);
		_firstRun = false;
	}
	else
	{
		DEVICECONTEXT->DrawAuto();
	}

	// ��Ʈ�� �ƿ� Ÿ�� ����
	ID3D11Buffer* bufferArray[1] = { 0 };
	DEVICECONTEXT->SOSetTargets(1, bufferArray, &offset);

	// ���� ����
	std::swap(_drawVB, _streamOutVB);


	// Render Particle
	
	

	shared_ptr<RasterizerState> rasterizerState = make_shared<RasterizerState>();
	RasterizerStateInfo states;
	states.fillMode = D3D11_FILL_SOLID;
	states.cullMode = D3D11_CULL_BACK;
	states.frontCouterClockWise = false;
	rasterizerState->CreateRasterizerState(states);

	shared_ptr<DepthStencilState> depthStencilState = make_shared<DepthStencilState>();
	depthStencilState->SetDepthStencilState(DSState::CUSTOM3);

	shared_ptr<BlendState> blendState = make_shared<BlendState>();
	blendState->CreateAdditiveBlendState();

	

	// inputAssembler
	DEVICECONTEXT->IASetVertexBuffers(0, 1, _drawVB.GetAddressOf(), &stride, &offset);

	DEVICECONTEXT->IASetInputLayout(renderParticleShader->GetInputLayout()->GetInputLayout().Get());
	DEVICECONTEXT->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	// VertexShader
	DEVICECONTEXT->VSSetShader(renderParticleShader->GetVertexShader().Get(), nullptr, 0);

	// GeometryShader
	DEVICECONTEXT->GSSetShader(renderParticleShader->GetGeometryShader().Get(), nullptr, 0);

	// Rasterizer
	DEVICECONTEXT->RSSetState(rasterizerState->GetRasterizerState().Get());

	// PixelShader
	DEVICECONTEXT->PSSetShader(renderParticleShader->GetPixelShader().Get(), nullptr, 0);

	renderParticleShader->PushConstantBufferToShader(ShaderType::GEOMETRY_SHADER, L"ParticleBuffer", 1, particleBuffer);

	renderParticleShader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"gTexArray", 1, _texArraySRV);

	DEVICECONTEXT->PSSetSamplers(0, 1, samplerState->GetSamplerState().GetAddressOf());


	// OutputMerger
	DEVICECONTEXT->OMSetBlendState(blendState->GetBlendState().Get(), nullptr, 0xFFFFFFFF);


	DEVICECONTEXT->OMSetDepthStencilState(depthStencilState->GetDepthStecilState().Get(), 1);

	DEVICECONTEXT->DrawAuto();

	DEVICECONTEXT->GSSetShader(nullptr, nullptr, 0);
}

void ParticleSystem::BuildVB()
{
	//
	// Create the buffer to kick-off the particle system.
	//

	D3D11_BUFFER_DESC vbd;
	vbd.Usage = D3D11_USAGE_DEFAULT;
	vbd.ByteWidth = sizeof(VertexParticle) * 1; // _initVB�� ���� 1��¥��
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	// The initial particle emitter has type 0 and age 0.  The rest
	// of the particle attributes do not apply to an emitter.
	VertexParticle p;
	ZeroMemory(&p, sizeof(VertexParticle));
	p.age = 0.0f;
	p.type = 0; // ù��° ���� Emitter Flag ����

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &p;

	HRESULT hr = DEVICE->CreateBuffer(&vbd, &vinitData, _initVB.GetAddressOf());
	CHECK(hr);

	//
	// Create the ping-pong buffers for stream-out and drawing.
	//
	vbd.ByteWidth = sizeof(VertexParticle) * _maxParticles; // _drawVB�� �ִ� ��ƼŬ ������ŭ(500��), ��� �������(GPU���� �������)
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT; // ���۸� SO�� �����Ű���� �����ϴ°�

	hr = DEVICE->CreateBuffer(&vbd, 0, _drawVB.GetAddressOf()); // ���� ���ڵ��� ����� ����
	CHECK(hr);

	hr = DEVICE->CreateBuffer(&vbd, 0, _streamOutVB.GetAddressOf());
	CHECK(hr);
}
