#include "pch.h"
#include "Light.h"

Light::Light()
	:Super(ComponentType::Light)
{
}

Light::~Light()
{
}

void Light::Update()
{
	// 현재 카메라 위치 가져오기
	Vec3 cameraPos = SCENE.GetActiveScene()->GetMainCamera()->transform()->GetWorldPosition();

	const float ROTATION_SPEED = 30.0f;  // 초당 회전 각도
	float deltaAngle = ROTATION_SPEED * TIME.GetDeltaTime();

	// 초기 거리 설정 (기존 코드에서 사용하던 거리)
	Vec3 initialOffset = Vec3(-30, 39, 34);  // 카메라로부터의 상대적 거리
	float radius = initialOffset.Length();    // 공전 반지름

	// 공전 위치 계산
	float x = radius * cos(XMConvertToRadians(30.0f));
	float z = radius * sin(XMConvertToRadians(30.0f));

	// y값은 고정된 높이 유지 (기존 높이 사용)
	float y = initialOffset.y;

	// 카메라 위치를 중심으로 한 새로운 빛의 위치
	Vec3 newLightPos = cameraPos + Vec3(x, y, z);

	// 빛의 위치 설정
	GetTransform()->SetLocalPosition(newLightPos);

	LightDesc lightDesc;
	lightDesc.ambient = Vec4(0.3f, 0.3f, 0.3f, 1.0f);
	lightDesc.diffuse = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
	lightDesc.specular = Vec4(0.8f, 0.8f, 0.7f, 1.0f);
	SetLightDesc(lightDesc);
}

void Light::SetLightDesc(LightDesc lightDesc)
{
	_lightDesc = lightDesc;
	_lightBuffer = make_shared<Buffer>();
	_lightBuffer->CreateConstantBuffer<LightDesc>();
	_lightBuffer->CopyData(lightDesc);
}
