#pragma once

struct Clip;
struct Transition;

struct Transition
{
	weak_ptr<Clip> clipA;
	weak_ptr<Clip> clipB;
	bool flag = false;
	bool hasCondition = false;
	bool hasExitTime = true;
	float transitionOffset = 0.0f;
	float transitionDuration = 1.0f;
};

struct Clip
{
	string name;
	bool isLoop = false;
	bool isEndFrame = false;
	int animIndex;
	shared_ptr<Transition> transition;
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
	void SetEntryClip(const string& clipName);
	void SetCurrentClip(const string& name) { _currClip = GetClip(name); }
	void SetCurrentTransition();

	shared_ptr<Clip> GetClip(const string& name);

	shared_ptr<Clip> _entry;
	shared_ptr<Clip> _currClip;
	shared_ptr<Transition> _currTransition;
	map<string, shared_ptr<Clip>> _clips;
	vector<shared_ptr<Transition>> _transitions;
};

