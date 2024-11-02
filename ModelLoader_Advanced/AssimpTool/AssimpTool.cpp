#include "pch.h"
#include "AssimpTool.h"
#include "Converter.h"

void AssimpTool::Init(HWND hwnd)
{
	//{
	//	shared_ptr<Converter> converter = make_shared<Converter>();
	//	converter->init(hwnd);
	//	// FBX -> Memory
	//	converter->ReadAssetFile(L"Tank/Tank.fbx");

	//	// Memory -> CustomData (File)
	//	converter->ExportMaterialData(L"Tank/Tank");
	//	converter->ExportModelData(L"Tank/Tank");

	//	// CustomData (File) -> Memory
	//}
	//{
	//	shared_ptr<Converter> converter = make_shared<Converter>();
	//	converter->init(hwnd);
	//	// FBX -> Memory
	//	converter->ReadAssetFile(L"Dragon/dragon.fbx");

	//	// Memory -> CustomData (File)
	//	converter->ExportMaterialData(L"Dragon/Dragon");
	//	converter->ExportModelData(L"Dragon/Dragon");

	//	// CustomData (File) -> Memory
	//}
	/*{
		shared_ptr<Converter> converter = make_shared<Converter>();
		converter->ReadAssetFile(L"Kachujin/Mesh.fbx");
		converter->ExportMaterialData(L"Kachujin/Kachujin");
		converter->ExportModelData(L"Kachujin/Kachujin");
	}
	{
		shared_ptr<Converter> converter = make_shared<Converter>();
		converter->ReadAssetFile(L"Kachujin/Idle.fbx");
		converter->ExportAnimationData(L"Kachujin/Idle");
	}*/
	/*{
		shared_ptr<Converter> converter = make_shared<Converter>();
		converter->ReadAssetFile(L"Kachujin/Run.fbx");
		converter->ExportAnimationData(L"Kachujin/Run");
	}*/
	/*{
		shared_ptr<Converter> converter = make_shared<Converter>();
		converter->ReadAssetFile(L"Kachujin/Slash.fbx");
		converter->ExportAnimationData(L"Kachujin/Slash");
		int a = 1;
	}*/
	/*{
		shared_ptr<Converter> converter = make_shared<Converter>();
		converter->ReadAssetFile(L"Kachujin/Dismissing Gesture.fbx");
		converter->ExportAnimationData(L"Kachujin/Dismissing");
	}*/
	/*{
		shared_ptr<Converter> converter = make_shared<Converter>();
		converter->ReadAssetFile(L"Ely/Ely By K.Atienza.fbx");
		converter->ExportMaterialData(L"Ely/Ely");
		converter->ExportModelData(L"Ely/Ely");
	}
	{
		shared_ptr<Converter> converter = make_shared<Converter>();
		converter->ReadAssetFile(L"Ely/Martelo 2.fbx");
		converter->ExportAnimationData(L"Ely/Stab");
	}*/
	/*{
		shared_ptr<Converter> converter = make_shared<Converter>();
		converter->ReadAssetFile(L"Dreyar/Dreyar By M.Aure.fbx");
		converter->ExportMaterialData(L"Dreyar/Dreyar");
		converter->ExportModelData(L"Dreyar/Dreyar");
	}
	{
		shared_ptr<Converter> converter = make_shared<Converter>();
		converter->ReadAssetFile(L"Dreyar/Mma Kick.fbx");
		converter->ExportAnimationData(L"Dreyar/Kick");
	}
	{
		shared_ptr<Converter> converter = make_shared<Converter>();
		converter->ReadAssetFile(L"Dreyar/Fall Flat.fbx");
		converter->ExportAnimationData(L"Dreyar/Fall");
	}
	{
		shared_ptr<Converter> converter = make_shared<Converter>();
		converter->ReadAssetFile(L"Dreyar/Breakdance 1990.fbx");
		converter->ExportAnimationData(L"Dreyar/Dance");
	}*/

	{
		shared_ptr<Converter> converter = make_shared<Converter>();
		converter->init(hwnd);
		// FBX -> Memory
		converter->ReadAssetFile(L"Sponza/Sponza.fbx");

		// Memory -> CustomData (File)
		converter->ExportMaterialData(L"Sponza/Sponza");
		converter->ExportModelData(L"Sponza/Sponza");

		// CustomData (File) -> Memory
	}
}

void AssimpTool::Update()
{

}

void AssimpTool::Render()
{

}
