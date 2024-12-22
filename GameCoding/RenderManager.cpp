#include "pch.h"
#include "RenderManager.h"
#include <cmath> 

#include "ScreenShot.h"
#include <thread> // std::this_thread::sleep_for
#include <chrono> // std::chrono::seconds
void RenderManager::Init()
{
	_pipeline = make_shared<Pipeline>();
	vector<shared_ptr<GameObject>> gameObjects = SCENE.GetActiveScene()->GetGameObjects();
	for (shared_ptr<GameObject>& gameObject : gameObjects)
	{
		shared_ptr<MeshRenderer> meshRenderer = gameObject->GetComponent<MeshRenderer>();
		if (meshRenderer != nullptr)
		{
			if (gameObject->GetName() == L"Terrain_obj")
			{
				_terrainObject = gameObject;
				if (_terrainObject != nullptr && gameObject->GetName() != L"Terrain_obj" && gameObject->GetName() != L"skyBox")
				{
					Vec3 position = gameObject->transform()->GetLocalPosition();
					float height = _terrainObject->GetComponent<MeshRenderer>()->GetMesh()->GetHeight(position.x, position.z);
					if (height > position.y)
						gameObject->transform()->SetLocalPosition(Vec3(position.x, height, position.z));
				}
			}
		}
		if (gameObject->GetName() == L"MainLight")
		{
			_lightObject = gameObject;
		}

	}
}

void RenderManager::Update()
{
	const float ROTATION_SPEED = 30.0f;  // 초당 회전 각도
	float deltaAngle = ROTATION_SPEED * TIME.GetDeltaTime();

	// 공전 중심점과 회전 각도 설정
	_lightObject->transform()->SetRevolutionCenter(GP.centerPos);
	_lightObject->transform()->RotateAround(
		GP.centerPos,         // 공전 중심
		Vec3::Up,            // 회전 축 (Y축)
		deltaAngle           // 이번 프레임의 회전 각도
	);

	//// 자전
	//Vec3 localRotation(0.0f, deltaAngle, 0.0f);
	//_lightObject->transform()->SetLocalRotation(localRotation);

	

	ClearRenderObject();

	GP.SetRenderTarget();
	GP.SetViewport();

	

	GetRenderableObject();
	Render();

	/*GP.SwapChain();*/
}

void RenderManager::GetRenderableObject()
{
	vector<shared_ptr<GameObject>> gameObjects = SCENE.GetActiveScene()->GetGameObjects();
	for (shared_ptr<GameObject>& gameObject : gameObjects)
	{
		shared_ptr<MeshRenderer> meshRenderer = gameObject->GetComponent<MeshRenderer>();
		shared_ptr<ParticleSystem> particleSystem = gameObject->GetComponent<ParticleSystem>();
		shared_ptr<UIImage> uiImage = gameObject->GetComponent<UIImage>();
		if (meshRenderer != nullptr)
		{
			if (uiImage != nullptr)
				_UIObjects.push_back(gameObject);
			else if (!meshRenderer->CheckUseEnvironmentMap())
				_renderObjects.push_back(gameObject);
			else
				_envMappedObjects.push_back(gameObject);
			
		}


		if (gameObject->GetComponent<Billboard>() != nullptr)
			_billboardObjs.push_back(gameObject);
		
		//if (gameObject->GetName() == L"Terrain_obj")
		//{
		//	_terrainObject = gameObject;
		//	/*shared_ptr<GameObject> cameraObject;
		//	cameraObject = SCENE.GetActiveScene()->Find(L"MainCamera");
		//	Vec3 position = cameraObject->transform()->GetLocalPosition();
		//	float height = _terrainObject->GetComponent<MeshRenderer>()->GetMesh()->GetHeight(position.x, position.z);
		//	cameraObject->transform()->SetLocalPosition(Vec3(position.x, height + 0.2f, position.z));*/
		//}
		/*if (gameObject->GetComponent<ParticleSystem>() != nullptr)
			gameObject->GetComponent<ParticleSystem>()->Update(TIME.GetDeltaTime(), TIME.GetTotalTime());*/
	}
}

void RenderManager::DrawRenderableObject(bool isEnv)
{
	for (const shared_ptr<GameObject>& gameObject : _renderObjects)
	{
		
		if (_drawShadowMapFlag && gameObject->GetName() == L"skyBox")
			continue;
		if (_drawShadowMapFlag && gameObject->GetName() == L"MainLight")
			continue;

		shared_ptr<MeshRenderer> meshRenderer = gameObject->GetComponent<MeshRenderer>();
		shared_ptr<Model> model = meshRenderer->GetModel();


		if (meshRenderer->GetRenderPasses().size() > 0)
		{
			int count = meshRenderer->GetRenderPasses().size();
			for (int i = 0; i < meshRenderer->GetRenderPasses().size(); i++)
			{
				shared_ptr<RenderPass> renderPass = meshRenderer->GetRenderPasses()[i];
				renderPass->Render(isEnv);
				if (i + 1 < meshRenderer->GetRenderPasses().size())
				{
					shared_ptr<RenderPass> nextRenderPass = meshRenderer->GetRenderPasses()[i + 1];
					nextRenderPass->SetInputSRV(renderPass->GetOutputSRV());
				}
			}
		}
	}
}

void RenderManager::DrawUIObject(ComPtr<ID3D11ShaderResourceView> srv)
{
	for (const shared_ptr<GameObject>& gameObject : _UIObjects)
	{
		shared_ptr<MeshRenderer> meshRenderer = gameObject->GetComponent<MeshRenderer>();
		shared_ptr<Model> model = meshRenderer->GetModel();

		if (meshRenderer->GetRenderPasses().size() > 0)
		{
			int count = meshRenderer->GetRenderPasses().size();
			for (int i = 0; i < meshRenderer->GetRenderPasses().size(); i++)
			{
				shared_ptr<RenderPass> renderPass = meshRenderer->GetRenderPasses()[i];
				renderPass->SetInputSRV(srv);
				renderPass->Render(false);
				if (i + 1 < meshRenderer->GetRenderPasses().size())
				{
					shared_ptr<RenderPass> nextRenderPass = meshRenderer->GetRenderPasses()[i + 1];
					nextRenderPass->SetInputSRV(renderPass->GetOutputSRV());
				}
			}
		}
	}
}

void RenderManager::RenderEnvironmentMappedObjects(shared_ptr<GameObject> gameObject, shared_ptr<Texture> envTexture)
{
	shared_ptr<MeshRenderer> meshRenderer = gameObject->GetComponent<MeshRenderer>();
	shared_ptr<Model> model = meshRenderer->GetModel();

	meshRenderer->GetRenderPasses()[0]->SetTexture(envTexture);

	if (model == nullptr)
	{
		if (meshRenderer->GetRenderPasses().size() > 0)
		{
			int count = meshRenderer->GetRenderPasses().size();
			for (int i = 0; i < meshRenderer->GetRenderPasses().size(); i++)
			{
				shared_ptr<RenderPass> renderPass = meshRenderer->GetRenderPasses()[i];
				renderPass->Render(true);
				if (i + 1 < meshRenderer->GetRenderPasses().size())
				{
					shared_ptr<RenderPass> nextRenderPass = meshRenderer->GetRenderPasses()[i + 1];
					nextRenderPass->SetInputSRV(renderPass->GetOutputSRV());
				}
			}
		}
	}
}

void RenderManager::Render()
{
	if (_filterType == FilterType::GAUSSIAN_BLUR)
		GP.SetOffscreenRenderTarget();

	if (_filterType == FilterType::SHADOW_MAP)
	{
		GP.SetShadowMapRenderTarget();
		TIME.SetPause(true);

		_drawShadowMapFlag = true;
		
		RenderAllGameObject();
		/*std::string filename = "shadowMap2" + std::to_string(GP.GetViewWidth()) + "x" + std::to_string(GP.GetViewHeight()) + ".dump";
		ScreenShot sc;
		sc.SaveShaderResourceViewToFile(GP.GetShadowMapSRV().Get(), filename, GP.GetViewWidth(), GP.GetViewHeight());*/
		_drawShadowMapFlag = false;
		GP.RestoreRenderTarget();
	}
	TIME.SetPause(false);

	RenderAllGameObject();

	//DrawUIObject(GP.GetShadowMapSRV());

	if (_filterType == FilterType::GAUSSIAN_BLUR)
		GP.RenderQuad();
	
}

void RenderManager::RenderAllGameObject()
{
	if (!_drawShadowMapFlag && _billboardObjs.size() > 0)
	{
		for (const shared_ptr<GameObject>& gameObject : _billboardObjs)
		{
			gameObject->GetComponent<Billboard>()->DrawBillboard();
		}
	}

	DrawRenderableObject(false);

	if (_drawShadowMapFlag)
	{
		for (const shared_ptr<GameObject>& gameObject : _envMappedObjects)
		{
			shared_ptr<Texture> envTexture = make_shared<Texture>();
			RenderEnvironmentMappedObjects(gameObject, envTexture);
		}
	}
	else
	{
		TIME.SetPause(true);
		float deltaTime = TIME.GetDeltaTime();
		for (const shared_ptr<GameObject>& gameObject : _envMappedObjects)
		{
			shared_ptr<Material> material = make_shared<Material>();
			material->CreateEnvironmentMapTexture(gameObject);
			shared_ptr<Texture> envTexture = make_shared<Texture>();
			envTexture->SetShaderResourceView(material->GetCubeMapSRV());

			RenderEnvironmentMappedObjects(gameObject, envTexture);
		}

		TIME.SetPause(false);
	}
	
}

void RenderManager::ClearRenderObject()
{
	_renderObjects.clear();
	_envMappedObjects.clear();
	_billboardObjs.clear();
}
