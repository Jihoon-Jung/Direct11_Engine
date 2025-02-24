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
	// ���� ī�޶� ��ġ ��������
	Vec3 cameraPos = SCENE.GetActiveScene()->GetMainCamera()->transform()->GetWorldPosition();

	const float ROTATION_SPEED = 30.0f;  // �ʴ� ȸ�� ����
	float deltaAngle = ROTATION_SPEED * TIME.GetDeltaTime();

	// �ʱ� �Ÿ� ���� (���� �ڵ忡�� ����ϴ� �Ÿ�)
	Vec3 initialOffset = Vec3(-30, 39, 34);  // ī�޶�κ����� ����� �Ÿ�
	float radius = initialOffset.Length();    // ���� ������

	// ���� ��ġ ���
	float x = radius * cos(XMConvertToRadians(30.0f));
	float z = radius * sin(XMConvertToRadians(30.0f));

	// y���� ������ ���� ���� (���� ���� ���)
	float y = initialOffset.y;

	// ī�޶� ��ġ�� �߽����� �� ���ο� ���� ��ġ
	Vec3 newLightPos = cameraPos + Vec3(x, y, z);

	// ���� ��ġ ����
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
