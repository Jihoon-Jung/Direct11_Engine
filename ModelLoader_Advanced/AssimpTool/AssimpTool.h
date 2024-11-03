#pragma once
class AssimpTool
{
public:
	void Init(HWND hwnd);
	void ConvertModel(const std::wstring& folderName, const std::wstring& fileName);
	void ConvertAnimation(const std::wstring& folderName, const std::wstring& fileName, const std::wstring& animName);
	void Update();
	void Render();
	void CreateDeviceAndSwapChain();
	void CreateRenderTarget();
	void RefreshFileList();
	void RenderUI();
	void SetScreenSize(int width, int height) { _screenWidth = width; _screenHeight = height; };
private:
	HWND _hwnd;
	ComPtr<ID3D11Device> _device = nullptr;
	ComPtr<ID3D11DeviceContext> _deviceContext = nullptr;
	ComPtr<IDXGISwapChain> _swapChain = nullptr;
	ComPtr<ID3D11RenderTargetView> _renderTargetView = nullptr;
	int _screenWidth = 0;
	int _screenHeight = 0;

	string _resourcePath;
	vector<string> _fileList;

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	bool show_demo_window = true;
	bool show_another_window = true;

private :
	bool show_convert_popup = false;
	bool show_anim_name_popup = false;
	char anim_name_buffer[256] = "";
	string selected_folder;
	string selected_file;
};

