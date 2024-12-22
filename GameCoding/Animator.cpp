#include "pch.h"
#include "Animator.h"

Animator::Animator()
	:Super(ComponentType::Animator)
{

}

Animator::~Animator()
{
}

void Animator::Start()
{
	AddClip("Clip1", 2, false);
	AddClip("Clip2", 0, false);
	AddClip("Clip3", 1, false);

	SetEntryClip("Clip1");

	AddTransition("Clip1", "Clip2");
	AddTransition("Clip2", "Clip3");
	//AddTransition("Clip3", "Clip1");
	//SetTransitionFlag(GetClip("Clip1")->transition, false);
	//SetTransitionFlag(GetClip("Clip2")->transition, false);
	SetCurrentClip("Clip1");
	SetCurrentTransition();
}

void Animator::Update()
{
	//SetTransitionFlag(GetClip("Clip1")->transition, GP.test);
	//SetTransitionFlag(GetClip("Clip2")->transition, GP.test2);
	//SetTransitionFlag(GetClip("Clip3")->transition, GP.test3);
}

void Animator::AddClip(const string& name, int animIndex, bool isLoop)
{
	shared_ptr<Clip> clip = make_shared<Clip>();
	clip->name = name;
	clip->animIndex = animIndex;
	clip->isLoop = isLoop;

	_clips[name] = clip;
}

void Animator::AddTransition(const string& clipAName, const string& clipBName)
{
	shared_ptr<Clip> clipA = GetClip(clipAName);
	shared_ptr<Clip> clipB = GetClip(clipBName);

	if (clipA && clipB)
	{
		shared_ptr<Transition> transition = make_shared<Transition>();
		transition->clipA = clipA;
		transition->clipB = clipB;
		transition->flag = false;
		transition->hasCondition = false;

		clipA->transition = transition;
		_transitions.push_back(transition);
	}
}
void Animator::SetTransitionFlag(shared_ptr<Transition> transition, bool flag)
{
	transition->flag = flag;
	transition->hasCondition = true;
}
void Animator::SetEntryClip(const string& clipName)
{
	_entry = GetClip(clipName);
}

void Animator::SetCurrentTransition()
{
	_currTransition = _currClip->transition;
}

shared_ptr<Clip> Animator::GetClip(const string& name)
{
    auto it = _clips.find(name);
    if (it != _clips.end())
        return it->second;
    return nullptr;
}
