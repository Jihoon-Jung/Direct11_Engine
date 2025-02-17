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
	// 1. 스테이징 버퍼 생성
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
		// 에러 처리
		return;
	}

	// 2. 버퍼 복사
	DEVICECONTEXT->CopyResource(stagingBuffer, _streamOutVB.Get());

	// 3. 스테이징 버퍼 매핑 및 데이터 읽기
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	hr = DEVICECONTEXT->Map(stagingBuffer, 0, D3D11_MAP_READ, 0, &mappedResource);
	if (FAILED(hr))
	{
		// 에러 처리
		stagingBuffer->Release();
		return;
	}

	// 데이터에 접근
	VertexParticle* particles = reinterpret_cast<VertexParticle*>(mappedResource.pData);
	UINT vertexCount = bufferDesc.ByteWidth / sizeof(VertexParticle);

	char outputBuffer[512]; // 충분히 큰 버퍼 확보
	for (UINT i = 0; i < min(vertexCount, 10u); ++i)
	{
		// 파티클 데이터 포맷팅
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

	// 4. 매핑 해제 및 스테이징 버퍼 해제
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
	vbd.ByteWidth = sizeof(VertexParticle) * 1; // _initVB는 정점 1개짜리
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;
	vbd.MiscFlags = 0;
	vbd.StructureByteStride = 0;

	// The initial particle emitter has type 0 and age 0.  The rest
	// of the particle attributes do not apply to an emitter.
	VertexParticle p;
	ZeroMemory(&p, sizeof(VertexParticle));
	p.age = 0.0f;
	p.type = 0; // 첫번째 정점 Emitter Flag 설정

	D3D11_SUBRESOURCE_DATA vinitData;
	vinitData.pSysMem = &p;

	HRESULT hr = DEVICE->CreateBuffer(&vbd, &vinitData, _initVB.GetAddressOf());
	CHECK(hr);

	//
	// Create the ping-pong buffers for stream-out and drawing.
	//
	vbd.ByteWidth = sizeof(VertexParticle) * _maxParticles; // _drawVB는 최대 파티클 개수만큼(500개), 대신 비어있음(GPU에서 만들거임)
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER | D3D11_BIND_STREAM_OUTPUT; // 버퍼를 SO와 연계시키려고 설정하는거

	hr = DEVICE->CreateBuffer(&vbd, 0, _drawVB.GetAddressOf()); // 실제 입자들이 저장될 버퍼
	CHECK(hr);

	hr = DEVICE->CreateBuffer(&vbd, 0, _streamOutVB.GetAddressOf());
	CHECK(hr);
}
