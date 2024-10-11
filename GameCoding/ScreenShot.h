#pragma once
class ScreenShot
{
public:
	void SaveRenderTargetToFile(ID3D11RenderTargetView* renderTargetView, const std::string& filename, int width, int height);
	void SaveShaderResourceViewToFile(ID3D11ShaderResourceView* shaderResourceView, const std::string& filename, int width, int height);
};

