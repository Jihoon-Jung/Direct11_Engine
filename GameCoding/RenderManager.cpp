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

	GetRenderableObject();
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

	
	GP.SetRenderTarget();
	GP.SetViewport();

	
	Render();

}

void RenderManager::GetRenderableObject()
{
	ClearRenderObject();

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

	}

	GetDefaultRenderObjectsForInstancing(_renderObjects);
	GetStaticMeshObjectsForInstancing(_renderObjects);
	GetAnimatedMeshObjectsForInstancing(_renderObjects);

}
void RenderManager::GetDefaultRenderObjectsForInstancing(vector<shared_ptr<GameObject>>& gameObjects)
{
	map<InstanceID, vector<shared_ptr<GameObject>>> cache;
	vector<shared_ptr<GameObject>> objectsToRemove;  // 삭제할 객체들을 저장할 벡터

	// 첫 번째 순회: 캐시 구성
	for (shared_ptr<GameObject>& gameObject : gameObjects)
	{
		shared_ptr<MeshRenderer> meshRenderer = gameObject->GetComponent<MeshRenderer>();
		if (meshRenderer == nullptr)
			continue;
		shared_ptr<RenderPass> renderPass = meshRenderer->GetRenderPasses()[0];
		if (renderPass->GetPass() != Pass::DEFAULT_RENDER)
			continue;

		const InstanceID instanceId = meshRenderer->GetInstanceID_DefaultMesh();
		cache[instanceId].push_back(gameObject);
		objectsToRemove.push_back(gameObject);  // 삭제할 객체 목록에 추가
	}
	
	// 두 번째 순회: 객체 제거
	for (const auto& obj : objectsToRemove)
	{
		if (objectsToRemove.size() == 1)
		{
			shared_ptr<MeshRenderer> meshRenderer = obj->GetComponent<MeshRenderer>();
			if (meshRenderer)
			{
				const InstanceID instanceId = meshRenderer->GetInstanceID_DefaultMesh();
				auto cacheIt = cache.find(instanceId);
				if (cacheIt != cache.end())
				{
					cache.erase(cacheIt);
				}
			}
			break;
		}
		auto it = find(gameObjects.begin(), gameObjects.end(), obj);
		if (it != gameObjects.end())
		{
			gameObjects.erase(it);
		}
	}

	cache_DefaultRender = cache;
	
}
void RenderManager::DrawInstancingDefaultRenderObject(bool isEnv)
{
	for (auto& pair : cache_DefaultRender)
	{
		const vector<shared_ptr<GameObject>>& vec = pair.second;

		if (vec.size() == 1)
		{
			//_renderObjects.push_back(vec[0]);
		}
		else
		{
			const InstanceID instanceId = pair.first;

			for (int32 i = 0; i < vec.size(); i++)
			{
				const shared_ptr<GameObject>& gameObject = vec[i];
				InstancingData data;
				data.world = gameObject->transform()->GetWorldMatrix();

				AddData(instanceId, data);
			}

			//// 첫놈을 대표로 해서 모든걸 그리도록 함.
			//shared_ptr<InstancingBuffer>& buffer = _buffers[instanceId];
			//vec[0]->GetComponent<MeshRenderer>()->RenderInstancing(buffer);
			shared_ptr<MeshRenderer> meshRenderer = pair.second[0]->GetComponent<MeshRenderer>();

			if (meshRenderer->GetRenderPasses().size() > 0)
			{
				int count = meshRenderer->GetRenderPasses().size();
				for (int i = 0; i < meshRenderer->GetRenderPasses().size(); i++)
				{
					shared_ptr<RenderPass> renderPass = meshRenderer->GetRenderPasses()[i];
					renderPass->DefaultRenderInstance(isEnv, _buffers[instanceId]);
					if (i + 1 < meshRenderer->GetRenderPasses().size())
					{
						shared_ptr<RenderPass> nextRenderPass = meshRenderer->GetRenderPasses()[i + 1];
						nextRenderPass->SetInputSRV(renderPass->GetOutputSRV());
					}
				}
			}
		}
	}
}
void RenderManager::GetStaticMeshObjectsForInstancing(vector<shared_ptr<GameObject>>& gameObjects)
{
	map<InstanceID, vector<shared_ptr<GameObject>>> cache;
	vector<shared_ptr<GameObject>> objectsToRemove;  // 삭제할 객체들을 저장할 벡터

	// 첫 번째 순회: 캐시 구성
	for (shared_ptr<GameObject>& gameObject : gameObjects)
	{
		shared_ptr<MeshRenderer> meshRenderer = gameObject->GetComponent<MeshRenderer>();
		if (meshRenderer == nullptr)
			continue;
		shared_ptr<RenderPass> renderPass = meshRenderer->GetRenderPasses()[0];
		if (renderPass->GetPass() != Pass::STATIC_MESH_RENDER)
			continue;

		const InstanceID instanceId = meshRenderer->GetInstanceID_ModelMesh();
		cache[instanceId].push_back(gameObject);
		objectsToRemove.push_back(gameObject);  // 삭제할 객체 목록에 추가
	}

	// 두 번째 순회: 객체 제거
	for (const auto& obj : objectsToRemove)
	{
		if (objectsToRemove.size() == 1)
		{
			shared_ptr<MeshRenderer> meshRenderer = obj->GetComponent<MeshRenderer>();
			if (meshRenderer)
			{
				const InstanceID instanceId = meshRenderer->GetInstanceID_ModelMesh();
				auto cacheIt = cache.find(instanceId);
				if (cacheIt != cache.end())
				{
					cache.erase(cacheIt);
				}
			}
			break;
		}
		auto it = find(gameObjects.begin(), gameObjects.end(), obj);
		if (it != gameObjects.end())
		{
			gameObjects.erase(it);
		}
	}
	cache_StaticMeshRender = cache;
}

void RenderManager::DrawInstancingStaticMeshObject(bool isEnv)
{
	for (auto& pair : cache_StaticMeshRender)
	{

		const vector<shared_ptr<GameObject>>& vec = pair.second;

		if (vec.size() == 1)
		{
			//_renderObjects.push_back(vec[0]);
		}
		else
		{
			const InstanceID instanceId = pair.first;

			_cache_blendDescs = make_shared<InstancedBlendDesc>();

			for (int32 i = 0; i < vec.size(); i++)
			{
				const shared_ptr<GameObject>& gameObject = vec[i];
				InstancingData data;
				data.world = gameObject->transform()->GetWorldMatrix();

				AddData(instanceId, data);
			}

			//// 첫놈을 대표로 해서 모든걸 그리도록 함.
			//shared_ptr<InstancingBuffer>& buffer = _buffers[instanceId];
			//vec[0]->GetComponent<MeshRenderer>()->RenderInstancing(buffer);
			shared_ptr<MeshRenderer> meshRenderer = vec[0]->GetComponent<MeshRenderer>();

			if (meshRenderer->GetRenderPasses().size() > 0)
			{
				int count = meshRenderer->GetRenderPasses().size();
				for (int i = 0; i < meshRenderer->GetRenderPasses().size(); i++)
				{
					shared_ptr<RenderPass> renderPass = meshRenderer->GetRenderPasses()[i];
					renderPass->StaticMeshRenderInstance(isEnv, _buffers[instanceId]);
					if (i + 1 < meshRenderer->GetRenderPasses().size())
					{
						shared_ptr<RenderPass> nextRenderPass = meshRenderer->GetRenderPasses()[i + 1];
						nextRenderPass->SetInputSRV(renderPass->GetOutputSRV());
					}
				}
			}
		}

	}
}
void RenderManager::GetAnimatedMeshObjectsForInstancing(vector<shared_ptr<GameObject>>& gameObjects)
{
	map<InstanceID, vector<shared_ptr<GameObject>>> cache;
	vector<shared_ptr<GameObject>> objectsToRemove;  // 삭제할 객체들을 저장할 벡터

	// 첫 번째 순회: 캐시 구성
	for (shared_ptr<GameObject>& gameObject : gameObjects)
	{
		shared_ptr<MeshRenderer> meshRenderer = gameObject->GetComponent<MeshRenderer>();
		if (meshRenderer == nullptr)
			continue;
		shared_ptr<RenderPass> renderPass = meshRenderer->GetRenderPasses()[0];
		if (renderPass->GetPass() != Pass::ANIMATED_MESH_RENDER)
			continue;

		const InstanceID instanceId = meshRenderer->GetInstanceID_ModelMesh();
		cache[instanceId].push_back(gameObject);
		objectsToRemove.push_back(gameObject);  // 삭제할 객체 목록에 추가
	}

	// 두 번째 순회: 객체 제거
	for (const auto& obj : objectsToRemove)
	{
		if (objectsToRemove.size() == 1)
		{
			shared_ptr<MeshRenderer> meshRenderer = obj->GetComponent<MeshRenderer>();
			if (meshRenderer)
			{
				const InstanceID instanceId = meshRenderer->GetInstanceID_ModelMesh();
				auto cacheIt = cache.find(instanceId);
				if (cacheIt != cache.end())
				{
					cache.erase(cacheIt);
				}
			}
			break;
		}
		auto it = find(gameObjects.begin(), gameObjects.end(), obj);
		if (it != gameObjects.end())
		{
			gameObjects.erase(it);
		}
	}

	
	cache_AnimatedRender = cache;

}

void RenderManager::DrawInstancingAnimatedMeshObject(bool isEnv)
{
	for (auto& pair : cache_AnimatedRender)
	{

		const vector<shared_ptr<GameObject>>& vec = pair.second;

		if (vec.size() == 1)
		{
			//_renderObjects.push_back(vec[0]);
		}
		else
		{
			const InstanceID instanceId = pair.first;

			_cache_blendDescs = make_shared<InstancedBlendDesc>();

			for (int32 i = 0; i < vec.size(); i++)
			{
				const shared_ptr<GameObject>& gameObject = vec[i];
				InstancingData data;
				data.world = gameObject->transform()->GetWorldMatrix();

				AddData(instanceId, data);

				shared_ptr<Animator> animator = gameObject->GetComponent<Animator>();
				if (animator)
					_cache_blendDescs->blendDesc[i] = gameObject->GetComponent<Animator>()->GetBlendAnimDesc();
				else
				{
					BlendAnimDesc desc;
					desc.curr.activeAnimation = 0;
					desc.next.activeAnimation = 0;
					_cache_blendDescs->blendDesc[i] = desc;
				}
					
			}

			//// 첫놈을 대표로 해서 모든걸 그리도록 함.
			//shared_ptr<InstancingBuffer>& buffer = _buffers[instanceId];
			//vec[0]->GetComponent<MeshRenderer>()->RenderInstancing(buffer);
			shared_ptr<MeshRenderer> meshRenderer = vec[0]->GetComponent<MeshRenderer>();

			if (meshRenderer->GetRenderPasses().size() > 0)
			{
				int count = meshRenderer->GetRenderPasses().size();
				for (int i = 0; i < meshRenderer->GetRenderPasses().size(); i++)
				{
					shared_ptr<RenderPass> renderPass = meshRenderer->GetRenderPasses()[i];
					renderPass->AnimatedMeshRenderInstance(isEnv, _buffers[instanceId], *_cache_blendDescs);
					if (i + 1 < meshRenderer->GetRenderPasses().size())
					{
						shared_ptr<RenderPass> nextRenderPass = meshRenderer->GetRenderPasses()[i + 1];
						nextRenderPass->SetInputSRV(renderPass->GetOutputSRV());
					}
				}
			}
		}
		
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

	ClearData();
	DrawInstancingDefaultRenderObject(isEnv);
	DrawInstancingStaticMeshObject(isEnv);
	DrawInstancingAnimatedMeshObject(isEnv);

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
			if (true/*_envTexture == nullptr*/) {

				shared_ptr<Material> material = make_shared<Material>();
				material->CreateEnvironmentMapTexture(gameObject);
			
				_envTexture = make_shared<Texture>();
				_envTexture->SetShaderResourceView(material->GetCubeMapSRV());
			}
			RenderEnvironmentMappedObjects(gameObject, _envTexture);
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

void RenderManager::ClearData()
{
	for (auto& pair : _buffers)
	{
		shared_ptr<InstancingBuffer>& buffer = pair.second;
		buffer->ClearData();
	}
}



void RenderManager::AddData(InstanceID instanceId, InstancingData& data)
{
	if (_buffers.find(instanceId) == _buffers.end())
		_buffers[instanceId] = make_shared<InstancingBuffer>();

	_buffers[instanceId]->AddData(data);
}
