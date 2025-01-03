#pragma once
#include <filesystem>

class GUIManager
{
protected:
	GUIManager() {}
	virtual ~GUIManager() {}
public:
	GUIManager(const GUIManager&) = delete;
	static GUIManager& GetInstance()
	{
		static GUIManager instance;
		return instance;
	}
	void Init();
	void Update();
	void RenderUI_Start();
	void RenderUI();
	void RenderUI_End();
	void RenderGuizmo();
	void RenderGameObjectHierarchy(shared_ptr<GameObject> gameObject);
	
	void RenderFolderTree(const filesystem::path& path, filesystem::path& selectedFolder);
	void RenderFileGrid(const filesystem::path& path);

	void OnResourceDroppedToViewport(const std::string& fullPath);  // ����Ʈ�� ���ҽ��� ��ӵǾ��� �� ȣ��Ǵ� �Լ�

private:
	shared_ptr<GameObject> _selectedObject = nullptr;
	bool _isColliderEditMode = false;  // �ݶ��̴� ���� ������� ����
	bool _isCameraMoving = false;
	Vec3 _cameraStartPos;
	Vec3 _cameraTargetPos;
	Vec3 _cameraStartRot;
	Vec3 _cameraTargetRot;
	float _cameraMoveTime = 0.0f;
	const float _cameraMoveSpeed = 3.0f;  // �̵� �ӵ� (���� Ŭ���� ����)
	bool _isTransformChanged = false;
private:
	// EmptyObject ���� ���� ������
	bool _showEmptyObjectPopup = false;
	char _newObjectName[128] = "NewObject";
	Vec3 _newObjectPosition = Vec3::Zero;
	Vec3 _newObjectRotation = Vec3::Zero;
	Vec3 _newObjectScale = Vec3(1.0f, 1.0f, 1.0f);
	bool _newObjectUseMeshRenderer = false;

	// Resource ����Ʈ ���� ������
	bool _resourceListInitialized = false;
	vector<wstring> _meshList;
	vector<wstring> _materialList;
	int _newObjectSelectedMesh = 0;
	int _newObjectSelectedMaterial = 0;

	// ���� ȸ�� ���� ���� ����
	enum RotationAxis
	{
		NONE = -1,
		X_AXIS = 0,
		Y_AXIS = 1,
		Z_AXIS = 2
	};
	ImGuizmo::OPERATION currentGizmoOperation = ImGuizmo::TRANSLATE;
	ImGuizmo::MODE currentGizmoMode = ImGuizmo::LOCAL;
	RotationAxis currentRotationAxis = NONE;

	bool _rotationUpdated = false;
	filesystem::path _selectedFolder;
	bool _isFirstFrame = true;  // ù ������ üũ�� ���� ����

	shared_ptr<GameObject> _droppedObject;

private:

	// ��� ���� ������
	struct NodeData
	{
		string name;
		ImVec2 pos;
		bool isEntry = false;
		shared_ptr<Clip> clip;
	};
	vector<NodeData> _nodes;
	shared_ptr<Animator> _selectedAnimator;

	// Animator Editor ���� ������
	bool _showAnimatorEditor = false;
	string _newParameterName;
	enum class ParameterType { Bool, Int, Float };
	bool _showAddParameterPopup = false;
	ParameterType _selectedParameterType;
	NodeData* _selectedNode = nullptr;
	shared_ptr<Transition> _selectedTransition = nullptr;
	static constexpr float NODE_WIDTH = 150.0f;
	static constexpr float NODE_HEIGHT = 50.0f;
	static constexpr float TITLE_HEIGHT = 20.0f;

	// Ʈ������ ������ ���� ������
	bool _isCreatingTransition = false;
	NodeData* _transitionStartNode = nullptr;
	ImVec2 _transitionEndPos;

	// ��Ŭ�� �޴� ���� ������
	shared_ptr<Transition> _rightClickedTransition = nullptr;
	NodeData* _rightClickedNode = nullptr;
	bool _isAnyArrowHovered = false;  // ȭ��ǥ ȣ�� ���� ������ ���� ���� �߰�

	// ������ ���� �Լ���
	void ShowAnimatorEditor();
	void RenderParametersPanel();
	void RenderGraphPanel();
	void DrawArrow(ImDrawList* drawList, const ImVec2& start, const ImVec2& end, ImU32 color, float thickness);
	void RenderInspectorPanel();
	void RenderNode(NodeData& node);
	void RenderTransitions();
	void DrawConnection(ImDrawList* drawList, const ImVec2& start, const ImVec2& end);
	void DrawArrowHead(ImDrawList* drawList, const ImVec2& pos, float angle, ImU32 color);
	shared_ptr<Transition> GetTransitionFromPoints(const ImVec2& start, const ImVec2& end);
	float DistancePointToLineSegment(const ImVec2& point, const ImVec2& lineStart, const ImVec2& lineEnd);
	ImVec2 GetNodeCenterPos(shared_ptr<Clip> clip);
	bool IsOppositeTransition(const ImVec2& start1, const ImVec2& end1, const ImVec2& start2, const ImVec2& end2);
	bool IsCurrentTransitionNewer(shared_ptr<Transition> current, shared_ptr<Transition> other);

	void ClearAnimatorEditorData()
	{
		_nodes.clear();
		_selectedNode = nullptr;
		_selectedTransition = nullptr;
		_rightClickedNode = nullptr;
		_rightClickedTransition = nullptr;
	}
};

