#include "pch.h"
#include "RenderManager.h"
#include <thread> // std::this_thread::sleep_for
#include <chrono> // std::chrono::seconds
void RenderManager::Init()
{
	_pipeline = make_shared<Pipeline>();

}

void RenderManager::Update()
{
	ClearRenderObject();

	GP.SetRenderTarget();
	GP.SetViewport();

	

	GetRenderableObject();
	Render();

	GP.SwapChain();
}

void RenderManager::GetRenderableObject()
{
	vector<shared_ptr<GameObject>> gameObjects = SCENE.GetActiveScene()->GetGameObjects();
	for (shared_ptr<GameObject>& gameObject : gameObjects)
	{
		shared_ptr<MeshRenderer> meshRenderer = gameObject->GetComponent<MeshRenderer>();
		if (meshRenderer != nullptr)
		{
			if (!meshRenderer->CheckUseEnvironmentMap())
				_renderObjects.push_back(gameObject);
			else
				_envMappedObjects.push_back(gameObject);
		}
			
		if (gameObject->GetComponent<Billboard>() != nullptr)
			_billboardObjs.push_back(gameObject);
	}
}

void RenderManager::RenderRenderableObject(bool isEnv)
{
	for (const shared_ptr<GameObject>& gameObject : _renderObjects)
	{

		shared_ptr<MeshRenderer> meshRenderer = gameObject->GetComponent<MeshRenderer>();
		shared_ptr<Model> model = meshRenderer->GetModel();

		shared_ptr<Shader> shader = meshRenderer->GetShader();

		shared_ptr<GameObject> cameraObject;
		if (gameObject->GetObjectType() == GameObjectType::NormalObject)
			cameraObject = SCENE.GetActiveScene()->Find(L"MainCamera");
		else
			cameraObject = SCENE.GetActiveScene()->Find(L"UICamera");


		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"TransformBuffer", 1, gameObject->GetTransformBuffer());

		if (cameraObject != nullptr)
		{
			if(isEnv)
				shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"CameraBuffer", 1, cameraObject->GetEnvCameraBuffer());
			else
				shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"CameraBuffer", 1, cameraObject->GetCameraBuffer());
		}
		shared_ptr<GameObject> lightObject = SCENE.GetActiveScene()->FindWithComponent(ComponentType::Light);
		if (lightObject != nullptr)
		{
			lightObject->GetLightBuffer();
			shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightDesc", 1, lightObject->GetLightBuffer());
		}

		LightAndCameraPos lightDirection;
		lightDirection.lightPosition = SCENE.GetActiveScene()->FindWithComponent(ComponentType::Light)->transform()->GetWorldPosition();
		lightDirection.cameraPosition = SCENE.GetActiveScene()->FindWithComponent(ComponentType::Camera)->transform()->GetWorldPosition();

		shared_ptr<Buffer> light = make_shared<Buffer>();
		light->CreateConstantBuffer<LightAndCameraPos>();
		light->CopyData(lightDirection);


		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightMaterial", 1, meshRenderer->GetMaterialBuffer());
		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightAndCameraPos", 1, light);

		if (model == nullptr)
		{
			// for stencil test
			if (gameObject->GetName() == L"stencil_cube1")
			{
				shared_ptr<Material> material = meshRenderer->GetMaterial();
				_pipeline->SetBuffer(meshRenderer->GetMesh()->GetBuffer());

				_pipeline->SetIndicesSize(meshRenderer->GetMesh()->GetGeometry()->GetIndices().size());
				_pipeline->SetTexture(material->GetTexture());
				_pipeline->SetNormalMap(material->GetNormalMap());
				_pipeline->SetShader(material->GetShader());


				_pipelineInfo = _pipeline->GetPipelineInfo();

				_pipelineInfo.blendState = make_shared<BlendState>();
				_pipelineInfo.blendState->CreateBlendState();

				_pipelineInfo.inputLayout = material->GetShader()->GetInputLayout();

				_pipelineInfo.rasterizerState = make_shared<RasterizerState>();
				_pipelineInfo.rasterizerState->CreateRasterizerState(meshRenderer->GetRasterzerStates());

				_pipelineInfo.depthStencilState = make_shared<DepthStencilState>();
				_pipelineInfo.depthStencilState->SetDepthStencilState(DSState::CUSTOM1);

				_pipelineInfo.samplerState = make_shared<SamplerState>();
				_pipelineInfo.samplerState->CreateSamplerState();


				LightAndCameraPos lightDirection;
				lightDirection.lightPosition = SCENE.GetActiveScene()->FindWithComponent(ComponentType::Light)->transform()->GetWorldPosition();
				lightDirection.cameraPosition = SCENE.GetActiveScene()->FindWithComponent(ComponentType::Camera)->transform()->GetWorldPosition();

				shared_ptr<Buffer> light = make_shared<Buffer>();
				light->CreateConstantBuffer<LightAndCameraPos>();
				light->CopyData(lightDirection);


				shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightMaterial", 1, meshRenderer->GetMaterialBuffer());
				shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightAndCameraPos", 1, light);

				_pipeline->UpdatePipeline(_pipelineInfo);
			}
			if (gameObject->GetName() == L"stencil_cube2")
			{
				shared_ptr<Material> material = meshRenderer->GetMaterial();
				_pipeline->SetBuffer(meshRenderer->GetMesh()->GetBuffer());

				_pipeline->SetIndicesSize(meshRenderer->GetMesh()->GetGeometry()->GetIndices().size());
				_pipeline->SetTexture(material->GetTexture());
				_pipeline->SetNormalMap(material->GetNormalMap());
				_pipeline->SetShader(material->GetShader());


				_pipelineInfo = _pipeline->GetPipelineInfo();

				_pipelineInfo.blendState = make_shared<BlendState>();
				_pipelineInfo.blendState->CreateBlendState();

				_pipelineInfo.inputLayout = material->GetShader()->GetInputLayout();

				_pipelineInfo.rasterizerState = make_shared<RasterizerState>();
				_pipelineInfo.rasterizerState->CreateRasterizerState(meshRenderer->GetRasterzerStates());

				_pipelineInfo.depthStencilState = make_shared<DepthStencilState>();
				_pipelineInfo.depthStencilState->SetDepthStencilState(DSState::CUStom2);

				_pipelineInfo.samplerState = make_shared<SamplerState>();
				_pipelineInfo.samplerState->CreateSamplerState();


				LightAndCameraPos lightDirection;
				lightDirection.lightPosition = SCENE.GetActiveScene()->FindWithComponent(ComponentType::Light)->transform()->GetWorldPosition();
				lightDirection.cameraPosition = SCENE.GetActiveScene()->FindWithComponent(ComponentType::Camera)->transform()->GetWorldPosition();

				shared_ptr<Buffer> light = make_shared<Buffer>();
				light->CreateConstantBuffer<LightAndCameraPos>();
				light->CopyData(lightDirection);


				shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightMaterial", 1, meshRenderer->GetMaterialBuffer());
				shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightAndCameraPos", 1, light);

				_pipeline->UpdatePipeline(_pipelineInfo);
				DEVICECONTEXT->OMSetDepthStencilState(0, 0);
			}
			else
			{
				if (meshRenderer->GetRenderPasses().size() > 0)
				{
					int count = meshRenderer->GetRenderPasses().size();
					for (int i = 0; i < meshRenderer->GetRenderPasses().size(); i++)
					{
						shared_ptr<RenderPass> renderPass = meshRenderer->GetRenderPasses()[i];
						renderPass->Render();
						if (i + 1 < meshRenderer->GetRenderPasses().size())
						{
							shared_ptr<RenderPass> nextRenderPass = meshRenderer->GetRenderPasses()[i + 1];
							nextRenderPass->SetInputSRV(renderPass->GetOutputSRV());
						}
					}
				}
			}

		}
		else
		{
			if (model->HasAnimation() && !isEnv)
			{
				/*_blendAnimDesc.SetAnimIndex(1,2);*/
				animationSumTime += TIME.GetDeltaTime();
				_blendAnimDesc.curr.sumTime += TIME.GetDeltaTime();
				_blendAnimDesc.SetAnimSpeed(2.0f, 2.0f);

				shared_ptr<Buffer> blendBuffer = make_shared<Buffer>();

				{
					shared_ptr<ModelAnimation> current = model->GetAnimationByIndex(_blendAnimDesc.curr.animIndex);

					if (current)
					{
						float timePerFrame = 1 / (current->frameRate * _blendAnimDesc.curr.speed); // 1프레임에 몇초냐?
						if (_blendAnimDesc.curr.sumTime >= timePerFrame)
						{
							_blendAnimDesc.curr.sumTime = 0.f;
							_blendAnimDesc.curr.currFrame = (_blendAnimDesc.curr.currFrame + 1) % current->frameCount;
							_blendAnimDesc.curr.nextFrame = (_blendAnimDesc.curr.currFrame + 1) % current->frameCount;
						}

						_blendAnimDesc.curr.ratio = (_blendAnimDesc.curr.sumTime / timePerFrame);
					}
				}

				if (animationSumTime > 3.0f)
				{
					_blendAnimDesc.blendSumTime += TIME.GetDeltaTime();
					_blendAnimDesc.blendRatio = (_blendAnimDesc.blendSumTime / _blendAnimDesc.blendDuration) * (_blendAnimDesc.curr.speed + _blendAnimDesc.next.speed);

					if (_blendAnimDesc.blendRatio > 1.0f)
					{
						animationSumTime = 0.0f;
						_blendAnimDesc.ClearNextAnim();
					}
					else
					{
						shared_ptr<ModelAnimation> next = model->GetAnimationByIndex(_blendAnimDesc.next.animIndex);

						if (next)
						{
							float timePerFrame = 1 / (next->frameRate * _blendAnimDesc.next.speed); // 1프레임에 몇초냐?
							if (_blendAnimDesc.next.ratio >= 1.0f)
							{
								_blendAnimDesc.next.sumTime = 0.f;
								_blendAnimDesc.next.currFrame = (_blendAnimDesc.next.currFrame + 1) % next->frameCount;
								_blendAnimDesc.next.nextFrame = (_blendAnimDesc.next.currFrame + 1) % next->frameCount;
							}

							_blendAnimDesc.next.ratio = (_blendAnimDesc.next.sumTime / timePerFrame);
						}
					}
				}

				blendBuffer->CreateConstantBuffer<BlendAnimDesc>();
				blendBuffer->CopyData(_blendAnimDesc);

				shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"BlendBuffer", 1, blendBuffer);
				UINT animationTexture_Slot = meshRenderer->GetMaterial()->GetShader()->GetShaderSlot()->GetSlotNumber(L"TransformMap");
				shader->PushShaderResourceToShader(ShaderType::VERTEX_SHADER, L"TransformMap", 1, model->GetAnimationTextureBuffer());


			}
			else
			{
				BoneBuffer boneDesc;

				const uint32 boneCount = model->GetBoneCount();
				for (uint32 i = 0; i < boneCount; i++)
				{
					shared_ptr<ModelBone> bone = model->GetBoneByIndex(i);
					boneDesc.BoneTransforms[i] = bone->transform;
				}

				shared_ptr<Buffer> boneBuffer = make_shared<Buffer>();
				boneBuffer->CreateConstantBuffer<BoneBuffer>();
				boneBuffer->CopyData(boneDesc);

				shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"BoneBuffer", 1, boneBuffer);
			}


			const auto& meshes = model->GetMeshes();
			for (auto& mesh : meshes)
			{
				if (mesh->material)
				{
					_pipeline->SetNormalMap(mesh->material->GetNormalMap());
					_pipeline->SetSpecularMap(mesh->material->GetSpecularMap());
					_pipeline->SetDiffuseMap(mesh->material->GetDiffuseMap());

				}

				if (!model->HasAnimation())
				{
					// BoneIndex
					BoneIndex boneIndex;
					boneIndex.index = mesh->boneIndex;
					shared_ptr<Buffer> boneIndexBuffer = make_shared<Buffer>();
					boneIndexBuffer->CreateConstantBuffer<BoneIndex>();
					boneIndexBuffer->CopyData(boneIndex);
					shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"BonIndex", 1, boneIndexBuffer);
				}

				// Light
				LightAndCameraPos lightDirection;
				lightDirection.lightPosition = SCENE.GetActiveScene()->FindWithComponent(ComponentType::Light)->transform()->GetWorldPosition();
				lightDirection.cameraPosition = SCENE.GetActiveScene()->FindWithComponent(ComponentType::Camera)->transform()->GetWorldPosition();

				shared_ptr<Buffer> light = make_shared<Buffer>();
				light->CreateConstantBuffer<LightAndCameraPos>();
				light->CopyData(lightDirection);


				shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightAndCameraPos", 1, light);

				_pipeline->SetBuffer(mesh->GetBuffer());
				_pipeline->SetIndicesSize(mesh->GetGeometry()->GetIndices().size());
				_pipeline->SetShader(mesh->material->GetShader());

				_pipelineInfo = _pipeline->GetPipelineInfo();

				_pipelineInfo.blendState = make_shared<BlendState>();
				_pipelineInfo.blendState->CreateBlendState();

				_pipelineInfo.inputLayout = mesh->material->GetShader()->GetInputLayout();

				_pipelineInfo.rasterizerState = make_shared<RasterizerState>();
				_pipelineInfo.rasterizerState->CreateRasterizerState(meshRenderer->GetRasterzerStates());

				_pipelineInfo.depthStencilState = make_shared<DepthStencilState>();
				_pipelineInfo.depthStencilState->SetDepthStencilState(DSState::NORMAL);

				_pipelineInfo.samplerState = make_shared<SamplerState>();
				_pipelineInfo.samplerState->CreateSamplerState();
				_pipeline->UpdatePipeline(_pipelineInfo);


			}
		}

	}
}

void RenderManager::RenderEnvironmentMappedObjects(shared_ptr<GameObject> gameObject, shared_ptr<Texture> envTexture)
{
	shared_ptr<MeshRenderer> meshRenderer = gameObject->GetComponent<MeshRenderer>();
	shared_ptr<Model> model = meshRenderer->GetModel();

	shared_ptr<Shader> shader = RESOURCE.GetResource<Shader>(L"EnvironmentMap_Shader");

	meshRenderer->GetRenderPasses()[0]->SetTexture(envTexture);
	meshRenderer->GetRenderPasses()[0]->SetShader(shader);

	shared_ptr<GameObject> cameraObject;
	if (gameObject->GetObjectType() == GameObjectType::NormalObject)
		cameraObject = SCENE.GetActiveScene()->Find(L"MainCamera");
	else
		cameraObject = SCENE.GetActiveScene()->Find(L"UICamera");


	shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"TransformBuffer", 1, gameObject->GetTransformBuffer());

	if (cameraObject != nullptr)
	{
		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"CameraBuffer", 1, cameraObject->GetCameraBuffer());
	}
	shared_ptr<GameObject> lightObject = SCENE.GetActiveScene()->FindWithComponent(ComponentType::Light);
	if (lightObject != nullptr)
	{
		lightObject->GetLightBuffer();
		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightDesc", 1, lightObject->GetLightBuffer());
	}

	LightAndCameraPos lightDirection;
	lightDirection.lightPosition = SCENE.GetActiveScene()->FindWithComponent(ComponentType::Light)->transform()->GetWorldPosition();
	lightDirection.cameraPosition = SCENE.GetActiveScene()->FindWithComponent(ComponentType::Camera)->transform()->GetWorldPosition();

	shared_ptr<Buffer> light = make_shared<Buffer>();
	light->CreateConstantBuffer<LightAndCameraPos>();
	light->CopyData(lightDirection);


	shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightMaterial", 1, meshRenderer->GetMaterialBuffer());
	shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightAndCameraPos", 1, light);

	if (model == nullptr)
	{
		if (meshRenderer->GetRenderPasses().size() > 0)
		{
			int count = meshRenderer->GetRenderPasses().size();
			for (int i = 0; i < meshRenderer->GetRenderPasses().size(); i++)
			{
				shared_ptr<RenderPass> renderPass = meshRenderer->GetRenderPasses()[i];
				renderPass->Render();
				if (i + 1 < meshRenderer->GetRenderPasses().size())
				{
					shared_ptr<RenderPass> nextRenderPass = meshRenderer->GetRenderPasses()[i + 1];
					nextRenderPass->SetInputSRV(renderPass->GetOutputSRV());
				}
			}
		}
	}
	//else
	//{
	//	if (model->HasAnimation())
	//	{
	//		/*_blendAnimDesc.SetAnimIndex(1,2);*/
	//		animationSumTime += TIME.GetDeltaTime();
	//		_blendAnimDesc.curr.sumTime += TIME.GetDeltaTime();
	//		_blendAnimDesc.SetAnimSpeed(2.0f, 2.0f);

	//		shared_ptr<Buffer> blendBuffer = make_shared<Buffer>();

	//		{
	//			shared_ptr<ModelAnimation> current = model->GetAnimationByIndex(_blendAnimDesc.curr.animIndex);

	//			if (current)
	//			{
	//				float timePerFrame = 1 / (current->frameRate * _blendAnimDesc.curr.speed); // 1프레임에 몇초냐?
	//				if (_blendAnimDesc.curr.sumTime >= timePerFrame)
	//				{
	//					_blendAnimDesc.curr.sumTime = 0.f;
	//					_blendAnimDesc.curr.currFrame = (_blendAnimDesc.curr.currFrame + 1) % current->frameCount;
	//					_blendAnimDesc.curr.nextFrame = (_blendAnimDesc.curr.currFrame + 1) % current->frameCount;
	//				}

	//				_blendAnimDesc.curr.ratio = (_blendAnimDesc.curr.sumTime / timePerFrame);
	//			}
	//		}

	//		if (animationSumTime > 3.0f)
	//		{
	//			_blendAnimDesc.blendSumTime += TIME.GetDeltaTime();
	//			_blendAnimDesc.blendRatio = (_blendAnimDesc.blendSumTime / _blendAnimDesc.blendDuration) * (_blendAnimDesc.curr.speed + _blendAnimDesc.next.speed);

	//			if (_blendAnimDesc.blendRatio > 1.0f)
	//			{
	//				animationSumTime = 0.0f;
	//				_blendAnimDesc.ClearNextAnim();
	//			}
	//			else
	//			{
	//				shared_ptr<ModelAnimation> next = model->GetAnimationByIndex(_blendAnimDesc.next.animIndex);

	//				if (next)
	//				{
	//					float timePerFrame = 1 / (next->frameRate * _blendAnimDesc.next.speed); // 1프레임에 몇초냐?
	//					if (_blendAnimDesc.next.ratio >= 1.0f)
	//					{
	//						_blendAnimDesc.next.sumTime = 0.f;
	//						_blendAnimDesc.next.currFrame = (_blendAnimDesc.next.currFrame + 1) % next->frameCount;
	//						_blendAnimDesc.next.nextFrame = (_blendAnimDesc.next.currFrame + 1) % next->frameCount;
	//					}

	//					_blendAnimDesc.next.ratio = (_blendAnimDesc.next.sumTime / timePerFrame);
	//				}
	//			}
	//		}

	//		blendBuffer->CreateConstantBuffer<BlendAnimDesc>();
	//		blendBuffer->CopyData(_blendAnimDesc);

	//		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"BlendBuffer", 1, blendBuffer);
	//		UINT animationTexture_Slot = meshRenderer->GetMaterial()->GetShader()->GetShaderSlot()->GetSlotNumber(L"TransformMap");
	//		shader->PushShaderResourceToShader(ShaderType::VERTEX_SHADER, L"TransformMap", 1, model->GetAnimationTextureBuffer());


	//	}
	//	else
	//	{
	//		BoneBuffer boneDesc;

	//		const uint32 boneCount = model->GetBoneCount();
	//		for (uint32 i = 0; i < boneCount; i++)
	//		{
	//			shared_ptr<ModelBone> bone = model->GetBoneByIndex(i);
	//			boneDesc.BoneTransforms[i] = bone->transform;
	//		}

	//		shared_ptr<Buffer> boneBuffer = make_shared<Buffer>();
	//		boneBuffer->CreateConstantBuffer<BoneBuffer>();
	//		boneBuffer->CopyData(boneDesc);

	//		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"BoneBuffer", 1, boneBuffer);
	//	}


	//	const auto& meshes = model->GetMeshes();
	//	for (auto& mesh : meshes)
	//	{
	//		if (mesh->material)
	//		{
	//			_pipeline->SetNormalMap(mesh->material->GetNormalMap());
	//			_pipeline->SetSpecularMap(mesh->material->GetSpecularMap());
	//			_pipeline->SetDiffuseMap(mesh->material->GetDiffuseMap());

	//		}

	//		if (!model->HasAnimation())
	//		{
	//			// BoneIndex
	//			BoneIndex boneIndex;
	//			boneIndex.index = mesh->boneIndex;
	//			shared_ptr<Buffer> boneIndexBuffer = make_shared<Buffer>();
	//			boneIndexBuffer->CreateConstantBuffer<BoneIndex>();
	//			boneIndexBuffer->CopyData(boneIndex);
	//			shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"BonIndex", 1, boneIndexBuffer);
	//		}

	//		// Light
	//		LightAndCameraPos lightDirection;
	//		lightDirection.lightPosition = SCENE.GetActiveScene()->FindWithComponent(ComponentType::Light)->transform()->GetWorldPosition();
	//		lightDirection.cameraPosition = SCENE.GetActiveScene()->FindWithComponent(ComponentType::Camera)->transform()->GetWorldPosition();

	//		shared_ptr<Buffer> light = make_shared<Buffer>();
	//		light->CreateConstantBuffer<LightAndCameraPos>();
	//		light->CopyData(lightDirection);


	//		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightAndCameraPos", 1, light);

	//		_pipeline->SetBuffer(mesh->GetBuffer());
	//		_pipeline->SetIndicesSize(mesh->GetGeometry()->GetIndices().size());
	//		_pipeline->SetShader(mesh->material->GetShader());

	//		_pipelineInfo = _pipeline->GetPipelineInfo();

	//		_pipelineInfo.blendState = make_shared<BlendState>();
	//		_pipelineInfo.blendState->CreateBlendState();

	//		_pipelineInfo.inputLayout = mesh->material->GetShader()->GetInputLayout();

	//		_pipelineInfo.rasterizerState = make_shared<RasterizerState>();
	//		_pipelineInfo.rasterizerState->CreateRasterizerState(meshRenderer->GetRasterzerStates());

	//		_pipelineInfo.depthStencilState = make_shared<DepthStencilState>();
	//		_pipelineInfo.depthStencilState->SetDepthStencilState(DSState::NORMAL);

	//		_pipelineInfo.samplerState = make_shared<SamplerState>();
	//		_pipelineInfo.samplerState->CreateSamplerState();
	//		_pipeline->UpdatePipeline(_pipelineInfo);


	//	}
	//}
}

void RenderManager::Render()
{
	if (_filterType == FilterType::GAUSSIAN_BLUR)
		GP.SetOffscreenRenderTarget();

	if (_billboardObjs.size() > 0)
	{
		for (const shared_ptr<GameObject>& gameObject : _billboardObjs)
		{
			gameObject->GetComponent<Billboard>()->DrawBillboard();
		}
	}
		
	RenderRenderableObject(false);

	for (const shared_ptr<GameObject>& gameObject : _envMappedObjects)
	{
		shared_ptr<Material> material = make_shared<Material>();
		material->CreateEnvironmentMapTexture(gameObject);
		shared_ptr<Texture> envTexture = make_shared<Texture>();
		envTexture->SetShaderResourceView(material->GetCubeMapSRV());

		RenderEnvironmentMappedObjects(gameObject, envTexture);
	}

	if (_filterType == FilterType::GAUSSIAN_BLUR)
		GP.RenderQuad();
	
}

void RenderManager::ClearRenderObject()
{
	_renderObjects.clear();
	_envMappedObjects.clear();
	_billboardObjs.clear();
}
