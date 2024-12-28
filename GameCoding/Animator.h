#pragma once

struct Clip;
struct Transition;



struct Parameter
{
	enum class Type { Bool, Int, Float };
	string name;
	Type type;

	union Value
	{
		bool boolValue;
		int intValue;
		float floatValue;

		Value() : boolValue(false) {} // 기본 생성자
	} value;
};

struct Condition
{
	enum class CompareType { Equals, NotEqual, Greater, Less };
	string parameterName;
	Parameter::Type parameterType;
	CompareType compareType;

	union Value
	{
		bool boolValue;
		int intValue;
		float floatValue;
		Value() : boolValue(false) {}
	} value;
};

struct Transition
{
	weak_ptr<Clip> clipA;
	weak_ptr<Clip> clipB;
	bool flag = false;
	bool hasCondition = false;
	bool hasExitTime = false;
	float transitionOffset = 0.0f;
	float transitionDuration = 0.3f;
	vector<Condition> conditions;
};

struct Clip
{
	string name;
	bool isLoop = false;
	bool isEndFrame = false;
	int animIndex;
	float speed = 1.0f;
	float progressRatio = 0.0f;  // 0.0f ~ 1.0f 사이의 재생 진행률
	ImVec2 pos;  // Animator편집기의 노드 위치
	shared_ptr<Transition> transition;
	vector <shared_ptr<Transition>> transitions;
};



class Animator : public Component
{
	using Super = Component;
public:
	Animator();
	virtual ~Animator();
	virtual void Update() override;
	virtual void Start() override;

	void AddClip(const string& name, int animIndex, bool isLoop = false);
	void AddTransition(const string& clipAName, const string& clipBName);
	void SetTransitionFlag(shared_ptr<Transition> transition, bool flag);
	void SetTransitionHasExit(shared_ptr<Transition> transition, bool flag);
	void SetTransitionOffset(shared_ptr<Transition> transition, float offset);
	void SetTransitionDuration(shared_ptr<Transition> transition, float duration);
	void SetEntryClip(const string& clipName);
	void SetCurrentClip(const string& name) { _currClip = GetClip(name); }
	void SetCurrentTransition();
	void RemoveParameter(const string& name);
	void CheckConditionsAndSetFlag();

	void AddCondition(shared_ptr<Transition> transition, const string& paramName,
		Parameter::Type paramType, Condition::CompareType compareType);
	void RemoveCondition(shared_ptr<Transition> transition, int index);

	shared_ptr<Clip> GetClip(const string& name);

	void SetClipCurrentTransition(shared_ptr<Clip> clip);

	shared_ptr<Clip> _entry;
	shared_ptr<Clip> _currClip;
	shared_ptr<Transition> _currTransition;
	map<string, shared_ptr<Clip>> _clips;
	vector<shared_ptr<Transition>> _transitions;

public:
	void AddParameter(const string& name, Parameter::Type type);
	Parameter* GetParameter(const string& name);
	vector<Parameter> _parameters;

	// 파라미터 값 설정 메서드들
	void SetBool(const string& name, bool value);
	void SetInt(const string& name, int value);
	void SetFloat(const string& name, float value);

	// 파라미터 값 가져오기 메서드들
	bool GetBool(const string& name);
	int GetInt(const string& name);
	float GetFloat(const string& name);
};

