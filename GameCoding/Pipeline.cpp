#include "pch.h"
#include "Pipeline.h"
#include "VertexDescription.h"

Pipeline::Pipeline()
{
}

Pipeline::~Pipeline()
{
}


void Pipeline::UpdatePipeline(PipelineInfo pipelineInfo)
{

	uint32 stride = _buffer->GetStride();
	uint32 offset = 0;

	// inputAssembler
	DEVICECONTEXT->IASetVertexBuffers(0, 1, _buffer->GetVertexBuffer().GetAddressOf(), &stride, &offset);
	DEVICECONTEXT->IASetIndexBuffer(_buffer->GetIndexBuffer().Get(), DXGI_FORMAT_R32_UINT, 0);

	DEVICECONTEXT->IASetInputLayout(pipelineInfo.inputLayout->GetInputLayout().Get());
	DEVICECONTEXT->IASetPrimitiveTopology(pipelineInfo.topology);

	// VertexShader
	DEVICECONTEXT->VSSetShader(_vertexShader->GetVertexShader().Get(), nullptr, 0);

	// Rasterizer
	DEVICECONTEXT->RSSetState(pipelineInfo.rasterizerState->GetRasterizerState().Get());

	// PixelShader
	DEVICECONTEXT->PSSetShader(_pixelShader->GetPixelShader().Get(), nullptr, 0);
	if (isAnimation)
	{
		// Set NormalMap
		if (_normalMap != nullptr)
			DEVICECONTEXT->PSSetShaderResources(0, 1, _normalMap->GetShaderResourceView().GetAddressOf());
		// Set SpecularMap
		if (_specularMap != nullptr)
			DEVICECONTEXT->PSSetShaderResources(1, 1, _specularMap->GetShaderResourceView().GetAddressOf());
		// Set DiffuseMap
		if (_diffuseMap != nullptr)
			DEVICECONTEXT->PSSetShaderResources(2, 1, _diffuseMap->GetShaderResourceView().GetAddressOf());
	}
	else
	{
		if (_texture != nullptr)
			DEVICECONTEXT->PSSetShaderResources(0, 1, _texture->GetShaderResourceView().GetAddressOf());
		// Set NormalMap
		if (_normalMap != nullptr)
			DEVICECONTEXT->PSSetShaderResources(1, 1, _normalMap->GetShaderResourceView().GetAddressOf());
		// Set SpecularMap
		if (_specularMap != nullptr)
			DEVICECONTEXT->PSSetShaderResources(2, 1, _specularMap->GetShaderResourceView().GetAddressOf());
		// Set DiffuseMap
		if (_diffuseMap != nullptr)
			DEVICECONTEXT->PSSetShaderResources(3, 1, _diffuseMap->GetShaderResourceView().GetAddressOf());
	}

	DEVICECONTEXT->PSSetSamplers(0, 1, pipelineInfo.samplerState->GetSamplerState().GetAddressOf());

	// OutputMerger
	DEVICECONTEXT->OMSetBlendState(pipelineInfo.blendState->GetBlendState().Get(), nullptr, 0xFFFFFFFF);

	DEVICECONTEXT->DrawIndexed(_indicesSize, 0, 0);

}
