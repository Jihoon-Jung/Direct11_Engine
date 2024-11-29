#include "pch.h"
#include "RenderPass.h"
#include <fstream>
#include "MeshRenderer.h"
#include "MathHelper.h"

RenderPass::RenderPass()
{
}

RenderPass::~RenderPass()
{
}

void RenderPass::Render(bool isEnv)
{
	_transformPtr = _transform.lock();

	if (_pass == Pass::DEFAULT_RENDER)
		DefaultRender(isEnv);
	else if (_pass == Pass::GAUSSIANBLUR_RENDER)
		GaussianBlurRender(isEnv);
	else if (_pass == Pass::OUTLINE_RENDER)
		OutlineRender(isEnv);
	else if (_pass == Pass::QUAD_RENDER)
		QuadRender(isEnv);
	else if (_pass == Pass::TESSELLATION_RENDER)
		TessellationRender(isEnv);
	else if (_pass == Pass::TERRAIN_RENDER)
		TerrainRender(isEnv);
	else if (_pass == Pass::ENVIRONMENTMAP_RENDER)
		EnvironmentMapRender();
	else if (_pass == Pass::STATIC_MESH_RENDER)
		StaticMeshRencer(isEnv);
	else if (_pass == Pass::ANIMATED_MESH_RENDER)
		AnimatedMeshRender(isEnv);
	else if (_pass == Pass::PARTICLE_RENDER)
		ParticleRender(isEnv);
	else if (_pass == Pass::DEBUG_2D_RENDER)
		DebugQuadRender();
}

void RenderPass::DefaultRender(bool isEnv)
{
	shared_ptr<GameObject> cameraObject = SCENE.GetActiveScene()->Find(L"MainCamera");
	shared_ptr<Shader> shader = _meshRenderer->GetMaterial()->GetShader();

	if (_transformPtr)
		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"TransformBuffer", 1, _transformPtr->GetTransformBuffer());
	else
		assert(0);

	if (isEnv)
		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"CameraBuffer", 1, cameraObject->GetEnvCameraBuffer());
	else if (RENDER.GetShadowMapFlag())
		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"CameraBuffer", 1, cameraObject->GetShadowCameraBuffer());
	else
	{
		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"CameraBuffer", 1, cameraObject->GetCameraBuffer());
		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightSpaceTransform", 1, cameraObject->GetShadowCameraBuffer());
	}
		

	shared_ptr<GameObject> lightObject = SCENE.GetActiveScene()->Find(L"MainLight");
	if (lightObject != nullptr)
	{
		lightObject->GetLightBuffer();
		shader->PushConstantBufferToShader(ShaderType::PIXEL_SHADER, L"LightDesc", 1, lightObject->GetLightBuffer());
	}

	shader->PushConstantBufferToShader(ShaderType::PIXEL_SHADER, L"LightMaterial", 1, _meshRenderer->GetMaterialBuffer());

	LightAndCameraPos lightDirection;
	lightDirection.lightPosition = GP.centerPos - lightObject->transform()->GetWorldPosition();
	lightDirection.cameraPosition = cameraObject->transform()->GetWorldPosition();
	Vec3 pos = cameraObject->transform()->GetLocalPosition();

	shared_ptr<Buffer> light = make_shared<Buffer>();
	light->CreateConstantBuffer<LightAndCameraPos>();
	light->CopyData(lightDirection);

	shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightAndCameraPos", 1, light);



	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	shared_ptr<Buffer> buffer = _meshRenderer->GetMesh()->GetBuffer();
	uint32 stride = buffer->GetStride();
	uint32 offset = 0;

	shared_ptr<InputLayout> inputLayout = shader->GetInputLayout();
	shared_ptr<RasterizerState> rasterizerState = make_shared<RasterizerState>();

	RasterizerStateInfo rasterzerStateInfo = _meshRenderer->GetRasterzerStates();
	/*if (isUseTessellation)
		rasterzerStateInfo.fillMode = D3D11_FILL_WIREFRAME;*/
	rasterizerState->CreateRasterizerState(rasterzerStateInfo);

	// inputAssembler
	DEVICECONTEXT->IASetVertexBuffers(0, 1, buffer->GetVertexBuffer().GetAddressOf(), &stride, &offset);
	DEVICECONTEXT->IASetIndexBuffer(buffer->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

	DEVICECONTEXT->IASetInputLayout(shader->GetInputLayout()->GetInputLayout().Get());
	DEVICECONTEXT->IASetPrimitiveTopology(topology);

	// VertexShader
	ComPtr<ID3D11VertexShader> vertexShader = shader->GetVertexShader();
	ComPtr<ID3D11PixelShader> pixelShader = shader->GetPixelShader();

	if (vertexShader != nullptr)
		DEVICECONTEXT->VSSetShader(vertexShader.Get(), nullptr, 0);

	if (pixelShader != nullptr)
		DEVICECONTEXT->PSSetShader(pixelShader.Get(), nullptr, 0);


	// Rasterizer
	DEVICECONTEXT->RSSetState(rasterizerState->GetRasterizerState().Get());

	shared_ptr<Texture> defaultTexture = _meshRenderer->GetMaterial()->GetTexture();
	shared_ptr<Texture> normalMap = _meshRenderer->GetMaterial()->GetNormalMap();
	shared_ptr<Texture> specularMap = _meshRenderer->GetMaterial()->GetSpecularMap();
	shared_ptr<Texture> diffuseMap = _meshRenderer->GetMaterial()->GetDiffuseMap();

	// Set Default Texture
	if (defaultTexture != nullptr)
		shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"texture0", 1, defaultTexture->GetShaderResourceView());
		
	// Set NormalMap
	if (normalMap != nullptr)
		shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"normalMap", 1, normalMap->GetShaderResourceView());

	// Set SpecularMap
	if (specularMap != nullptr)
		shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"specularMap", 1, specularMap->GetShaderResourceView());

	// Set DiffuseMap
	if (diffuseMap != nullptr)
		shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"diffuseMap", 1, diffuseMap->GetShaderResourceView());

	if (!RENDER.GetShadowMapFlag())
		shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"shadowMap", 1, GP.GetShadowMapSRV());

	shared_ptr<DepthStencilState> depthStencilState = make_shared<DepthStencilState>();
	depthStencilState->SetDepthStencilState(DSState::NORMAL);

	shared_ptr<SamplerState> samplerState = make_shared<SamplerState>();
	samplerState->CreateSamplerState();

	shared_ptr<SamplerState> shadow_SamplerState = make_shared<SamplerState>();
	shadow_SamplerState->CreateShadowSamplerState();

	shared_ptr<BlendState> blendState = make_shared<BlendState>();
	blendState->CreateBlendState();

	DEVICECONTEXT->PSSetSamplers(0, 1, samplerState->GetSamplerState().GetAddressOf());
	DEVICECONTEXT->PSSetSamplers(1, 1, shadow_SamplerState->GetSamplerState().GetAddressOf());

	// OutputMerger
	DEVICECONTEXT->OMSetBlendState(blendState->GetBlendState().Get(), nullptr, 0xFFFFFFFF);
	DEVICECONTEXT->OMSetDepthStencilState(depthStencilState->GetDepthStecilState().Get(), 1);
	DEVICECONTEXT->DrawIndexed(_meshRenderer->GetMesh()->GetGeometry()->GetIndices().size(), 0, 0);

}

void RenderPass::EnvironmentMapRender()
{
	shared_ptr<GameObject> cameraObject = SCENE.GetActiveScene()->Find(L"MainCamera");
	shared_ptr<Shader> shader = RESOURCE.GetResource<Shader>(L"EnvironmentMap_Shader");

	if (_transformPtr)
		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"TransformBuffer", 1, _transformPtr->GetTransformBuffer());
	else
		assert(0);
	/*if (isEnv)
		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"CameraBuffer", 1, cameraObject->GetEnvCameraBuffer());
	else*/
	if (RENDER.GetShadowMapFlag())
		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"CameraBuffer", 1, cameraObject->GetShadowCameraBuffer());
	else
	{
		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"CameraBuffer", 1, cameraObject->GetCameraBuffer());
		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightSpaceTransform", 1, cameraObject->GetShadowCameraBuffer());
	}

	shared_ptr<GameObject> lightObject = SCENE.GetActiveScene()->Find(L"MainLight");
	if (lightObject != nullptr)
	{
		lightObject->GetLightBuffer();
		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightDesc", 1, lightObject->GetLightBuffer());
	}

	shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightMaterial", 1, _meshRenderer->GetMaterialBuffer());

	LightAndCameraPos lightDirection;
	lightDirection.lightPosition = GP.centerPos - lightObject->transform()->GetWorldPosition();
	lightDirection.cameraPosition = cameraObject->transform()->GetWorldPosition();
	Vec3 pos = cameraObject->transform()->GetLocalPosition();

	shared_ptr<Buffer> light = make_shared<Buffer>();
	light->CreateConstantBuffer<LightAndCameraPos>();
	light->CopyData(lightDirection);

	shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightAndCameraPos", 1, light);



	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	shared_ptr<Buffer> buffer = _meshRenderer->GetMesh()->GetBuffer();
	uint32 stride = buffer->GetStride();
	uint32 offset = 0;

	shared_ptr<InputLayout> inputLayout = shader->GetInputLayout();
	shared_ptr<RasterizerState> rasterizerState = make_shared<RasterizerState>();

	RasterizerStateInfo rasterzerStateInfo = _meshRenderer->GetRasterzerStates();
	/*if (isUseTessellation)
		rasterzerStateInfo.fillMode = D3D11_FILL_WIREFRAME;*/
	rasterizerState->CreateRasterizerState(rasterzerStateInfo);

	// inputAssembler
	DEVICECONTEXT->IASetVertexBuffers(0, 1, buffer->GetVertexBuffer().GetAddressOf(), &stride, &offset);
	DEVICECONTEXT->IASetIndexBuffer(buffer->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

	DEVICECONTEXT->IASetInputLayout(shader->GetInputLayout()->GetInputLayout().Get());
	DEVICECONTEXT->IASetPrimitiveTopology(topology);

	// VertexShader
	ComPtr<ID3D11VertexShader> vertexShader = shader->GetVertexShader();
	ComPtr<ID3D11PixelShader> pixelShader = shader->GetPixelShader();

	if (vertexShader != nullptr)
		DEVICECONTEXT->VSSetShader(vertexShader.Get(), nullptr, 0);

	if (pixelShader != nullptr)
		DEVICECONTEXT->PSSetShader(pixelShader.Get(), nullptr, 0);


	// Rasterizer
	DEVICECONTEXT->RSSetState(rasterizerState->GetRasterizerState().Get());

	shared_ptr<Texture> normalMap = _meshRenderer->GetMaterial()->GetNormalMap();
	shared_ptr<Texture> specularMap = _meshRenderer->GetMaterial()->GetSpecularMap();
	shared_ptr<Texture> diffuseMap = _meshRenderer->GetMaterial()->GetDiffuseMap();

	// Set Default Texture
	if (_envTexture != nullptr)
		shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"texture0", 1, _envTexture->GetShaderResourceView());

	// Set NormalMap
	if (normalMap != nullptr)
		shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"normalMap", 1, normalMap->GetShaderResourceView());

	// Set SpecularMap
	if (specularMap != nullptr)
		shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"specularMap", 1, specularMap->GetShaderResourceView());

	// Set DiffuseMap
	if (diffuseMap != nullptr)
		shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"diffuseMap", 1, diffuseMap->GetShaderResourceView());


	shared_ptr<DepthStencilState> depthStencilState = make_shared<DepthStencilState>();
	depthStencilState->SetDepthStencilState(_dsStateType);

	shared_ptr<SamplerState> samplerState = make_shared<SamplerState>();
	samplerState->CreateSamplerState();

	shared_ptr<SamplerState> shadow_SamplerState = make_shared<SamplerState>();
	shadow_SamplerState->CreateShadowSamplerState();

	shared_ptr<BlendState> blendState = make_shared<BlendState>();
	blendState->CreateBlendState();

	DEVICECONTEXT->PSSetSamplers(0, 1, samplerState->GetSamplerState().GetAddressOf());
	DEVICECONTEXT->PSSetSamplers(1, 1, shadow_SamplerState->GetSamplerState().GetAddressOf());

	// OutputMerger
	DEVICECONTEXT->OMSetBlendState(blendState->GetBlendState().Get(), nullptr, 0xFFFFFFFF);
	DEVICECONTEXT->OMSetDepthStencilState(depthStencilState->GetDepthStecilState().Get(), 1);
	DEVICECONTEXT->DrawIndexed(_meshRenderer->GetMesh()->GetGeometry()->GetIndices().size(), 0, 0);
}

void RenderPass::TessellationRender(bool isEnv)
{
	shared_ptr<GameObject> cameraObject = SCENE.GetActiveScene()->Find(L"MainCamera");
	shared_ptr<Shader> shader = _meshRenderer->GetMaterial()->GetShader();
	if (_transformPtr)
		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"TransformBuffer", 1, _transformPtr->GetTransformBuffer());
	else
		assert(0);

	if (isEnv)
		shader->PushConstantBufferToShader(ShaderType::DOMAIN_SHADER, L"CameraBuffer", 1, cameraObject->GetEnvCameraBuffer());
	else if (RENDER.GetShadowMapFlag())
		shader->PushConstantBufferToShader(ShaderType::DOMAIN_SHADER, L"CameraBuffer", 1, cameraObject->GetShadowCameraBuffer());
	else
	{
		shader->PushConstantBufferToShader(ShaderType::DOMAIN_SHADER, L"CameraBuffer", 1, cameraObject->GetCameraBuffer());
		shader->PushConstantBufferToShader(ShaderType::DOMAIN_SHADER, L"LightSpaceTransform", 1, cameraObject->GetShadowCameraBuffer());
	}
		

	shared_ptr<GameObject> lightObject = SCENE.GetActiveScene()->Find(L"MainLight");
	if (lightObject != nullptr)
	{
		lightObject->GetLightBuffer();
		shader->PushConstantBufferToShader(ShaderType::PIXEL_SHADER, L"LightDesc", 1, lightObject->GetLightBuffer());
	}

	shader->PushConstantBufferToShader(ShaderType::PIXEL_SHADER, L"LightMaterial", 1, _meshRenderer->GetMaterialBuffer());

	LightAndCameraPos lightDirection;
	lightDirection.lightPosition = GP.centerPos - lightObject->transform()->GetWorldPosition();
	lightDirection.cameraPosition = cameraObject->transform()->GetWorldPosition();
	Vec3 pos = cameraObject->transform()->GetLocalPosition();

	shared_ptr<Buffer> light = make_shared<Buffer>();
	light->CreateConstantBuffer<LightAndCameraPos>();
	light->CopyData(lightDirection);

	shader->PushConstantBufferToShader(ShaderType::DOMAIN_SHADER, L"LightAndCameraPos", 1, light);
	shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightAndCameraPos", 1, light);
	shader->PushConstantBufferToShader(ShaderType::PIXEL_SHADER, L"LightAndCameraPos", 1, light);

	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;

	shared_ptr<Buffer> buffer = _meshRenderer->GetMesh()->GetBuffer();
	uint32 stride = buffer->GetStride();
	uint32 offset = 0;

	shared_ptr<InputLayout> inputLayout = shader->GetInputLayout();
	shared_ptr<RasterizerState> rasterizerState = make_shared<RasterizerState>();

	RasterizerStateInfo rasterzerStateInfo = _meshRenderer->GetRasterzerStates();

	rasterizerState->CreateRasterizerState(rasterzerStateInfo);

	// inputAssembler
	DEVICECONTEXT->IASetVertexBuffers(0, 1, buffer->GetVertexBuffer().GetAddressOf(), &stride, &offset);
	DEVICECONTEXT->IASetIndexBuffer(buffer->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

	DEVICECONTEXT->IASetInputLayout(shader->GetInputLayout()->GetInputLayout().Get());
	DEVICECONTEXT->IASetPrimitiveTopology(topology);

	// VertexShader
	ComPtr<ID3D11VertexShader> vertexShader = shader->GetVertexShader();
	ComPtr<ID3D11PixelShader> pixelShader = shader->GetPixelShader();
	ComPtr<ID3D11HullShader> hullShader = shader->GetHullShader();
	ComPtr<ID3D11DomainShader> domainShader = shader->GetDomainShader();

	if (vertexShader != nullptr)
		DEVICECONTEXT->VSSetShader(vertexShader.Get(), nullptr, 0);

	if (pixelShader != nullptr)
		DEVICECONTEXT->PSSetShader(pixelShader.Get(), nullptr, 0);

	if (hullShader != nullptr)
		DEVICECONTEXT->HSSetShader(hullShader.Get(), nullptr, 0);

	if (domainShader != nullptr)
		DEVICECONTEXT->DSSetShader(domainShader.Get(), nullptr, 0);

	// Rasterizer
	DEVICECONTEXT->RSSetState(rasterizerState->GetRasterizerState().Get());

	shared_ptr<Texture> defaultTexture = _meshRenderer->GetMaterial()->GetTexture();
	shared_ptr<Texture> normalMap = _meshRenderer->GetMaterial()->GetNormalMap();
	shared_ptr<Texture> specularMap = _meshRenderer->GetMaterial()->GetSpecularMap();
	shared_ptr<Texture> diffuseMap = _meshRenderer->GetMaterial()->GetDiffuseMap();
	// Set Default Texture
	if (defaultTexture != nullptr)
	{
		shader->PushShaderResourceToShader(ShaderType::DOMAIN_SHADER, L"texture0", 1, defaultTexture->GetShaderResourceView());
	}

	// Set NormalMap
	if (normalMap != nullptr)
	{
		shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"normalMap", 1, normalMap->GetShaderResourceView());
		shader->PushShaderResourceToShader(ShaderType::DOMAIN_SHADER, L"normalMap", 1, normalMap->GetShaderResourceView());
	}

	// Set SpecularMap
	if (specularMap != nullptr)
		shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"specularMap", 1, specularMap->GetShaderResourceView());

	// Set DiffuseMap
	if (diffuseMap != nullptr)
		shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"diffuseMap", 1, diffuseMap->GetShaderResourceView());


	shared_ptr<DepthStencilState> depthStencilState = make_shared<DepthStencilState>();
	depthStencilState->SetDepthStencilState(_dsStateType);

	shared_ptr<SamplerState> samplerState = make_shared<SamplerState>();
	samplerState->CreateSamplerState();

	shared_ptr<SamplerState> shadow_SamplerState = make_shared<SamplerState>();
	shadow_SamplerState->CreateShadowSamplerState();

	shared_ptr<BlendState> blendState = make_shared<BlendState>();
	blendState->CreateBlendState();

	DEVICECONTEXT->PSSetSamplers(0, 1, samplerState->GetSamplerState().GetAddressOf());
	DEVICECONTEXT->PSSetSamplers(1, 1, shadow_SamplerState->GetSamplerState().GetAddressOf());

	// OutputMerger
	DEVICECONTEXT->OMSetBlendState(blendState->GetBlendState().Get(), nullptr, 0xFFFFFFFF);
	DEVICECONTEXT->OMSetDepthStencilState(depthStencilState->GetDepthStecilState().Get(), 1);
	DEVICECONTEXT->DrawIndexed(_meshRenderer->GetMesh()->GetGeometry()->GetIndices().size(), 0, 0);

	DEVICECONTEXT->HSSetShader(nullptr, nullptr, 0);
	DEVICECONTEXT->DSSetShader(nullptr, nullptr, 0);

}

void RenderPass::OutlineRender(bool isEnv)
{
	Vec3 firstObjectScale = _transformPtr->GetLocalScale();
	Vec3 secondObjectScale = firstObjectScale * 1.1f;
	DSState firstObjectDStype = _dsStateType;
	shared_ptr<Material> firstObjectMaterial = _meshRenderer->GetMaterial();
	for (int i = 0; i < 2; i++)
	{
		if (i > 0)
		{
			_transformPtr->SetLocalScale(secondObjectScale);
			_dsStateType = DSState::CUSTOM2;
			_meshRenderer->SetMaterial(RESOURCE.GetResource<Material>(L"SimpleMaterial"));
		}
			

		shared_ptr<GameObject> cameraObject = SCENE.GetActiveScene()->Find(L"MainCamera");
		
		shared_ptr<Shader> shader = _meshRenderer->GetMaterial()->GetShader();
		shader = _meshRenderer->GetMaterial()->GetShader();
		if (_transformPtr)
			shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"TransformBuffer", 1, _transformPtr->GetTransformBuffer());
		else
			assert(0);

		if (isEnv)
			shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"CameraBuffer", 1, cameraObject->GetEnvCameraBuffer());
		else if (RENDER.GetShadowMapFlag())
			shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"CameraBuffer", 1, cameraObject->GetShadowCameraBuffer());
		else
		{
			shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"CameraBuffer", 1, cameraObject->GetCameraBuffer());
			shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightSpaceTransform", 1, cameraObject->GetShadowCameraBuffer());
		}


		shared_ptr<GameObject> lightObject = SCENE.GetActiveScene()->Find(L"MainLight");
		if (lightObject != nullptr)
		{
			lightObject->GetLightBuffer();
			shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightDesc", 1, lightObject->GetLightBuffer());
		}

		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightMaterial", 1, _meshRenderer->GetMaterialBuffer());

		LightAndCameraPos lightDirection;
		lightDirection.lightPosition = GP.centerPos - lightObject->transform()->GetWorldPosition();
		lightDirection.cameraPosition = cameraObject->transform()->GetWorldPosition();
		Vec3 pos = cameraObject->transform()->GetLocalPosition();

		shared_ptr<Buffer> light = make_shared<Buffer>();
		light->CreateConstantBuffer<LightAndCameraPos>();
		light->CopyData(lightDirection);

		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightAndCameraPos", 1, light);



		D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		shared_ptr<Buffer> buffer = _meshRenderer->GetMesh()->GetBuffer();
		uint32 stride = buffer->GetStride();
		uint32 offset = 0;

		shared_ptr<InputLayout> inputLayout = shader->GetInputLayout();
		shared_ptr<RasterizerState> rasterizerState = make_shared<RasterizerState>();

		RasterizerStateInfo rasterzerStateInfo = _meshRenderer->GetRasterzerStates();
		/*if (isUseTessellation)
			rasterzerStateInfo.fillMode = D3D11_FILL_WIREFRAME;*/
		rasterizerState->CreateRasterizerState(rasterzerStateInfo);

		// inputAssembler
		DEVICECONTEXT->IASetVertexBuffers(0, 1, buffer->GetVertexBuffer().GetAddressOf(), &stride, &offset);
		DEVICECONTEXT->IASetIndexBuffer(buffer->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

		DEVICECONTEXT->IASetInputLayout(shader->GetInputLayout()->GetInputLayout().Get());
		DEVICECONTEXT->IASetPrimitiveTopology(topology);

		// VertexShader
		ComPtr<ID3D11VertexShader> vertexShader = shader->GetVertexShader();
		ComPtr<ID3D11PixelShader> pixelShader = shader->GetPixelShader();
		ComPtr<ID3D11HullShader> hullShader = shader->GetHullShader();
		ComPtr<ID3D11DomainShader> domainShader = shader->GetDomainShader();

		if (vertexShader != nullptr)
			DEVICECONTEXT->VSSetShader(vertexShader.Get(), nullptr, 0);

		if (pixelShader != nullptr)
			DEVICECONTEXT->PSSetShader(pixelShader.Get(), nullptr, 0);


		// Rasterizer
		DEVICECONTEXT->RSSetState(rasterizerState->GetRasterizerState().Get());

		shared_ptr<Texture> defaultTexture = _meshRenderer->GetMaterial()->GetTexture();
		shared_ptr<Texture> normalMap = _meshRenderer->GetMaterial()->GetNormalMap();
		shared_ptr<Texture> specularMap = _meshRenderer->GetMaterial()->GetSpecularMap();
		shared_ptr<Texture> diffuseMap = _meshRenderer->GetMaterial()->GetDiffuseMap();
		// Set Default Texture
		if (i > 0)
			shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"texture0", 1, RESOURCE.GetResource<Texture>(L"Yellow")->GetShaderResourceView());
		else
		{
			if (defaultTexture != nullptr)
				shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"texture0", 1, defaultTexture->GetShaderResourceView());
		}

		

		// Set NormalMap
		if (normalMap != nullptr)
			shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"normalMap", 1, normalMap->GetShaderResourceView());

		// Set SpecularMap
		if (specularMap != nullptr)
			shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"specularMap", 1, specularMap->GetShaderResourceView());

		// Set DiffuseMap
		if (diffuseMap != nullptr)
			shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"diffuseMap", 1, diffuseMap->GetShaderResourceView());


		shared_ptr<DepthStencilState> depthStencilState = make_shared<DepthStencilState>();
		depthStencilState->SetDepthStencilState(_dsStateType);

		shared_ptr<SamplerState> samplerState = make_shared<SamplerState>();
		samplerState->CreateSamplerState();

		shared_ptr<SamplerState> shadow_SamplerState = make_shared<SamplerState>();
		shadow_SamplerState->CreateShadowSamplerState();

		shared_ptr<BlendState> blendState = make_shared<BlendState>();
		blendState->CreateBlendState();

		DEVICECONTEXT->PSSetSamplers(0, 1, samplerState->GetSamplerState().GetAddressOf());
		DEVICECONTEXT->PSSetSamplers(1, 1, shadow_SamplerState->GetSamplerState().GetAddressOf());

		// OutputMerger
		DEVICECONTEXT->OMSetBlendState(blendState->GetBlendState().Get(), nullptr, 0xFFFFFFFF);
		DEVICECONTEXT->OMSetDepthStencilState(depthStencilState->GetDepthStecilState().Get(), 1);
		DEVICECONTEXT->DrawIndexed(_meshRenderer->GetMesh()->GetGeometry()->GetIndices().size(), 0, 0);
	}
	
	// Restore
	_transformPtr->SetLocalScale(firstObjectScale);
	_dsStateType = firstObjectDStype;
	_meshRenderer->SetMaterial(firstObjectMaterial);
}

void RenderPass::GaussianBlurRender(bool isEnv)
{
	SetRenderTarget(GWinSizeX, GWinSizeY);
	DefaultRender(isEnv);
	_outputSRV = _offscreenSRV;

	//SaveRenderTargetToFile(_offscreenRTV.Get(), "output.dump", GWinSizeX, GWinSizeY);

	shared_ptr<Material> material = make_shared<Material>();
	
	shared_ptr<Shader> computeShader_gaussianBlurHorizontal = RESOURCE.GetResource<Shader>(L"Gaussian_Horizontal");
	shared_ptr<Shader> computeShader_gaussianBlurVertical = RESOURCE.GetResource<Shader>(L"Gaussian_Vertical");
	_outputSRV = material->GaussainBlur(computeShader_gaussianBlurVertical, computeShader_gaussianBlurHorizontal, _offscreenSRV);
}

void RenderPass::QuadRender(bool isEnv)
{
	GP.RestoreRenderTarget();

	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	shared_ptr<Buffer> buffer = _meshRenderer->GetMesh()->GetBuffer();
	uint32 stride = buffer->GetStride();
	uint32 offset = 0;

	shared_ptr<InputLayout> inputLayout = _meshRenderer->GetShader()->GetInputLayout();
	shared_ptr<RasterizerState> rasterizerState = make_shared<RasterizerState>();

	RasterizerStateInfo rasterzerStates;
	rasterzerStates.fillMode = D3D11_FILL_SOLID;
	rasterzerStates.cullMode = D3D11_CULL_BACK;
	rasterzerStates.frontCounterClockwise = false;
	rasterizerState->CreateRasterizerState(rasterzerStates);

	// inputAssembler
	DEVICECONTEXT->IASetVertexBuffers(0, 1, buffer->GetVertexBuffer().GetAddressOf(), &stride, &offset);
	DEVICECONTEXT->IASetIndexBuffer(buffer->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

	DEVICECONTEXT->IASetInputLayout(_meshRenderer->GetShader()->GetInputLayout()->GetInputLayout().Get());
	DEVICECONTEXT->IASetPrimitiveTopology(topology);

	// VertexShader
	DEVICECONTEXT->VSSetShader(_meshRenderer->GetShader()->GetVertexShader().Get(), nullptr, 0);

	// Rasterizer
	DEVICECONTEXT->RSSetState(rasterizerState->GetRasterizerState().Get());

	// PixelShader
	DEVICECONTEXT->PSSetShader(_meshRenderer->GetShader()->GetPixelShader().Get(), nullptr, 0);


	// Set Default Texture
	_meshRenderer->GetShader()->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"texture0", 1, _inputSRV);

	shared_ptr<DepthStencilState> depthStencilState = make_shared<DepthStencilState>();
	depthStencilState->SetDepthStencilState(DSState::NORMAL);

	shared_ptr<SamplerState> samplerState = make_shared<SamplerState>();
	samplerState->CreateSamplerState();

	shared_ptr<BlendState> blendState = make_shared<BlendState>();
	blendState->CreateBlendState();

	DEVICECONTEXT->PSSetSamplers(0, 1, samplerState->GetSamplerState().GetAddressOf());

	// OutputMerger
	DEVICECONTEXT->OMSetBlendState(blendState->GetBlendState().Get(), nullptr, 0xFFFFFFFF);
	DEVICECONTEXT->OMSetDepthStencilState(depthStencilState->GetDepthStecilState().Get(), 1);
	DEVICECONTEXT->DrawIndexed(_meshRenderer->GetMesh()->GetGeometry()->GetIndices().size(), 0, 0);
}

void RenderPass::TerrainRender(bool isEnv)
{
	shared_ptr<GameObject> cameraObject = SCENE.GetActiveScene()->Find(L"MainCamera");
	shared_ptr<Shader> shader = _meshRenderer->GetMaterial()->GetShader();
	if (_transformPtr)
		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"TransformBuffer", 1, _transformPtr->GetTransformBuffer());
	else
		assert(0);

	if (isEnv)
		shader->PushConstantBufferToShader(ShaderType::DOMAIN_SHADER, L"CameraBuffer", 1, cameraObject->GetEnvCameraBuffer());
	else if (RENDER.GetShadowMapFlag())
		shader->PushConstantBufferToShader(ShaderType::DOMAIN_SHADER, L"CameraBuffer", 1, cameraObject->GetShadowCameraBuffer());
	else
	{
		shader->PushConstantBufferToShader(ShaderType::DOMAIN_SHADER, L"CameraBuffer", 1, cameraObject->GetCameraBuffer());
		shader->PushConstantBufferToShader(ShaderType::DOMAIN_SHADER, L"LightSpaceTransform", 1, cameraObject->GetShadowCameraBuffer());
	}
		

	shared_ptr<GameObject> lightObject = SCENE.GetActiveScene()->Find(L"MainLight");
	if (lightObject != nullptr)
	{
		lightObject->GetLightBuffer();
		shader->PushConstantBufferToShader(ShaderType::PIXEL_SHADER, L"LightDesc", 1, lightObject->GetLightBuffer());
	}

	shader->PushConstantBufferToShader(ShaderType::PIXEL_SHADER, L"LightMaterial", 1, _meshRenderer->GetMaterialBuffer());

	LightAndCameraPos lightDirection;
	lightDirection.lightPosition = GP.centerPos - lightObject->transform()->GetWorldPosition();
	lightDirection.cameraPosition = cameraObject->transform()->GetWorldPosition();
	Vec3 pos = cameraObject->transform()->GetLocalPosition();

	shared_ptr<Buffer> light = make_shared<Buffer>();
	light->CreateConstantBuffer<LightAndCameraPos>();
	light->CopyData(lightDirection);

	shader->PushConstantBufferToShader(ShaderType::HULL_SHADER, L"LightAndCameraPos", 1, light);
	shader->PushConstantBufferToShader(ShaderType::PIXEL_SHADER, L"LightAndCameraPos", 1, light);



	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;

	shared_ptr<Buffer> buffer = _meshRenderer->GetMesh()->GetBuffer();
	uint32 stride = buffer->GetStride();
	uint32 offset = 0;

	shared_ptr<InputLayout> inputLayout = shader->GetInputLayout();
	shared_ptr<RasterizerState> rasterizerState = make_shared<RasterizerState>();

	RasterizerStateInfo rasterzerStateInfo = _meshRenderer->GetRasterzerStates();
	//rasterzerStateInfo.fillMode = D3D11_FILL_WIREFRAME;
	/*if (isUseTessellation)
		rasterzerStateInfo.fillMode = D3D11_FILL_WIREFRAME;*/
	rasterizerState->CreateRasterizerState(rasterzerStateInfo);

	// inputAssembler
	DEVICECONTEXT->IASetVertexBuffers(0, 1, buffer->GetVertexBuffer().GetAddressOf(), &stride, &offset);
	DEVICECONTEXT->IASetIndexBuffer(buffer->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

	DEVICECONTEXT->IASetInputLayout(shader->GetInputLayout()->GetInputLayout().Get());
	DEVICECONTEXT->IASetPrimitiveTopology(topology);

	// VertexShader
	ComPtr<ID3D11VertexShader> vertexShader = shader->GetVertexShader();
	ComPtr<ID3D11PixelShader> pixelShader = shader->GetPixelShader();
	ComPtr<ID3D11HullShader> hullShader = shader->GetHullShader();
	ComPtr<ID3D11DomainShader> domainShader = shader->GetDomainShader();

	if (vertexShader != nullptr)
		DEVICECONTEXT->VSSetShader(vertexShader.Get(), nullptr, 0);

	if (pixelShader != nullptr)
		DEVICECONTEXT->PSSetShader(pixelShader.Get(), nullptr, 0);

	if (hullShader != nullptr)
		DEVICECONTEXT->HSSetShader(hullShader.Get(), nullptr, 0);

	if (domainShader != nullptr)
		DEVICECONTEXT->DSSetShader(domainShader.Get(), nullptr, 0);

	// Rasterizer
	DEVICECONTEXT->RSSetState(rasterizerState->GetRasterizerState().Get());

	ComPtr<ID3D11ShaderResourceView> layerMapArray = _meshRenderer->GetMesh()->GetLayerMapArraySRV();	// PS
	ComPtr<ID3D11ShaderResourceView> blendMap = _meshRenderer->GetMesh()->GetBlendMapSRV();		// PS
	ComPtr<ID3D11ShaderResourceView> heightMap = _meshRenderer->GetMesh()->GetHeightMapSRV();		// VS, DS, PS


	shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"gLayerMapArray", 1, layerMapArray);
	shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"gBlendMap", 1, blendMap);
	shader->PushShaderResourceToShader(ShaderType::VERTEX_SHADER, L"gHeightMap", 1, heightMap);
	shader->PushShaderResourceToShader(ShaderType::DOMAIN_SHADER, L"gHeightMap", 1, heightMap);
	shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"gHeightMap", 1, heightMap);

	if (!RENDER.GetShadowMapFlag())
		shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"shadowMap", 1, GP.GetShadowMapSRV());

	Matrix viewMat = SCENE.GetActiveScene()->Find(L"MainCamera")->GetComponent<Camera>()->GetViewMatrix();
	Matrix projMat = SCENE.GetActiveScene()->Find(L"MainCamera")->GetComponent<Camera>()->GetProjectionMatrix();
	if (isEnv)
	{
		viewMat = SCENE.GetActiveScene()->Find(L"MainCamera")->GetComponent<Camera>()->GetEnvViewMatrix();
		projMat = SCENE.GetActiveScene()->Find(L"MainCamera")->GetComponent<Camera>()->GetEnvProjectionMatrix();
	}
		
	Matrix viewProj = viewMat * projMat;
	Vec4 frustom[6];
	MathHelper::ExtractFrustumPlanes(frustom, viewProj);

	TerrainBuffer terrainBufferInfo;
	terrainBufferInfo.minDist = 20.0f;
	terrainBufferInfo.maxDist = 500.0f;
	terrainBufferInfo.minTess = 0.0f;
	terrainBufferInfo.maxTess = 6.0f;
	terrainBufferInfo.texelCellSpaceU = (1.0 / _meshRenderer->GetMesh()->GetTerrainInfo().heightmapWidth);
	terrainBufferInfo.texelCellSpaceV = (1.0f / _meshRenderer->GetMesh()->GetTerrainInfo().heightmapHeight);
	terrainBufferInfo.worldCellSpace = _meshRenderer->GetMesh()->GetTerrainInfo().cellSpacing;
	terrainBufferInfo.texScale = 50.0f;
	memcpy(terrainBufferInfo.frustom, frustom, sizeof(frustom));

	shared_ptr<Buffer> terrainBuffer = make_shared<Buffer>();
	terrainBuffer->CreateConstantBuffer<TerrainBuffer>();
	terrainBuffer->CopyData(terrainBufferInfo);


	shader->PushConstantBufferToShader(ShaderType::HULL_SHADER, L"TerrainBuffer", 1, terrainBuffer);
	shader->PushConstantBufferToShader(ShaderType::PIXEL_SHADER, L"TerrainBuffer", 1, terrainBuffer);
	shader->PushConstantBufferToShader(ShaderType::DOMAIN_SHADER, L"TerrainBuffer", 1, terrainBuffer);

	shared_ptr<DepthStencilState> depthStencilState = make_shared<DepthStencilState>();
	depthStencilState->SetDepthStencilState(_dsStateType);

	shared_ptr<SamplerState> samplerState = make_shared<SamplerState>();
	samplerState->CreateSamplerState();
	shared_ptr<SamplerState> shadow_SamplerState = make_shared<SamplerState>();
	shadow_SamplerState->CreateShadowSamplerState();

	shared_ptr<SamplerState> heightMapSamplerState = make_shared<SamplerState>();
	heightMapSamplerState->CreateHeightMapSamplerState();

	shared_ptr<BlendState> blendState = make_shared<BlendState>();
	blendState->CreateBlendState();

	
	DEVICECONTEXT->VSSetSamplers(2, 1, heightMapSamplerState->GetSamplerState().GetAddressOf());
	DEVICECONTEXT->DSSetSamplers(2, 1, heightMapSamplerState->GetSamplerState().GetAddressOf());
	DEVICECONTEXT->PSSetSamplers(0, 1, samplerState->GetSamplerState().GetAddressOf());
	DEVICECONTEXT->PSSetSamplers(1, 1, shadow_SamplerState->GetSampleCmpState().GetAddressOf());
	DEVICECONTEXT->PSSetSamplers(2, 1, heightMapSamplerState->GetSamplerState().GetAddressOf());

	// OutputMerger
	DEVICECONTEXT->OMSetBlendState(blendState->GetBlendState().Get(), nullptr, 0xFFFFFFFF);
	DEVICECONTEXT->OMSetDepthStencilState(depthStencilState->GetDepthStecilState().Get(), 1);
	DEVICECONTEXT->DrawIndexed(_meshRenderer->GetMesh()->GetTerrainGeometry()->GetIndices().size(), 0, 0);

	DEVICECONTEXT->HSSetShader(nullptr, nullptr, 0);
	DEVICECONTEXT->DSSetShader(nullptr, nullptr, 0);
}

void RenderPass::StaticMeshRencer(bool isEnv)
{
	shared_ptr<Model> model = _meshRenderer->GetModel();
	shared_ptr<Shader> shader = _meshRenderer->GetShader();

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

	const auto& meshes = model->GetMeshes();
	for (auto& mesh : meshes)
	{
		shared_ptr<Texture> normalMap;
		shared_ptr<Texture> specularMap;
		shared_ptr<Texture> diffuseMap;

		if (mesh->material)
		{
			normalMap = mesh->material->GetNormalMap();
			specularMap = mesh->material->GetSpecularMap();
			diffuseMap = mesh->material->GetDiffuseMap();

		}

		// BoneIndex
		BoneIndex boneIndex;
		boneIndex.index = mesh->boneIndex;
		shared_ptr<Buffer> boneIndexBuffer = make_shared<Buffer>();
		boneIndexBuffer->CreateConstantBuffer<BoneIndex>();
		boneIndexBuffer->CopyData(boneIndex);
		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"BonIndex", 1, boneIndexBuffer);

		shared_ptr<GameObject> cameraObject = SCENE.GetActiveScene()->Find(L"MainCamera");

		if (_transformPtr)
			shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"TransformBuffer", 1, _transformPtr->GetTransformBuffer());
		else
			assert(0);

		if (isEnv)
			shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"CameraBuffer", 1, cameraObject->GetEnvCameraBuffer());
		else if (RENDER.GetShadowMapFlag())
			shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"CameraBuffer", 1, cameraObject->GetShadowCameraBuffer());
		else
		{
			shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"CameraBuffer", 1, cameraObject->GetCameraBuffer());
			shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightSpaceTransform", 1, cameraObject->GetShadowCameraBuffer());
		}

		shared_ptr<GameObject> lightObject = SCENE.GetActiveScene()->Find(L"MainLight");
		if (lightObject != nullptr)
		{
			lightObject->GetLightBuffer();
			shader->PushConstantBufferToShader(ShaderType::PIXEL_SHADER, L"LightDesc", 1, lightObject->GetLightBuffer());
		}

		shader->PushConstantBufferToShader(ShaderType::PIXEL_SHADER, L"LightMaterial", 1, _meshRenderer->GetMaterialBuffer());

		// Light
		LightAndCameraPos lightDirection;
		lightDirection.lightPosition = GP.centerPos - lightObject->transform()->GetWorldPosition();
		lightDirection.cameraPosition = cameraObject->transform()->GetWorldPosition();

		shared_ptr<Buffer> light = make_shared<Buffer>();
		light->CreateConstantBuffer<LightAndCameraPos>();
		light->CopyData(lightDirection);


		shader->PushConstantBufferToShader(ShaderType::PIXEL_SHADER, L"LightAndCameraPos", 1, light);



		D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		shared_ptr<Buffer> buffer = mesh->GetBuffer();
		uint32 stride = buffer->GetStride();
		uint32 offset = 0;

		shared_ptr<InputLayout> inputLayout = shader->GetInputLayout();
		shared_ptr<RasterizerState> rasterizerState = make_shared<RasterizerState>();

		RasterizerStateInfo rasterzerStateInfo = _meshRenderer->GetRasterzerStates();
		/*if (isUseTessellation)
			rasterzerStateInfo.fillMode = D3D11_FILL_WIREFRAME;*/
		rasterizerState->CreateRasterizerState(rasterzerStateInfo);

		// inputAssembler
		DEVICECONTEXT->IASetVertexBuffers(0, 1, buffer->GetVertexBuffer().GetAddressOf(), &stride, &offset);
		DEVICECONTEXT->IASetIndexBuffer(buffer->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

		DEVICECONTEXT->IASetInputLayout(shader->GetInputLayout()->GetInputLayout().Get());
		DEVICECONTEXT->IASetPrimitiveTopology(topology);

		// VertexShader
		ComPtr<ID3D11VertexShader> vertexShader = shader->GetVertexShader();
		ComPtr<ID3D11PixelShader> pixelShader = shader->GetPixelShader();

		normalMap = _meshRenderer->GetMaterial()->GetNormalMap();
		specularMap = _meshRenderer->GetMaterial()->GetSpecularMap();
		diffuseMap = _meshRenderer->GetMaterial()->GetDiffuseMap();


		// Set NormalMap
		if (normalMap != nullptr)
			shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"normalMap", 1, normalMap->GetShaderResourceView());

		// Set SpecularMap
		if (specularMap != nullptr)
			shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"specularMap", 1, specularMap->GetShaderResourceView());

		// Set DiffuseMap
		if (diffuseMap != nullptr)
			shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"diffuseMap", 1, diffuseMap->GetShaderResourceView());

		if (vertexShader != nullptr)
			DEVICECONTEXT->VSSetShader(vertexShader.Get(), nullptr, 0);

		if (pixelShader != nullptr)
			DEVICECONTEXT->PSSetShader(pixelShader.Get(), nullptr, 0);



		// Rasterizer
		DEVICECONTEXT->RSSetState(rasterizerState->GetRasterizerState().Get());

		shared_ptr<DepthStencilState> depthStencilState = make_shared<DepthStencilState>();
		depthStencilState->SetDepthStencilState(DSState::NORMAL);

		shared_ptr<SamplerState> samplerState = make_shared<SamplerState>();
		samplerState->CreateSamplerState();

		shared_ptr<SamplerState> shadow_SamplerState = make_shared<SamplerState>();
		shadow_SamplerState->CreateShadowSamplerState();

		shared_ptr<BlendState> blendState = make_shared<BlendState>();
		blendState->CreateBlendState();

		DEVICECONTEXT->PSSetSamplers(0, 1, samplerState->GetSamplerState().GetAddressOf());
		DEVICECONTEXT->PSSetSamplers(1, 1, shadow_SamplerState->GetSamplerState().GetAddressOf());

		// OutputMerger
		DEVICECONTEXT->OMSetBlendState(blendState->GetBlendState().Get(), nullptr, 0xFFFFFFFF);
		DEVICECONTEXT->OMSetDepthStencilState(depthStencilState->GetDepthStecilState().Get(), 1);
		DEVICECONTEXT->DrawIndexed(mesh->GetGeometry()->GetIndices().size(), 0, 0);
	}
}

void RenderPass::AnimatedMeshRender(bool isEnv)
{
	animationSumTime += TIME.GetDeltaTime() / 18.0f;
	_blendAnimDesc.curr.sumTime += TIME.GetDeltaTime() / 18.0f;
	_blendAnimDesc.SetAnimSpeed(2.0f, 2.0f);

	shared_ptr<Model> model = _meshRenderer->GetModel();
	shared_ptr<Shader> shader = _meshRenderer->GetShader();

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
	UINT animationTexture_Slot = shader->GetShaderSlot()->GetSlotNumber(L"TransformMap");
	shader->PushShaderResourceToShader(ShaderType::VERTEX_SHADER, L"TransformMap", 1, model->GetAnimationTextureBuffer());

	const auto& meshes = model->GetMeshes();
	for (auto& mesh : meshes)
	{
		shared_ptr<Texture> normalMap;
		shared_ptr<Texture> specularMap;
		shared_ptr<Texture> diffuseMap;

		if (mesh->material)
		{
			normalMap = mesh->material->GetNormalMap();
			specularMap = mesh->material->GetSpecularMap();
			diffuseMap = mesh->material->GetDiffuseMap();

		}

		shared_ptr<GameObject> cameraObject = SCENE.GetActiveScene()->Find(L"MainCamera");

		if (_transformPtr)
			shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"TransformBuffer", 1, _transformPtr->GetTransformBuffer());
		else
			assert(0);

		if (isEnv)
			shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"CameraBuffer", 1, cameraObject->GetEnvCameraBuffer());
		else if (RENDER.GetShadowMapFlag())
			shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"CameraBuffer", 1, cameraObject->GetShadowCameraBuffer());
		else
		{
			shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"CameraBuffer", 1, cameraObject->GetCameraBuffer());
			shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightSpaceTransform", 1, cameraObject->GetShadowCameraBuffer());
		}
			

		shared_ptr<GameObject> lightObject = SCENE.GetActiveScene()->Find(L"MainLight");
		if (lightObject != nullptr)
		{
			lightObject->GetLightBuffer();
			shader->PushConstantBufferToShader(ShaderType::PIXEL_SHADER, L"LightDesc", 1, lightObject->GetLightBuffer());
		}

		shader->PushConstantBufferToShader(ShaderType::PIXEL_SHADER, L"LightMaterial", 1, _meshRenderer->GetMaterialBuffer());

		// Light
		LightAndCameraPos lightDirection;
		lightDirection.lightPosition = GP.centerPos - lightObject->transform()->GetWorldPosition();
		lightDirection.cameraPosition = cameraObject->transform()->GetWorldPosition();

		shared_ptr<Buffer> light = make_shared<Buffer>();
		light->CreateConstantBuffer<LightAndCameraPos>();
		light->CopyData(lightDirection);


		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightAndCameraPos", 1, light);



		D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		shared_ptr<Buffer> buffer = mesh->GetBuffer();
		uint32 stride = buffer->GetStride();
		uint32 offset = 0;

		shared_ptr<InputLayout> inputLayout = shader->GetInputLayout();
		shared_ptr<RasterizerState> rasterizerState = make_shared<RasterizerState>();

		RasterizerStateInfo rasterzerStateInfo = _meshRenderer->GetRasterzerStates();
		rasterizerState->CreateRasterizerState(rasterzerStateInfo);

		// inputAssembler
		DEVICECONTEXT->IASetVertexBuffers(0, 1, buffer->GetVertexBuffer().GetAddressOf(), &stride, &offset);
		DEVICECONTEXT->IASetIndexBuffer(buffer->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

		DEVICECONTEXT->IASetInputLayout(shader->GetInputLayout()->GetInputLayout().Get());
		DEVICECONTEXT->IASetPrimitiveTopology(topology);

		// VertexShader
		ComPtr<ID3D11VertexShader> vertexShader = shader->GetVertexShader();
		ComPtr<ID3D11PixelShader> pixelShader = shader->GetPixelShader();

		normalMap = _meshRenderer->GetMaterial()->GetNormalMap();
		specularMap = _meshRenderer->GetMaterial()->GetSpecularMap();
		diffuseMap = _meshRenderer->GetMaterial()->GetDiffuseMap();


		// Set NormalMap
		if (normalMap != nullptr)
			shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"normalMap", 1, normalMap->GetShaderResourceView());

		// Set SpecularMap
		if (specularMap != nullptr)
			shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"specularMap", 1, specularMap->GetShaderResourceView());

		// Set DiffuseMap
		if (diffuseMap != nullptr)
			shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"diffuseMap", 1, diffuseMap->GetShaderResourceView());

		if (vertexShader != nullptr)
			DEVICECONTEXT->VSSetShader(vertexShader.Get(), nullptr, 0);

		if (pixelShader != nullptr)
			DEVICECONTEXT->PSSetShader(pixelShader.Get(), nullptr, 0);



		// Rasterizer
		DEVICECONTEXT->RSSetState(rasterizerState->GetRasterizerState().Get());

		shared_ptr<DepthStencilState> depthStencilState = make_shared<DepthStencilState>();
		depthStencilState->SetDepthStencilState(DSState::NORMAL);

		shared_ptr<SamplerState> samplerState = make_shared<SamplerState>();
		samplerState->CreateSamplerState();

		shared_ptr<SamplerState> shadow_SamplerState = make_shared<SamplerState>();
		shadow_SamplerState->CreateSamplerState();

		shared_ptr<BlendState> blendState = make_shared<BlendState>();
		blendState->CreateBlendState();

		DEVICECONTEXT->PSSetSamplers(0, 1, samplerState->GetSamplerState().GetAddressOf());
		DEVICECONTEXT->PSSetSamplers(1, 1, shadow_SamplerState->GetSamplerState().GetAddressOf());

		// OutputMerger
		DEVICECONTEXT->OMSetBlendState(blendState->GetBlendState().Get(), nullptr, 0xFFFFFFFF);
		DEVICECONTEXT->OMSetDepthStencilState(depthStencilState->GetDepthStecilState().Get(), 1);
		DEVICECONTEXT->DrawIndexed(mesh->GetGeometry()->GetIndices().size(), 0, 0);
	}
}

void RenderPass::ParticleRender(bool isEnv)
{
	shared_ptr<Shader> initParticleShader = RESOURCE.GetResource<Shader>(L"InitParticle_Shader");
	shared_ptr<Shader> renderParticleShader = _meshRenderer->GetShader();// RESOURCE.GetResource<Shader>(L"RenderParticle_Shader");

	Matrix viewMat;
	Matrix projMat;
	if (isEnv)
	{
		viewMat = SCENE.GetActiveScene()->Find(L"MainCamera")->GetComponent<Camera>()->GetEnvViewMatrix();
		projMat = SCENE.GetActiveScene()->Find(L"MainCamera")->GetComponent<Camera>()->GetEnvProjectionMatrix();
	}
	else
	{
		viewMat = SCENE.GetActiveScene()->Find(L"MainCamera")->GetComponent<Camera>()->GetViewMatrix();
		projMat = SCENE.GetActiveScene()->Find(L"MainCamera")->GetComponent<Camera>()->GetProjectionMatrix();
	}


	shared_ptr<ParticleSystem> particleComponent = _transformPtr->GetGameObject()->GetComponent<ParticleSystem>();
	Vec3 _eyePosW = XMFLOAT3(0.0f, 0.0f, 0.0f);
	Vec3 _emitPosW = XMFLOAT3(0.0f, 1.0f, 0.0f);
	Vec3 _emitDirW = XMFLOAT3(0.0f, 1.0f, 0.0f);

	Matrix viewProj = viewMat * projMat;

	ParticleBuffer pBuffer;
	pBuffer.gView = viewMat;
	pBuffer.gProj = projMat;
	pBuffer.gGameTime = TIME.GetTotalTime();
	pBuffer.gTimeStep = TIME.GetDeltaTime() / 7.0f;
	pBuffer.gEyePosW = SCENE.GetActiveScene()->FindWithComponent(ComponentType::Camera)->transform()->GetWorldPosition();
	pBuffer.gEmitPosW = _emitPosW;
	pBuffer.gEmitDirW = _emitDirW;

	shared_ptr<Buffer> particleBuffer = make_shared<Buffer>();
	particleBuffer->CreateConstantBuffer<ParticleBuffer>();
	particleBuffer->CopyData(pBuffer);

	static_assert(sizeof(ParticleBuffer) % 16 == 0, "Constant buffer size must be a multiple of 16 bytes.");

	initParticleShader->PushConstantBufferToShader(ShaderType::GEOMETRY_SHADER, L"ParticleBuffer", 1, particleBuffer);
	initParticleShader->PushShaderResourceToShader(ShaderType::GEOMETRY_SHADER, L"gRandomTex", 1, RESOURCE.GetResource<Texture>(L"Random_Texture")->GetShaderResourceView());

	shared_ptr<SamplerState> samplerState = make_shared<SamplerState>();
	samplerState->CreateSamplerState();

	// make particle vertex
	DEVICECONTEXT->IASetInputLayout(initParticleShader->GetInputLayout()->GetInputLayout().Get());
	DEVICECONTEXT->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	uint32 stride = sizeof(VertexParticle);
	uint32 offset = 0;

	if (particleComponent->GetFirstRunFlag())
		DEVICECONTEXT->IASetVertexBuffers(0, 1, particleComponent->GetinitVB().GetAddressOf(), &stride, &offset);
	else
		DEVICECONTEXT->IASetVertexBuffers(0, 1, particleComponent->GetDrawVB().GetAddressOf(), &stride, &offset);

	// VertexShader
	DEVICECONTEXT->VSSetShader(initParticleShader->GetVertexShader().Get(), nullptr, 0);

	// GeometryShader
	DEVICECONTEXT->GSSetShader(initParticleShader->GetOutputStreamGeometryShader().Get(), nullptr, 0);

	DEVICECONTEXT->GSSetSamplers(0, 1, samplerState->GetSamplerState().GetAddressOf());
	// PixelShader (not use)
	DEVICECONTEXT->PSSetShader(nullptr, nullptr, 0);

	DEVICECONTEXT->SOSetTargets(1, particleComponent->GetStreamOutVB().GetAddressOf(), &offset);

	if (particleComponent->GetFirstRunFlag())
	{
		DEVICECONTEXT->Draw(1, 0);
		particleComponent->SetFirstRunFlag(false);
	}
	else
	{
		DEVICECONTEXT->DrawAuto();
	}

	// 스트림 아웃 타겟 해제
	ID3D11Buffer* bufferArray[1] = { 0 };
	DEVICECONTEXT->SOSetTargets(1, bufferArray, &offset);

	// 버퍼 스왑
	std::swap(particleComponent->GetDrawVB(), particleComponent->GetStreamOutVB());


	// Render Particle



	shared_ptr<RasterizerState> rasterizerState = make_shared<RasterizerState>();
	RasterizerStateInfo states;
	states.fillMode = D3D11_FILL_SOLID;
	states.cullMode = D3D11_CULL_BACK;
	states.frontCounterClockwise = false;
	rasterizerState->CreateRasterizerState(states);

	shared_ptr<DepthStencilState> depthStencilState = make_shared<DepthStencilState>();
	depthStencilState->SetDepthStencilState(DSState::CUSTOM3);

	shared_ptr<BlendState> blendState = make_shared<BlendState>();
	blendState->CreateAdditiveBlendState();



	// inputAssembler
	DEVICECONTEXT->IASetVertexBuffers(0, 1, particleComponent->GetDrawVB().GetAddressOf(), &stride, &offset);

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

	shared_ptr<Texture> fireParticleTexture = _meshRenderer->GetMaterial()->GetTexture();
	renderParticleShader->PushConstantBufferToShader(ShaderType::GEOMETRY_SHADER, L"ParticleBuffer", 1, particleBuffer);
	renderParticleShader->PushConstantBufferToShader(ShaderType::GEOMETRY_SHADER, L"TransformBuffer", 1, _transformPtr->GetTransformBuffer());
	renderParticleShader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"gTexArray", 1, fireParticleTexture->GetShaderResourceView());

	DEVICECONTEXT->PSSetSamplers(0, 1, samplerState->GetSamplerState().GetAddressOf());


	// OutputMerger
	DEVICECONTEXT->OMSetBlendState(blendState->GetBlendState().Get(), nullptr, 0xFFFFFFFF);


	DEVICECONTEXT->OMSetDepthStencilState(depthStencilState->GetDepthStecilState().Get(), 1);

	DEVICECONTEXT->DrawAuto();

	DEVICECONTEXT->GSSetShader(nullptr, nullptr, 0);
}

void RenderPass::DebugQuadRender()
{
	shared_ptr<GameObject> cameraObject = SCENE.GetActiveScene()->Find(L"UICamera");
	shared_ptr<Shader> shader = _meshRenderer->GetMaterial()->GetShader();
	shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"TransformBuffer", 1, _transformPtr->GetTransformBuffer());
	shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"CameraBuffer", 1, cameraObject->GetCameraBuffer());

	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	shared_ptr<Buffer> buffer = _meshRenderer->GetMesh()->GetBuffer();
	uint32 stride = buffer->GetStride();
	uint32 offset = 0;

	shared_ptr<InputLayout> inputLayout = shader->GetInputLayout();
	shared_ptr<RasterizerState> rasterizerState = make_shared<RasterizerState>();

	RasterizerStateInfo rasterzerStateInfo = _meshRenderer->GetRasterzerStates();
	rasterizerState->CreateRasterizerState(rasterzerStateInfo);

	// inputAssembler
	DEVICECONTEXT->IASetVertexBuffers(0, 1, buffer->GetVertexBuffer().GetAddressOf(), &stride, &offset);
	DEVICECONTEXT->IASetIndexBuffer(buffer->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

	DEVICECONTEXT->IASetInputLayout(shader->GetInputLayout()->GetInputLayout().Get());
	DEVICECONTEXT->IASetPrimitiveTopology(topology);

	// VertexShader
	ComPtr<ID3D11VertexShader> vertexShader = shader->GetVertexShader();
	ComPtr<ID3D11PixelShader> pixelShader = shader->GetPixelShader();

	if (vertexShader != nullptr)
		DEVICECONTEXT->VSSetShader(vertexShader.Get(), nullptr, 0);

	if (pixelShader != nullptr)
		DEVICECONTEXT->PSSetShader(pixelShader.Get(), nullptr, 0);


	// Rasterizer
	DEVICECONTEXT->RSSetState(rasterizerState->GetRasterizerState().Get());

	shared_ptr<Texture> defaultTexture = _meshRenderer->GetMaterial()->GetTexture();

	// Set Default Texture
	if (defaultTexture != nullptr)
		shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"texture0", 1, _inputSRV);



	shared_ptr<DepthStencilState> depthStencilState = make_shared<DepthStencilState>();
	depthStencilState->SetDepthStencilState(DSState::NORMAL);

	shared_ptr<SamplerState> samplerState = make_shared<SamplerState>();
	samplerState->CreateSamplerState();

	shared_ptr<BlendState> blendState = make_shared<BlendState>();
	blendState->CreateBlendState();

	DEVICECONTEXT->PSSetSamplers(0, 1, samplerState->GetSamplerState().GetAddressOf());

	// OutputMerger
	DEVICECONTEXT->OMSetBlendState(blendState->GetBlendState().Get(), nullptr, 0xFFFFFFFF);
	DEVICECONTEXT->OMSetDepthStencilState(depthStencilState->GetDepthStecilState().Get(), 1);
	DEVICECONTEXT->DrawIndexed(_meshRenderer->GetMesh()->GetGeometry()->GetIndices().size(), 0, 0);
}

void RenderPass::SetRenderTarget(int width, int height)
{
	if (_pass != Pass::DEFAULT_RENDER)
	{
		CreateAndSetOffscreenRenderTarget(width, height);
	}

}

void RenderPass::CreateAndSetOffscreenRenderTarget(int width, int height)
{
	D3D11_TEXTURE2D_DESC texDesc;

	texDesc.Width = width;
	texDesc.Height = height;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_B8G8R8X8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> offscreenTex = 0;
	HRESULT hr = DEVICE->CreateTexture2D(&texDesc, 0, offscreenTex.GetAddressOf());
	CHECK(hr);

	// Null description means to create a view to all mipmap levels using 
	// the format the texture was created with.
	hr = DEVICE->CreateShaderResourceView(offscreenTex.Get(), 0, _offscreenSRV.GetAddressOf());
	CHECK(hr);
	hr = DEVICE->CreateRenderTargetView(offscreenTex.Get(), 0, _offscreenRTV.GetAddressOf());
	CHECK(hr);

	// depthStenciView 생성
	ComPtr<ID3D11Texture2D> depthStencilTexture;
	ComPtr<ID3D11DepthStencilView> depthStencilView;

	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = width;  // 렌더 타겟과 동일한 너비
	depthDesc.Height = height; // 렌더 타겟과 동일한 높이
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	hr = DEVICE->CreateTexture2D(&depthDesc, nullptr, &depthStencilTexture);
	CHECK(hr);

	hr = DEVICE->CreateDepthStencilView(depthStencilTexture.Get(), nullptr, &depthStencilView);
	CHECK(hr);


	// viewport 설정
	D3D11_VIEWPORT viewport = {};
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	float _clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };


	DEVICECONTEXT->RSSetViewports(1, &viewport);
	DEVICECONTEXT->OMSetRenderTargets(1, _offscreenRTV.GetAddressOf(), depthStencilView.Get());
	DEVICECONTEXT->ClearRenderTargetView(_offscreenRTV.Get(), _clearColor);
	DEVICECONTEXT->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void RenderPass::SaveRenderTargetToFile(ID3D11RenderTargetView* renderTargetView, const std::string& filename, int width, int height)
{
	// 1. Render Target에서 사용 중인 텍스처를 가져옴
	ComPtr<ID3D11Resource> renderTargetResource;
	renderTargetView->GetResource(&renderTargetResource);

	ComPtr<ID3D11Texture2D> renderTargetTexture;
	renderTargetResource.As(&renderTargetTexture);

	// 2. Staging 텍스처 생성 (CPU에서 읽기 가능하도록 설정)
	D3D11_TEXTURE2D_DESC stagingDesc = {};
	renderTargetTexture->GetDesc(&stagingDesc);
	stagingDesc.Usage = D3D11_USAGE_STAGING;
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	stagingDesc.BindFlags = 0;
	stagingDesc.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> stagingTexture;
	HRESULT hr = DEVICE->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);
	CHECK(hr);

	// 3. Render Target 텍스처를 Staging 텍스처로 복사
	DEVICECONTEXT->CopyResource(stagingTexture.Get(), renderTargetTexture.Get());

	// 4. Staging 텍스처에서 데이터를 맵핑하여 가져옴
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	hr = DEVICECONTEXT->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mappedResource);
	CHECK(hr);

	// 5. 픽셀 데이터를 파일로 저장
	std::ofstream outFile(filename, std::ios::binary);
	if (outFile.is_open())
	{
		// 한 행마다의 바이트 수 (행의 너비 * 4)
		auto rowPitch = width * 4;

		// 각 행의 데이터를 순차적으로 파일에 기록
		for (int y = 0; y < height; ++y)
		{
			outFile.write(static_cast<const char*>(mappedResource.pData) + y * mappedResource.RowPitch, rowPitch);
		}
		outFile.close();
	}

	// 6. 맵핑 해제
	DEVICECONTEXT->Unmap(stagingTexture.Get(), 0);
}
