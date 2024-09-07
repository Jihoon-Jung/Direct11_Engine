#include "pch.h"
#include "RenderPass.h"
#include <fstream>
#include "MeshRenderer.h"
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
	else if (_pass == Pass::QUAD_RENDER)
		QuadRender();

}

void RenderPass::DefaultRender()
{
	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	shared_ptr<Buffer> buffer = _mesh->GetBuffer();
	uint32 stride = buffer->GetStride();
	uint32 offset = 0;

	shared_ptr<InputLayout> inputLayout = _shader->GetInputLayout();
	shared_ptr<RasterizerState> rasterizerState = make_shared<RasterizerState>();

	RasterizerStateInfo rasterzerStateInfo = _rasterizerStates;
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
		_shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"texture0", 1, _texture->GetShaderResourceView());
	// Set NormalMap
	if (_normalMap != nullptr)
		_shader->PushShaderResourceToShader(ShaderType::PIXEL_SHADER, L"normalMap", 1, _normalMap->GetShaderResourceView());

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
}

void RenderPass::GaussianBlurRender()
{
	SetRenderTarget(GWinSizeX, GWinSizeY);
	DefaultRender();
	_outputSRV = _offscreenSRV;

	SaveRenderTargetToFile(_offscreenRTV.Get(), "output.dump", GWinSizeX, GWinSizeY);

	shared_ptr<Material> material = make_shared<Material>();
	
	shared_ptr<Shader> computeShader_gaussianBlurHorizontal = RESOURCE.GetResource<Shader>(L"Gaussian_Horizontal");
	shared_ptr<Shader> computeShader_gaussianBlurVertical = RESOURCE.GetResource<Shader>(L"Gaussian_Vertical");
	_outputSRV = material->GaussainBlur(computeShader_gaussianBlurVertical, computeShader_gaussianBlurHorizontal, _offscreenSRV);
}

void RenderPass::QuadRender()
{
	GP.RestoreRenderTarget();

	D3D11_PRIMITIVE_TOPOLOGY topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

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
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
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

	// depthStenciView ����
	ComPtr<ID3D11Texture2D> depthStencilTexture;
	ComPtr<ID3D11DepthStencilView> depthStencilView;

	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = width;  // ���� Ÿ�ٰ� ������ �ʺ�
	depthDesc.Height = height; // ���� Ÿ�ٰ� ������ ����
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

	hr = DEVICE->CreateTexture2D(&depthDesc, nullptr, &depthStencilTexture);
	CHECK(hr);

	hr = DEVICE->CreateDepthStencilView(depthStencilTexture.Get(), nullptr, &depthStencilView);
	CHECK(hr);


	// viewport ����
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
	// 1. Render Target���� ��� ���� �ؽ�ó�� ������
	ComPtr<ID3D11Resource> renderTargetResource;
	renderTargetView->GetResource(&renderTargetResource);

	ComPtr<ID3D11Texture2D> renderTargetTexture;
	renderTargetResource.As(&renderTargetTexture);

	// 2. Staging �ؽ�ó ���� (CPU���� �б� �����ϵ��� ����)
	D3D11_TEXTURE2D_DESC stagingDesc = {};
	renderTargetTexture->GetDesc(&stagingDesc);
	stagingDesc.Usage = D3D11_USAGE_STAGING;
	stagingDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	stagingDesc.BindFlags = 0;
	stagingDesc.MiscFlags = 0;

	ComPtr<ID3D11Texture2D> stagingTexture;
	HRESULT hr = DEVICE->CreateTexture2D(&stagingDesc, nullptr, &stagingTexture);
	CHECK(hr);

	// 3. Render Target �ؽ�ó�� Staging �ؽ�ó�� ����
	DEVICECONTEXT->CopyResource(stagingTexture.Get(), renderTargetTexture.Get());

	// 4. Staging �ؽ�ó���� �����͸� �����Ͽ� ������
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	hr = DEVICECONTEXT->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mappedResource);
	CHECK(hr);

	// 5. �ȼ� �����͸� ���Ϸ� ����
	std::ofstream outFile(filename, std::ios::binary);
	if (outFile.is_open())
	{
		// �� �ึ���� ����Ʈ �� (���� �ʺ� * 4)
		auto rowPitch = width * 4;

		// �� ���� �����͸� ���������� ���Ͽ� ���
		for (int y = 0; y < height; ++y)
		{
			outFile.write(static_cast<const char*>(mappedResource.pData) + y * mappedResource.RowPitch, rowPitch);
		}
		outFile.close();
	}

	// 6. ���� ����
	DEVICECONTEXT->Unmap(stagingTexture.Get(), 0);
}
