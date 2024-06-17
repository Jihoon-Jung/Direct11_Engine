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

	vector<shared_ptr<GameObject>> gameObjects = SCENE.GetActiveScene()->GetGameObjects();
	shared_ptr<GameObject> cameraObject = SCENE.GetActiveScene()->FindWithComponent(ComponentType::Camera);
	if (cameraObject != nullptr)
	{
		cameraObject->GetCameraBuffer();
		DEVICECONTEXT->VSSetConstantBuffers(0, 1, cameraObject->GetCameraBuffer()->GetConstantBuffer().GetAddressOf());
	}
	shared_ptr<GameObject> lightObject = SCENE.GetActiveScene()->FindWithComponent(ComponentType::Light);
	if (lightObject != nullptr)
	{
		lightObject->GetLightBuffer();
		DEVICECONTEXT->PSSetConstantBuffers(3, 1, lightObject->GetLightBuffer()->GetConstantBuffer().GetAddressOf());
	}

	GetRenderObject();
	RenderObject();

	GP.SwapChain();
}

void RenderManager::GetRenderObject()
{
	vector<shared_ptr<GameObject>> gameObjects = SCENE.GetActiveScene()->GetGameObjects();
	for (shared_ptr<GameObject>& gameObject : gameObjects)
	{
		shared_ptr<MeshRenderer> meshRenderer = gameObject->GetComponent<MeshRenderer>();
		if (meshRenderer != nullptr)
			_renderObjects.push_back(gameObject);
	}
}

void RenderManager::RenderObject()
{
	for (const shared_ptr<GameObject>& gameObject : _renderObjects)
	{
		_pipeline->isAnimation = false;
		shared_ptr<MeshRenderer> meshRenderer = gameObject->GetComponent<MeshRenderer>();
		shared_ptr<Model> model = meshRenderer->GetModel();
		if (model == nullptr)
		{
			
			shared_ptr<Material> material = meshRenderer->GetMaterial();
			_pipeline->SetBuffer(meshRenderer->GetMesh()->GetBuffer());

			_pipeline->SetIndicesSize(meshRenderer->GetMesh()->GetGeometry()->GetIndices().size());
			_pipeline->SetTexture(material->GetTexture());
			_pipeline->SetNormalMap(material->GetNormalMap());
			_pipeline->SetShader(material->GetVertexShader(), material->GetPixelShader());


			_pipelineInfo = _pipeline->GetPipelineInfo();
			_pipelineInfo.blendState = make_shared<BlendState>();
			_pipelineInfo.blendState->CreateBlendState();
			_pipelineInfo.inputLayout = material->GetVertexShader()->GetInputLayout();
			_pipelineInfo.rasterizerState = make_shared<RasterizerState>();
			_pipelineInfo.rasterizerState->CreateRasterizerState(meshRenderer->GetRasterzerStates());

			_pipelineInfo.samplerState = make_shared<SamplerState>();
			_pipelineInfo.samplerState->CreateSamplerState();
			

			LightAndCameraPos lightDirection;
			lightDirection.lightPosition = SCENE.GetActiveScene()->FindWithComponent(ComponentType::Light)->transform()->GetWorldPosition();
			lightDirection.cameraPosition = SCENE.GetActiveScene()->FindWithComponent(ComponentType::Camera)->transform()->GetWorldPosition();

			shared_ptr<Buffer> light = make_shared<Buffer>();
			light->CreateConstantBuffer<LightAndCameraPos>();
			light->CopyData(lightDirection);

			DEVICECONTEXT->VSSetConstantBuffers(1, 1, gameObject->GetTransformBuffer()->GetConstantBuffer().GetAddressOf());
			DEVICECONTEXT->PSSetConstantBuffers(2, 1, meshRenderer->GetMaterialBuffer()->GetConstantBuffer().GetAddressOf());
			DEVICECONTEXT->PSSetConstantBuffers(4, 1, light->GetConstantBuffer().GetAddressOf());

			_pipeline->UpdatePipeline(_pipelineInfo);
		}
		else
		{
			if (model->HasAnimation())
			{
				/*_blendAnimDesc.SetAnimIndex(1,2);*/
				animationSumTime += TIME.GetDeltaTime();
				_blendAnimDesc.curr.sumTime += TIME.GetDeltaTime();
				_blendAnimDesc.SetAnimSpeed(2.0f, 2.0f);

				_pipeline->isAnimation = true;
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

				DEVICECONTEXT->VSSetConstantBuffers(7, 1, blendBuffer->GetConstantBuffer().GetAddressOf());
				DEVICECONTEXT->VSSetShaderResources(3, 1, model->GetAnimationTextureBuffer().GetAddressOf());

				//model->SetTBuffer();
			}

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
			DEVICECONTEXT->VSSetConstantBuffers(5, 1, boneBuffer->GetConstantBuffer().GetAddressOf());
			DEVICECONTEXT->VSSetConstantBuffers(1, 1, gameObject->GetTransformBuffer()->GetConstantBuffer().GetAddressOf());

			const auto& meshes = model->GetMeshes();
			for (auto& mesh : meshes)
			{
				if (mesh->material)
				{
					_pipeline->SetNormalMap(mesh->material->GetNormalMap());
					_pipeline->SetSpecularMap(mesh->material->GetSpecularMap());
					_pipeline->SetDiffuseMap(mesh->material->GetDiffuseMap());

				}

				// BoneIndex
				BoneIndex boneIndex;
				boneIndex.index = mesh->boneIndex;
				shared_ptr<Buffer> boneIndexBuffer = make_shared<Buffer>();
				boneIndexBuffer->CreateConstantBuffer<BoneIndex>();
				boneIndexBuffer->CopyData(boneIndex);
				DEVICECONTEXT->VSSetConstantBuffers(6, 1, boneIndexBuffer->GetConstantBuffer().GetAddressOf());

				// Light
				LightAndCameraPos lightDirection;
				lightDirection.lightPosition = SCENE.GetActiveScene()->FindWithComponent(ComponentType::Light)->transform()->GetWorldPosition();
				lightDirection.cameraPosition = SCENE.GetActiveScene()->FindWithComponent(ComponentType::Camera)->transform()->GetWorldPosition();

				shared_ptr<Buffer> light = make_shared<Buffer>();
				light->CreateConstantBuffer<LightAndCameraPos>();
				light->CopyData(lightDirection);
				DEVICECONTEXT->PSSetConstantBuffers(4, 1, light->GetConstantBuffer().GetAddressOf());


				_pipeline->SetBuffer(mesh->GetBuffer());
				_pipeline->SetIndicesSize(mesh->GetGeometry()->GetIndices().size());
				_pipeline->SetShader(mesh->material->GetVertexShader(), mesh->material->GetPixelShader());

				_pipelineInfo = _pipeline->GetPipelineInfo();
				_pipelineInfo.blendState = make_shared<BlendState>();
				_pipelineInfo.blendState->CreateBlendState();
				_pipelineInfo.inputLayout = mesh->material->GetVertexShader()->GetInputLayout();
				_pipelineInfo.rasterizerState = make_shared<RasterizerState>();
				_pipelineInfo.rasterizerState->CreateRasterizerState(meshRenderer->GetRasterzerStates());

				_pipelineInfo.samplerState = make_shared<SamplerState>();
				_pipelineInfo.samplerState->CreateSamplerState();
				_pipeline->UpdatePipeline(_pipelineInfo);

				
			}
		}
		
	}
	
	
}

void RenderManager::ClearRenderObject()
{
	_renderObjects.clear();
}
