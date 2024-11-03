#include "pch.h"
#include "AssimpTool.h"
#include "Converter.h"

void AssimpTool::Init(HWND hwnd)
{
	_hwnd = hwnd;
	CreateDeviceAndSwapChain();

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX11_Init(_device.Get(), _deviceContext.Get());


	// 상대 경로 설정
	char buffer[256];
	GetCurrentDirectoryA(256, buffer);
	_resourcePath = string(buffer) + "\\..\\Resources\\Assets\\";
	RefreshFileList();

}

void AssimpTool::ConvertModel(const std::wstring& folderName, const std::wstring& fileName)
{
	shared_ptr<Converter> converter = make_shared<Converter>();

	std::wstring filePath = folderName + L"/" + fileName;
	converter->ReadAssetFile(filePath);

	std::wstring basePath = folderName + L"/" + folderName;
	converter->ExportMaterialData(basePath);
	converter->ExportModelData(basePath);

	// 변환 완료 팝업 표시 설정
	show_convert_popup = true;

}

void AssimpTool::ConvertAnimation(const std::wstring& folderName, const std::wstring& fileName, const std::wstring& animName)
{
	shared_ptr<Converter> converter = make_shared<Converter>();

	std::wstring filePath = folderName + L"/" + fileName;
	converter->ReadAssetFile(filePath);

	std::wstring basePath = folderName + L"/" + animName;
	converter->ExportAnimationData(basePath);
}

void AssimpTool::Update()
{
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	CreateRenderTarget();
	RenderUI();
}

void AssimpTool::Render()
{
	// Rendering
	ImGui::Render();
	
	const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
	_deviceContext->OMSetRenderTargets(1, _renderTargetView.GetAddressOf(), nullptr);
	_deviceContext->ClearRenderTargetView(_renderTargetView.Get(), clear_color_with_alpha);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// Present
	HRESULT hr = _swapChain->Present(1, 0);   // Present with vsync
	//HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
}

void AssimpTool::CreateDeviceAndSwapChain()
{
	DXGI_SWAP_CHAIN_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	{
		int32 _width = _screenWidth;
		int32 _height = _screenHeight;
		desc.BufferDesc.Width = _width;
		desc.BufferDesc.Height = _height;
		desc.BufferDesc.RefreshRate.Numerator = 60;
		desc.BufferDesc.RefreshRate.Denominator = 1;
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = 1;
		desc.OutputWindow = _hwnd;
		desc.Windowed = true;
		desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	}

	HRESULT hr = ::D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		0,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&desc,
		_swapChain.GetAddressOf(),
		_device.GetAddressOf(),
		nullptr,
		_deviceContext.GetAddressOf()
	);

	CHECK(hr);
}

void AssimpTool::CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	_swapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	_device->CreateRenderTargetView(pBackBuffer, nullptr, _renderTargetView.GetAddressOf());
	pBackBuffer->Release();
}

void AssimpTool::RefreshFileList()
{
	_fileList.clear();

	WIN32_FIND_DATAA findData;
	string searchPath = _resourcePath + "*.*";
	HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (strcmp(findData.cFileName, ".") != 0 && strcmp(findData.cFileName, "..") != 0)
			{
				// 파일이나 폴더인 경우만 추가
				if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					_fileList.push_back("[Dir] " + string(findData.cFileName));
				else
					_fileList.push_back(findData.cFileName);
			}
		} while (FindNextFileA(hFind, &findData));
		FindClose(hFind);
	}
}

void AssimpTool::RenderUI()
{
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	// Fixed Position Window
	{
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(_screenWidth, _screenHeight / 6.0f));
		ImGui::Begin("Manual", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
		ImGui::Text("This tool converts FBX files to make them usable in our project.\n"
			"Select the FBX file in the folder shown in the window below, right-click it, and if it is a Mesh, click Add Mesh.\n"
			"If it is an Animation, click Add Animation.\n"
			"When the pop-up window displays \"Convert Finish!\", the process is complete.\nPlease note that we currently only guarantee conversion for FBX files provided by Mixamo \n(in the case of Animation).");
		ImGui::End();
	}

	// Another Window
	{
		ImGui::SetNextWindowPos(ImVec2(0, _screenHeight / 6.0f), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(_screenWidth, _screenHeight * 5.0f / 6.0f));
		ImGui::Begin("Resource Browser", nullptr,
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoCollapse);

		// 현재 경로 표시
		ImGui::Text("Path: %s", _resourcePath.c_str());
		ImGui::Separator();

		// 새로고침 버튼
		if (ImGui::Button("Refresh##ResourceBrowser"))  // 고유 ID 추가
			RefreshFileList();

		// 상위 폴더로 이동 버튼
		ImGui::SameLine();
		if (ImGui::Button("Up##ResourceBrowser"))  // 고유 ID 추가
		{
			size_t lastSlash = _resourcePath.find_last_of('\\', _resourcePath.length() - 2);
			if (lastSlash != string::npos)
			{
				_resourcePath = _resourcePath.substr(0, lastSlash + 1);
				RefreshFileList();
			}
		}

		// 파일 목록
		ImGui::BeginChild("Files##ResourceBrowser", ImVec2(0, 0), true);
		for (size_t i = 0; i < _fileList.size(); i++)
		{
			const auto& file = _fileList[i];
			string label = file + "##" + to_string(i);  // 각 항목에 고유 ID 추가

			if (ImGui::Selectable(label.c_str(), false, 0, ImVec2(0, 0)))
			{
				// 싱글 클릭 처리
				string fullPath = _resourcePath + file;
			}

			// 더블 클릭 처리
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
			{
				if (file.substr(0, 5) == "[Dir]")
				{
					string folderName = file.substr(6);  // "[Dir] " 제거
					_resourcePath += folderName + "\\";
					RefreshFileList();
				}
			}

			// 우클릭 메뉴 (FBX 파일인 경우에만)
			if (ImGui::BeginPopupContextItem(label.c_str()))
			{
				if (file.substr(0, 5) != "[Dir]")  // 디렉토리가 아닌 경우에만
				{
					string extension = file.substr(file.find_last_of(".") + 1);
					if (_stricmp(extension.c_str(), "fbx") == 0)
					{
						if (ImGui::MenuItem("Add Mesh"))
						{
							// 현재 폴더 이름 추출
							string currentPath = _resourcePath;
							size_t assetsPos = currentPath.find("Assets\\");
							if (assetsPos != string::npos)
							{
								string currentFolder = currentPath.substr(assetsPos + 7); // "Assets\" 이후의 경로
								if (!currentFolder.empty())
								{
									currentFolder = currentFolder.substr(0, currentFolder.length() - 1); // 마지막 '\\' 제거

									// 파일 이름에서 확장자 제거
									string fileName = file;
									fileName = fileName.substr(0, fileName.find_last_of("."));

									// wstring으로 변환
									wstring folderNameW = wstring(currentFolder.begin(), currentFolder.end());
									wstring fileNameW = wstring(fileName.begin(), fileName.end());

									// ConvertModel 호출
									ConvertModel(folderNameW, fileNameW + L".fbx");
								}
							}
						}
						// Add Animation 메뉴 아이템 부분 (RenderUI 함수 내)
						if (ImGui::MenuItem("Add Animation"))
						{
							ImGui::CloseCurrentPopup(); // 현재 컨텍스트 메뉴를 닫음

							// 현재 폴더와 파일 이름 저장
							string currentPath = _resourcePath;
							size_t assetsPos = currentPath.find("Assets\\");
							if (assetsPos != string::npos)
							{
								selected_folder = currentPath.substr(assetsPos + 7);
								if (!selected_folder.empty())
								{
									selected_folder = selected_folder.substr(0, selected_folder.length() - 1);
									selected_file = file;
									show_anim_name_popup = true;
								}
							}
						}
					}
				}
				ImGui::EndPopup();
			}
		}
		ImGui::EndChild();

		ImGui::End();
	}
	// 변환 완료 팝업
	if (show_convert_popup)
	{
		// 팝업창을 화면 중앙에 위치시킴
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("Convert Result", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Convert Finish !");
			ImGui::Separator();

			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				show_convert_popup = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		// 팝업이 표시되지 않은 경우에만 OpenPopup 호출
		if (!ImGui::IsPopupOpen("Convert Result"))
			ImGui::OpenPopup("Convert Result");
	}
	// 애니메이션 이름 입력 팝업
	if (show_anim_name_popup)
	{
		ImGui::OpenPopup("Animation Name");
		ImVec2 center = ImGui::GetMainViewport()->GetCenter();
		ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("Animation Name", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Enter animation name:");
			ImGui::InputText("##AnimName", anim_name_buffer, IM_ARRAYSIZE(anim_name_buffer));
			ImGui::Separator();

			if (ImGui::Button("OK", ImVec2(120, 0)) && anim_name_buffer[0] != '\0')
			{
				wstring folderNameW = wstring(selected_folder.begin(), selected_folder.end());
				wstring fileNameW = wstring(selected_file.begin(), selected_file.end());
				wstring animNameW = wstring(anim_name_buffer, anim_name_buffer + strlen(anim_name_buffer));

				ConvertAnimation(folderNameW, fileNameW, animNameW);

				show_anim_name_popup = false;
				ImGui::CloseCurrentPopup();
				show_convert_popup = true;
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				show_anim_name_popup = false;
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}
}
