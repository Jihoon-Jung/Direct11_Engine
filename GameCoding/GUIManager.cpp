#include "pch.h"
#include "GUIManager.h"

void GUIManager::Init()
{
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
	ImGui_ImplWin32_Init(GP.GetWindowHandle());
	ImGui_ImplDX11_Init(GP.GetDevice().Get(), GP.GetDeviceContext().Get());
}

void GUIManager::Update()
{
	RenderUI_Start();
	RenderUI();
	RenderUI_End();
}

void GUIManager::RenderUI_Start()
{
	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void GUIManager::RenderUI()
{
	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	// Fixed Position Window
	{
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
		ImGui::SetNextWindowSize(ImVec2(GP.GetViewWidth(), GP.GetViewHeight() / 6.0f));
		ImGui::Begin("Manual", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
		ImGui::Text("This tool converts FBX files to make them usable in our project.\n"
			"Select the FBX file in the folder shown in the window below, right-click it, and if it is a Mesh, click Add Mesh.\n"
			"If it is an Animation, click Add Animation.\n"
			"When the pop-up window displays \"Convert Finish!\", the process is complete.\nPlease note that we currently only guarantee conversion for FBX files provided by Mixamo \n(in the case of Animation).");
		ImGui::End();
	}
}

void GUIManager::RenderUI_End()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
