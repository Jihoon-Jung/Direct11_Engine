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
	DEVICECONTEXT->VSSetShader(_shader->GetVertexShader().Get(), nullptr, 0);

	// Rasterizer
	DEVICECONTEXT->RSSetState(pipelineInfo.rasterizerState->GetRasterizerState().Get());

	// PixelShader
	DEVICECONTEXT->PSSetShader(_shader->GetPixelShader().Get(), nullptr, 0);
	

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

	DEVICECONTEXT->PSSetSamplers(0, 1, pipelineInfo.samplerState->GetSamplerState().GetAddressOf());

	// OutputMerger
	DEVICECONTEXT->OMSetBlendState(pipelineInfo.blendState->GetBlendState().Get(), nullptr, 0xFFFFFFFF);
	DEVICECONTEXT->OMSetDepthStencilState(pipelineInfo.depthStencilState->GetDepthStecilState().Get(), 1);
	DEVICECONTEXT->DrawIndexed(_indicesSize, 0, 0);

}
