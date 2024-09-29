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

void RenderPass::Render()
{
	if (_pass == Pass::DEFAULT_RENDER)
		DefaultRender();
	else if (_pass == Pass::GAUSSIANBLUR_RENDER)
		GaussianBlurRender();
	else if (_pass == Pass::OUTLINE_RENDER)
		OutlineRender();
	else if (_pass == Pass::QUAD_RENDER)
		QuadRender();
	else if (_pass == Pass::TERRAIN_RENDER)
		TerrainRender();
}

void RenderPass::DefaultRender()
{
	D3D11_PRIMITIVE_TOPOLOGY topology;
	if (isUseTessellation)
		topology = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
	else
		topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	shared_ptr<Buffer> buffer = _mesh->GetBuffer();
	uint32 stride = buffer->GetStride();
	uint32 offset = 0;

	shared_ptr<InputLayout> inputLayout = _shader->GetInputLayout();
	shared_ptr<RasterizerState> rasterizerState = make_shared<RasterizerState>();

	RasterizerStateInfo rasterzerStateInfo = _rasterizerStates;
	/*if (isUseTessellation)
		rasterzerStateInfo.fillMode = D3D11_FILL_WIREFRAME;*/
	rasterizerState->CreateRasterizerState(rasterzerStateInfo);

	// inputAssembler
	DEVICECONTEXT->IASetVertexBuffers(0, 1, buffer->GetVertexBuffer().GetAddressOf(), &stride, &offset);
	DEVICECONTEXT->IASetIndexBuffer(buffer->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

	DEVICECONTEXT->IASetInputLayout(_shader->GetInputLayout()->GetInputLayout().Get());
	DEVICECONTEXT->IASetPrimitiveTopology(topology);

	// VertexShader
	ComPtr<ID3D11VertexShader> vertexShader = _shader->GetVertexShader();
	ComPtr<ID3D11PixelShader> pixelShader = _shader->GetPixelShader();
	ComPtr<ID3D11HullShader> hullShader = _shader->GetHullShader();
	ComPtr<ID3D11DomainShader> domainShader = _shader->GetDomainShader();

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

	// Set Default Texture
	if (_texture != nullptr)
	{
		if (isUseTessellation)
		{
			_shader->PushShaderResourceToShader(ShaderType::DOMAIN_SHADER, L"texture0", 1, _texture->GetShaderResourceView());
		}
		_shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"texture0", 1, _texture->GetShaderResourceView());
	}
		
	// Set NormalMap
	if (_normalMap != nullptr)
	{
		if (isUseTessellation)
		{
			_shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"normalMap", 1, _normalMap->GetShaderResourceView());
			_shader->PushShaderResourceToShader(ShaderType::DOMAIN_SHADER, L"normalMap", 1, _normalMap->GetShaderResourceView());
		}
		else
			_shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"normalMap", 1, _normalMap->GetShaderResourceView());
	}

	// Set SpecularMap
	if (_specularMap != nullptr)
		_shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"specularMap", 1, _specularMap->GetShaderResourceView());

	// Set DiffuseMap
	if (_diffuseMap != nullptr)
		_shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"diffuseMap", 1, _diffuseMap->GetShaderResourceView());


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
	DEVICECONTEXT->DrawIndexed(_mesh->GetGeometry()->GetIndices().size(), 0, 0);

	if (isUseTessellation)
	{
		DEVICECONTEXT->HSSetShader(nullptr, nullptr, 0);
		DEVICECONTEXT->DSSetShader(nullptr, nullptr, 0);
	}
	
}

void RenderPass::OutlineRender()
{
	shared_ptr<GameObject> cameraObject = SCENE.GetActiveScene()->Find(L"MainCamera");
	shared_ptr<Shader> shader = _meshRenderer->GetMaterial()->GetShader();
	shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"TransformBuffer", 1, _transform->GetTransformBuffer());
	if (isEnvironmentMap)
		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"CameraBuffer", 1, cameraObject->GetEnvCameraBuffer());
	else
		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"CameraBuffer", 1, cameraObject->GetCameraBuffer());

	shared_ptr<GameObject> lightObject = SCENE.GetActiveScene()->FindWithComponent(ComponentType::Light);
	if (lightObject != nullptr)
	{
		lightObject->GetLightBuffer();
		shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightDesc", 1, lightObject->GetLightBuffer());
	}

	shader->PushConstantBufferToShader(ShaderType::VERTEX_SHADER, L"LightMaterial", 1, _meshRenderer->GetMaterialBuffer());

	LightAndCameraPos lightDirection;
	lightDirection.lightPosition = SCENE.GetActiveScene()->FindWithComponent(ComponentType::Light)->transform()->GetWorldPosition();
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
		shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"diffuseMap", 1, _diffuseMap->GetShaderResourceView());


	shared_ptr<DepthStencilState> depthStencilState = make_shared<DepthStencilState>();
	depthStencilState->SetDepthStencilState(_dsStateType);

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

void RenderPass::GaussianBlurRender()
{
	SetRenderTarget(GWinSizeX, GWinSizeY);
	DefaultRender();
	_outputSRV = _offscreenSRV;

	//SaveRenderTargetToFile(_offscreenRTV.Get(), "output.dump", GWinSizeX, GWinSizeY);

	shared_ptr<Material> material = make_shared<Material>();
	
	shared_ptr<Shader> computeShader_gaussianBlurHorizontal = RESOURCE.GetResource<Shader>(L"Gaussian_Horizontal");
	shared_ptr<Shader> computeShader_gaussianBlurVertical = RESOURCE.GetResource<Shader>(L"Gaussian_Vertical");
	_outputSRV = material->GaussainBlur(computeShader_gaussianBlurVertical, computeShader_gaussianBlurHorizontal, _offscreenSRV);
}

void RenderPass::QuadRender()
{
	GP.RestoreRenderTarget();

	D3D11_PRIMITIVE_TOPOLOGY topology;
	if (isUseTessellation)
		topology = D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
	else
		topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	shared_ptr<Buffer> buffer = _mesh->GetBuffer();
	uint32 stride = buffer->GetStride();
	uint32 offset = 0;

	shared_ptr<InputLayout> inputLayout = _shader->GetInputLayout();
	shared_ptr<RasterizerState> rasterizerState = make_shared<RasterizerState>();

	RasterizerStateInfo rasterzerStates;
	rasterzerStates.fillMode = D3D11_FILL_SOLID;
	rasterzerStates.cullMode = D3D11_CULL_BACK;
	rasterzerStates.frontCouterClockWise = false;
	rasterizerState->CreateRasterizerState(rasterzerStates);

	// inputAssembler
	DEVICECONTEXT->IASetVertexBuffers(0, 1, buffer->GetVertexBuffer().GetAddressOf(), &stride, &offset);
	DEVICECONTEXT->IASetIndexBuffer(buffer->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

	DEVICECONTEXT->IASetInputLayout(_shader->GetInputLayout()->GetInputLayout().Get());
	DEVICECONTEXT->IASetPrimitiveTopology(topology);

	// VertexShader
	DEVICECONTEXT->VSSetShader(_shader->GetVertexShader().Get(), nullptr, 0);

	// Rasterizer
	DEVICECONTEXT->RSSetState(rasterizerState->GetRasterizerState().Get());

	// PixelShader
	DEVICECONTEXT->PSSetShader(_shader->GetPixelShader().Get(), nullptr, 0);


	// Set Default Texture
	_shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"texture0", 1, _inputSRV);

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
	DEVICECONTEXT->DrawIndexed(_mesh->GetGeometry()->GetIndices().size(), 0, 0);
}

void RenderPass::TerrainRender()
{
	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;

	shared_ptr<Buffer> buffer = _mesh->GetBuffer();
	uint32 stride = buffer->GetStride();
	uint32 offset = 0;

	shared_ptr<InputLayout> inputLayout = _shader->GetInputLayout();
	shared_ptr<RasterizerState> rasterizerState = make_shared<RasterizerState>();

	RasterizerStateInfo rasterzerStateInfo = _rasterizerStates;
	//rasterzerStateInfo.fillMode = D3D11_FILL_WIREFRAME;
	/*if (isUseTessellation)
		rasterzerStateInfo.fillMode = D3D11_FILL_WIREFRAME;*/
	rasterizerState->CreateRasterizerState(rasterzerStateInfo);

	// inputAssembler
	DEVICECONTEXT->IASetVertexBuffers(0, 1, buffer->GetVertexBuffer().GetAddressOf(), &stride, &offset);
	DEVICECONTEXT->IASetIndexBuffer(buffer->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

	DEVICECONTEXT->IASetInputLayout(_shader->GetInputLayout()->GetInputLayout().Get());
	DEVICECONTEXT->IASetPrimitiveTopology(topology);

	// VertexShader
	ComPtr<ID3D11VertexShader> vertexShader = _shader->GetVertexShader();
	ComPtr<ID3D11PixelShader> pixelShader = _shader->GetPixelShader();
	ComPtr<ID3D11HullShader> hullShader = _shader->GetHullShader();
	ComPtr<ID3D11DomainShader> domainShader = _shader->GetDomainShader();

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

	ComPtr<ID3D11ShaderResourceView> layerMapArray = _mesh->GetLayerMapArraySRV();	// PS
	ComPtr<ID3D11ShaderResourceView> blendMap = _mesh->GetBlendMapSRV();		// PS
	ComPtr<ID3D11ShaderResourceView> heightMap = _mesh->GetHeightMapSRV();		// VS, DS, PS


	_shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"gLayerMapArray", 1, layerMapArray);
	_shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"gBlendMap", 1, blendMap);
	_shader->PushShaderResourceToShader(ShaderType::VERTEX_SHADER, L"gHeightMap", 1, heightMap);
	_shader->PushShaderResourceToShader(ShaderType::DOMAIN_SHADER, L"gHeightMap", 1, heightMap);
	_shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"gHeightMap", 1, heightMap);

	Matrix viewMat = SCENE.GetActiveScene()->Find(L"MainCamera")->GetComponent<Camera>()->GetViewMatrix();
	Matrix projMat = SCENE.GetActiveScene()->Find(L"MainCamera")->GetComponent<Camera>()->GetProjectionMatrix();
	if (isEnvironmentMap)
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
	terrainBufferInfo.texelCellSpaceU = (1.0 / _mesh->GetTerrainInfo().heightmapWidth);
	terrainBufferInfo.texelCellSpaceV = (1.0f / _mesh->GetTerrainInfo().heightmapHeight);
	terrainBufferInfo.worldCellSpace = _mesh->GetTerrainInfo().cellSpacing;
	terrainBufferInfo.texScale = 50.0f;
	memcpy(terrainBufferInfo.frustom, frustom, sizeof(frustom));

	shared_ptr<Buffer> terrainBuffer = make_shared<Buffer>();
	terrainBuffer->CreateConstantBuffer<TerrainBuffer>();
	terrainBuffer->CopyData(terrainBufferInfo);


	_shader->PushConstantBufferToShader(ShaderType::HULL_SHADER, L"TerrainBuffer", 1, terrainBuffer);
	_shader->PushConstantBufferToShader(ShaderType::PIXEL_SHADER, L"TerrainBuffer", 1, terrainBuffer);
	_shader->PushConstantBufferToShader(ShaderType::DOMAIN_SHADER, L"TerrainBuffer", 1, terrainBuffer);

	shared_ptr<DepthStencilState> depthStencilState = make_shared<DepthStencilState>();
	depthStencilState->SetDepthStencilState(DSState::NORMAL);

	shared_ptr<SamplerState> samplerState = make_shared<SamplerState>();
	samplerState->CreateSamplerState();

	shared_ptr<SamplerState> heightMapSamplerState = make_shared<SamplerState>();
	heightMapSamplerState->CreateHeightMapSamplerState();

	shared_ptr<BlendState> blendState = make_shared<BlendState>();
	blendState->CreateBlendState();

	DEVICECONTEXT->PSSetSamplers(0, 1, samplerState->GetSamplerState().GetAddressOf());

	DEVICECONTEXT->VSSetSamplers(0, 1, heightMapSamplerState->GetSamplerState().GetAddressOf());
	DEVICECONTEXT->DSSetSamplers(0, 1, heightMapSamplerState->GetSamplerState().GetAddressOf());
	DEVICECONTEXT->PSSetSamplers(0, 1, heightMapSamplerState->GetSamplerState().GetAddressOf());

	// OutputMerger
	DEVICECONTEXT->OMSetBlendState(blendState->GetBlendState().Get(), nullptr, 0xFFFFFFFF);
	DEVICECONTEXT->OMSetDepthStencilState(depthStencilState->GetDepthStecilState().Get(), 1);
	DEVICECONTEXT->DrawIndexed(_mesh->GetTerrainGeometry()->GetIndices().size(), 0, 0);

	DEVICECONTEXT->HSSetShader(nullptr, nullptr, 0);
	DEVICECONTEXT->DSSetShader(nullptr, nullptr, 0);
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
