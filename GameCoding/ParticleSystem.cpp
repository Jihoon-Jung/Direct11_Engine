#include "pch.h"
#include "ParticleSystem.h"

ParticleSystem::ParticleSystem()
	: Super(ComponentType::Particle)
{
	_firstRun = true;

	ComPtr<ID3D11ShaderResourceView> texArraySRV = RESOURCE.GetResource<Texture>(L"Fire_Particle")->GetShaderResourceView();
	ComPtr<ID3D11ShaderResourceView> randomTexSRV = RESOURCE.GetResource<Texture>(L"Random_Texture")->GetShaderResourceView();
	uint32 maxParticles = 500;

	_maxParticles = maxParticles;

	_texArraySRV = texArraySRV;
	_randomTexSRV = randomTexSRV;

	BuildVB();

}

ParticleSystem::~ParticleSystem()
{
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



void ParticleSystem::Update()
{

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
