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
	

	/*AddClip("Clip1", 2, false);
	AddClip("Clip2", 0, false);
	AddClip("Clip3", 1, false);

	SetEntryClip("Clip1");

	AddTransition("Clip1", "Clip2");
	AddTransition("Clip2", "Clip3");
	AddTransition("Clip3", "Clip1");
	SetTransitionFlag(GetClip("Clip1")->transition, false);
	SetTransitionFlag(GetClip("Clip2")->transition, false);

	SetCurrentTransition();*/

	
}

void Animator::Update()
{
	/*SetTransitionFlag(GetClip("Clip1")->transition, GP.test);
	SetTransitionFlag(GetClip("Clip2")->transition, GP.test2);*/

	if (_model.expired())
	{
		_model = GetGameObject()->GetComponent<MeshRenderer>()->GetModel();
		if (_model.expired())  // model을 가져오지 못한 경우
			return;
	}


	shared_ptr<Clip> currClip = _currClip;
	shared_ptr<Transition> currTransition = currClip->transition;// animator->_currTransition;
	int currIndex = currClip->animIndex;
	int nextIndex = currTransition != nullptr ? currTransition->clipB.lock()->animIndex : INT_MAX;
	_blendAnimDesc.SetAnimIndex(currIndex, nextIndex);
	_blendAnimDesc.curr.speed = currClip->speed;

	// 현재 애니메이션 업데이트
	shared_ptr<ModelAnimation> current = _model.lock()->GetAnimationByIndex(_blendAnimDesc.curr.animIndex);
	if (current)
	{
		// 현재 프레임의 시간 계산 (프레임 수를 프레임 레이트로 나누어 초 단위로 변환)
		float clipLength = (current->frameCount - 1) / current->frameRate;
		float currentTime = currClip->progressRatio * clipLength;

		float currentRatio = currClip->progressRatio;
		float frameStep = 1.0f / current->frameCount;  // 한 프레임당 증가량

		// 이벤트 체크
		for (auto& event : currClip->events)
		{
			// 현재 프레임 구간에 이벤트가 있는지 확인
			if (!event.isFuctionCalled && event.time > (currentRatio - frameStep) && event.time <= currentRatio)
			{
				InvokeAnimationEvent(event.function);
				event.isFuctionCalled = true;
			}
		}

		currClip->progressRatio = static_cast<float>(_blendAnimDesc.curr.currFrame) / (current->frameCount - 1);

		// exitTime 도달 여부 체크
		if (currClip->transition && currClip->transition->hasExitTime)
		{
			if (currClip->progressRatio >= currClip->transition->exitTime)
			{
				currClip->isEndFrame = true;
			}
		}

		float timePerFrame = 1 / (current->frameRate * _blendAnimDesc.curr.speed);
		_blendAnimDesc.curr.sumTime += TIME.GetDeltaTime();

		// 한 프레임이 끝났는지 체크
		if (_blendAnimDesc.curr.sumTime >= timePerFrame)
		{
			_blendAnimDesc.curr.sumTime = 0.f;

			// 마지막 프레임 체크
			if (_blendAnimDesc.curr.currFrame >= current->frameCount - 1)
			{
				currClip->isEndFrame = true;

				for (auto& event : currClip->events)
				{
					if (event.isFuctionCalled)
						event.isFuctionCalled = false;
				}

				if (currClip->isLoop)
				{
					_blendAnimDesc.curr.currFrame = 0;
					_blendAnimDesc.curr.nextFrame = 1;
				}
				else
				{
					_blendAnimDesc.curr.currFrame = current->frameCount - 1;
					_blendAnimDesc.curr.nextFrame = current->frameCount - 1;
				}
			}
			else
			{
				_blendAnimDesc.curr.currFrame++;
				_blendAnimDesc.curr.nextFrame = min(_blendAnimDesc.curr.currFrame + 1, current->frameCount - 1);

			}
		}

		_blendAnimDesc.curr.ratio = (_blendAnimDesc.curr.sumTime / timePerFrame);
	}

	// 트랜지션 처리
	if (currTransition != nullptr)
	{
		// Case 1: Has Exit Time O + Condition O
		if (currTransition->hasExitTime && currTransition->hasCondition)
		{
			if (currClip->isEndFrame && currTransition->flag)
			{
				HandleTransitionBlend(currTransition);
			}
		}
		// Case 2: Has Exit Time O + Condition X
		else if (currTransition->hasExitTime && !currTransition->hasCondition)
		{
			if (currClip->isEndFrame)
			{
				HandleTransitionBlend(currTransition);
			}
		}
		// Case 3: Has Exit Time X + Condition O
		else if (!currTransition->hasExitTime && currTransition->hasCondition)
		{
			if (currTransition->flag)
			{
				HandleTransitionBlend(currTransition);
			}
		}
		// Case 4: Has Exit Time X + Condition X는 의미 없으므로 구현하지 않음
	}

	_blendAnimDesc.curr.activeAnimation = 1;
	_blendAnimDesc.next.activeAnimation = 1;


	UpdateBoneTransforms();
	
}

void Animator::AddClip(const string& name, int animIndex, bool isLoop)
{
	string uniqueName = name;

	// 이름이 이미 존재하는 경우 처리
	if (_clips.find(uniqueName) != _clips.end())
	{
		// 기본 이름과 숫자 부분 분리
		size_t underscorePos = name.rfind('_');
		string baseName = name;
		int number = 1;

		// 이미 _숫자 형식이 있는지 확인
		if (underscorePos != string::npos)
		{
			string numberStr = name.substr(underscorePos + 1);
			// 숫자로만 이루어져 있는지 확인
			bool isNumber = !numberStr.empty() &&
				std::all_of(numberStr.begin(), numberStr.end(), ::isdigit);

			if (isNumber)
			{
				baseName = name.substr(0, underscorePos);
				number = std::stoi(numberStr) + 1;
			}
		}

		// 사용 가능한 이름을 찾을 때까지 반복
		do {
			uniqueName = baseName + "_" + std::to_string(number);
			number++;
		} while (_clips.find(uniqueName) != _clips.end());
	}

	shared_ptr<Clip> clip = make_shared<Clip>();
	clip->name = uniqueName;
	clip->animIndex = animIndex;
	clip->isLoop = isLoop;

	_clips[uniqueName] = clip;
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
		transition->hasExitTime = true;
		transition->hasCondition = false;

		//clipA->transition = transition;
		clipA->transitions.push_back(transition);
		if (_currClip->name == clipAName || _currClip->name == clipBName)
			SetCurrentTransition();

		_transitions.push_back(transition);
	}
	SetClipCurrentTransition(clipA);
}
void Animator::SetTransitionFlag(shared_ptr<Transition> transition, bool flag)
{
	transition->flag = flag;
	transition->hasCondition = true;
}
void Animator::SetEntryClip(const string& clipName)
{
	_entry = GetClip(clipName);
	SetCurrentClip(clipName);
}

void Animator::SetCurrentTransition()
{
	_currTransition = _currClip->transition;
}

void Animator::SetTransitionHasExit(shared_ptr<Transition> transition, bool flag)
{
	transition->hasExitTime = flag;
}

void Animator::SetTransitionExitTime(shared_ptr<Transition> transition, float exitTime)
{
	transition->exitTime = exitTime;
}

void Animator::SetTransitionOffset(shared_ptr<Transition> transition, float offset)
{
	transition->transitionOffset = offset;
}

void Animator::SetTransitionDuration(shared_ptr<Transition> transition, float duration)
{
	transition->transitionDuration = duration;
}



void Animator::HandleTransitionBlend(shared_ptr<Transition>& transition)
{
	// transitionOffset 적용: 다음 애니메이션의 시작 시점 조절
	if (auto model = _model.lock())
	{
		if (_blendAnimDesc.blendSumTime == 0.0f)  // 블렌딩 시작 시
		{
			shared_ptr<ModelAnimation> next = model->GetAnimationByIndex(_blendAnimDesc.next.animIndex);
			if (next)
			{
				// Offset 위치로 다음 애니메이션 시작 프레임 설정
				float offsetFrame = next->frameCount * transition->transitionOffset;
				_blendAnimDesc.next.currFrame = static_cast<int>(offsetFrame);
				_blendAnimDesc.next.nextFrame = (_blendAnimDesc.next.currFrame + 1) % next->frameCount;
				_blendAnimDesc.next.sumTime = 0.f;
			}
		}

		_blendAnimDesc.blendSumTime += TIME.GetDeltaTime();
		_blendAnimDesc.blendRatio = _blendAnimDesc.blendSumTime / transition->transitionDuration;

		if (_blendAnimDesc.blendRatio > 1.0f)
		{
			animationSumTime = 0.0f;
			_blendAnimDesc.ClearNextAnim(transition->clipB.lock()->animIndex);

			// 현재 클립의 isEndFrame 초기화
			if (auto currClip = _currClip)
			{
				currClip->isEndFrame = false;
				for (auto& event : currClip->events)
				{
					if (event.isFuctionCalled)
						event.isFuctionCalled = false;
				}
			}


			// 다음 클립의 isEndFrame도 초기화
			if (auto nextClip = GetClip(transition->clipB.lock()->name))
				nextClip->isEndFrame = false;

			SetCurrentClip(transition->clipB.lock()->name);
			SetCurrentTransition();

		}
		else
		{
			// 다음 애니메이션 업데이트
			shared_ptr<ModelAnimation> next = model->GetAnimationByIndex(_blendAnimDesc.next.animIndex);
			if (next)
			{
				// 다음 클립의 진행률 업데이트
				if (auto nextClip = transition->clipB.lock())
				{
					nextClip->progressRatio = static_cast<float>(_blendAnimDesc.next.currFrame) / (next->frameCount - 1);
				}

				_blendAnimDesc.next.sumTime += TIME.GetDeltaTime();
				float timePerFrame = 1 / (next->frameRate * _blendAnimDesc.next.speed);

				if (_blendAnimDesc.next.ratio >= 1.0f)
				{
					_blendAnimDesc.next.sumTime = 0.f;
					_blendAnimDesc.next.currFrame = (_blendAnimDesc.next.currFrame + 1) % next->frameCount;
					_blendAnimDesc.next.nextFrame = (_blendAnimDesc.next.currFrame + 1) % next->frameCount;
				}

				_blendAnimDesc.next.ratio = (_blendAnimDesc.next.sumTime / timePerFrame);
			}
		}
	}
	
}

void Animator::AddParameter(const string& name, Parameter::Type type)
{
	Parameter param;
	param.name = name;
	param.type = type;

	// 기본값 설정
	switch (type)
	{
	case Parameter::Type::Bool:
		param.value.boolValue = false;
		break;
	case Parameter::Type::Int:
		param.value.intValue = 0;
		break;
	case Parameter::Type::Float:
		param.value.floatValue = 0.0f;
		break;
	}

	_parameters.push_back(param);
}

Parameter* Animator::GetParameter(const string& name)
{
	auto it = std::find_if(_parameters.begin(), _parameters.end(),
		[&name](const Parameter& param) { return param.name == name; });

	return it != _parameters.end() ? &(*it) : nullptr;
}

shared_ptr<Clip> Animator::GetClip(const string& name)
{
    auto it = _clips.find(name);
    if (it != _clips.end())
        return it->second;
    return nullptr;
}
void Animator::SetBool(const string& name, bool value)
{
	if (auto param = GetParameter(name))
	{
		if (param->type == Parameter::Type::Bool)
		{
			param->value.boolValue = value;
			CheckConditionsAndSetFlag();
		}
			
	}
}

void Animator::SetInt(const string& name, int value)
{
	if (auto param = GetParameter(name))
	{
		if (param->type == Parameter::Type::Int)
		{
			param->value.intValue = value;
			CheckConditionsAndSetFlag();
		}
	}
}

void Animator::SetFloat(const string& name, float value)
{
	if (auto param = GetParameter(name))
	{
		if (param->type == Parameter::Type::Float)
		{
			param->value.floatValue = value;
			CheckConditionsAndSetFlag();
		}
	}
}

bool Animator::GetBool(const string& name)
{
	if (auto param = GetParameter(name))
	{
		if (param->type == Parameter::Type::Bool)
			return param->value.boolValue;
	}
	return false;
}

void Animator::CheckConditionsAndSetFlag()
{
	for (shared_ptr<Transition> transition : _transitions)
	{
		bool prevFlag = transition->flag;
		bool isAllConditionSatisfy = true;

		for (const Condition& condition : transition->conditions)
		{
			bool conditionSatisfied = false;

			// 파라미터 값 가져오기
			switch (condition.parameterType)
			{
			case Parameter::Type::Bool:
			{
				bool paramValue = GetBool(condition.parameterName);
				bool conditionValue = condition.value.boolValue;

				switch (condition.compareType)
				{
				case Condition::CompareType::Equals:
					conditionSatisfied = (paramValue == conditionValue);
					break;
				case Condition::CompareType::NotEqual:
					conditionSatisfied = (paramValue != conditionValue);
					break;
				}
				break;
			}
			case Parameter::Type::Int:
			{
				int paramValue = GetInt(condition.parameterName);
				int conditionValue = condition.value.intValue;

				switch (condition.compareType)
				{
				case Condition::CompareType::Equals:
					conditionSatisfied = (paramValue == conditionValue);
					break;
				case Condition::CompareType::NotEqual:
					conditionSatisfied = (paramValue != conditionValue);
					break;
				case Condition::CompareType::Greater:
					conditionSatisfied = (paramValue > conditionValue);
					break;
				case Condition::CompareType::Less:
					conditionSatisfied = (paramValue < conditionValue);
					break;
				}
				break;
			}
			case Parameter::Type::Float:
			{
				float paramValue = GetFloat(condition.parameterName);
				float conditionValue = condition.value.floatValue;

				switch (condition.compareType)
				{
				case Condition::CompareType::Equals:
					conditionSatisfied = (abs(paramValue - conditionValue) < 0.0001f);
					break;
				case Condition::CompareType::NotEqual:
					conditionSatisfied = (abs(paramValue - conditionValue) >= 0.0001f);
					break;
				case Condition::CompareType::Greater:
					conditionSatisfied = (paramValue > conditionValue);
					break;
				case Condition::CompareType::Less:
					conditionSatisfied = (paramValue < conditionValue);
					break;
				}
				break;
			}
			}

			// 하나의 조건이라도 만족하지 않으면 전체 조건은 실패
			if (!conditionSatisfied)
			{
				isAllConditionSatisfy = false;
				break;
			}
		}

		if (isAllConditionSatisfy)
		{
			transition->flag = true;
		}
		else
			transition->flag = false;

		SetClipCurrentTransition(transition->clipA.lock());
		
		if (prevFlag != transition->flag)
		{
			if (auto clipA = transition->clipA.lock())
				if (auto clipB = transition->clipB.lock())
					SCENE.UpdateAnimatorTransitionFlagInXML(
						SCENE.GetActiveScene()->GetSceneName(),
						GetGameObject()->GetName(),
						clipA->name, clipB->name,
						transition->flag, transition->hasCondition);
		}
	}
}

void Animator::InvokeAnimationEvent(const std::string& functionName)
{
	// Animator가 달려있는 GameObject 취득
	auto owner = GetGameObject();
	if (!owner)
		return;

	// 1) owner->GetComponents() 순회
	for (auto& comp : owner->GetComponents())
	{
		// 2) MonoBehaviour* 캐스팅
		MonoBehaviour* mb = dynamic_cast<MonoBehaviour*>(comp.get());
		if (mb == nullptr)
			continue;

		// 3) 현재 MonoBehaviour의 C++ RTTI 이름
		std::string className = typeid(*mb).name();
		// 4) 레지스트리에서 조회할 키를 만든다: "MoveObject::TestLog"
		std::string key = className + "::" + functionName;

		// 5) 실제 함수 호출 람다를 구해 옴
		auto method = MR.getMethod(key);
		if (method)
		{
			// 6) 함수 호출
			method(mb);
		}
	}
}

void Animator::InvokeAnimationEvent(const AvailableFunction& function)
{
	if (auto method = MR.getMethod(function.functionKey))
	{
		method(function.script);
	}
}

vector<AvailableFunction> Animator::GetAvailableFunctions()
{
	vector<AvailableFunction> availableFunctions;

	auto owner = GetGameObject();
	if (!owner)
		return availableFunctions;

	const auto& registeredMethods = MR.GetAllMethods();

	for (auto& comp : owner->GetComponents())
	{
		MonoBehaviour* mb = dynamic_cast<MonoBehaviour*>(comp.get());
		if (!mb)
			continue;

		string className = typeid(*mb).name();

		for (const auto& [key, method] : registeredMethods)
		{
			if (key.find(className + "::") == 0)
			{
				availableFunctions.push_back({ mb, key });
			}
		}
	}

	return availableFunctions;
}

int Animator::GetInt(const string& name)
{
	if (auto param = GetParameter(name))
	{
		if (param->type == Parameter::Type::Int)
			return param->value.intValue;
	}
	return 0;
}

float Animator::GetFloat(const string& name)
{
	if (auto param = GetParameter(name))
	{
		if (param->type == Parameter::Type::Float)
			return param->value.floatValue;
	}
	return 0.0f;
}

void Animator::AddCondition(shared_ptr<Transition> transition, const string& paramName,
	Parameter::Type paramType, Condition::CompareType compareType)
{
	if (!transition)
		return;

	Condition newCondition;
	newCondition.parameterName = paramName;
	newCondition.parameterType = paramType;
	newCondition.compareType = compareType;
	transition->conditions.push_back(newCondition);
	transition->hasCondition = true;

}

void Animator::RemoveCondition(shared_ptr<Transition> transition, int index)
{
	if (!transition || index < 0 || index >= transition->conditions.size())
		return;

	transition->conditions.erase(transition->conditions.begin() + index);

	if (transition->conditions.size() == 0)
		transition->hasCondition = false;
}

void Animator::RemoveParameter(const string& name)
{
	auto it = std::find_if(_parameters.begin(), _parameters.end(),
		[&name](const Parameter& param) { return param.name == name; });

	if (it != _parameters.end())
	{
		// 파라미터와 관련된 모든 트랜지션의 조건들도 제거
		for (auto& transition : _transitions)
		{
			transition->conditions.erase(
				std::remove_if(transition->conditions.begin(), transition->conditions.end(),
					[&name](const Condition& condition) {
						return condition.parameterName == name;
					}
				),
				transition->conditions.end()
						);

			// 조건이 모두 제거되었다면 hasCondition 플래그 업데이트
			if (transition->conditions.empty())
				transition->hasCondition = false;
		}

		// 파라미터 제거
		_parameters.erase(it);
	}
}

void Animator::SetClipCurrentTransition(shared_ptr<Clip> clip)
{
	if (!clip)
		return;

	// 1. hasCondition이 true인 트랜지션들 찾기
	vector<shared_ptr<Transition>> conditionTransitions;
	for (const auto& transition : clip->transitions)
	{
		if (transition->hasCondition)
			conditionTransitions.push_back(transition);
	}

	// 조건이 있는 트랜지션이 존재하는 경우
	if (!conditionTransitions.empty())
	{
		if (conditionTransitions.size() > 1)
		{
			// HasExitTime이 false인 트랜지션들 찾기
			vector<shared_ptr<Transition>> noExitTransitions;
			for (const auto& transition : conditionTransitions)
			{
				if (!transition->hasExitTime)
					noExitTransitions.push_back(transition);
			}

			if (!noExitTransitions.empty())
			{
				// HasExitTime이 false인 트랜지션 중 flag가 true인 것들 찾기
				vector<shared_ptr<Transition>> flaggedTransitions;
				for (const auto& transition : noExitTransitions)
				{
					if (transition->flag)
						flaggedTransitions.push_back(transition);
				}

				if (flaggedTransitions.size() > 1)
				{
					clip->transition = flaggedTransitions[0]; // 먼저 넣은 것 사용
				}
				else if (flaggedTransitions.size() == 1)
				{
					clip->transition = flaggedTransitions[0];
				}
				else
				{
					clip->transition = nullptr;  // flag가 true인 것이 없음
				}
			}
			else
			{
				// HasExitTime이 true인 트랜지션들 중 flag가 true인 것들 찾기
				vector<shared_ptr<Transition>> flaggedTransitions;
				for (const auto& transition : conditionTransitions)
				{
					if (transition->flag)
						flaggedTransitions.push_back(transition);
				}

				if (flaggedTransitions.size() > 1)
				{
					clip->transition = flaggedTransitions[0];
				}
				else if (flaggedTransitions.size() == 1)
				{
					clip->transition = flaggedTransitions[0];
				}
				else
				{
					clip->transition = nullptr;
				}
			}
		}
		else
		{
			// 트랜지션이 1개인 경우 flag가 true일 때만 설정
			if (conditionTransitions[0]->flag)
				clip->transition = conditionTransitions[0];
			else
				clip->transition = nullptr;
		}
	}
	else
	{
		// 조건이 없는 트랜지션들 중 hasExitTime이 true인 것들 찾기
		vector<shared_ptr<Transition>> exitTimeTransitions;
		for (const auto& transition : clip->transitions)
		{
			if (transition->hasExitTime)
				exitTimeTransitions.push_back(transition);
		}

		if (exitTimeTransitions.size() > 1)
		{
			clip->transition = exitTimeTransitions[0];
		}
		else if (exitTimeTransitions.size() == 1)
		{
			clip->transition = exitTimeTransitions[0];
		}
		else
		{
			clip->transition = nullptr;
		}
	}
}

void Animator::UpdateBoneTransforms()
{
	if (auto model = _model.lock())  // weak_ptr를 shared_ptr로 변환
	{
		if (!_currClip)
			return;

		shared_ptr<Transition> currTransition = _currClip->transition;
		bool isBlending = (currTransition != nullptr && _blendAnimDesc.blendSumTime > 0.0f);

		for (int boneIndex : GetGameObject()->GetActiveBoneIndices())
		{
			const auto& bone = model->GetBoneByIndex(boneIndex);
			shared_ptr<GameObject> boneObject = FindBoneObjectByIndex(bone->index);
			if (!boneObject || bone->index < 0)
				continue;

			Matrix finalTransform;

			if (isBlending)
			{
				// 현재 애니메이션 보간값 계산
				Matrix curr1 = model->GetAnimationTransform(bone->index, _blendAnimDesc.curr.currFrame, _blendAnimDesc.curr.animIndex);
				Matrix next1 = model->GetAnimationTransform(bone->index, _blendAnimDesc.curr.nextFrame, _blendAnimDesc.curr.animIndex);
				Matrix anim1 = Matrix::Lerp(curr1, next1, _blendAnimDesc.curr.ratio);

				// 다음 애니메이션 보간값 계산
				Matrix curr2 = model->GetAnimationTransform(bone->index, _blendAnimDesc.next.currFrame, _blendAnimDesc.next.animIndex);
				Matrix next2 = model->GetAnimationTransform(bone->index, _blendAnimDesc.next.nextFrame, _blendAnimDesc.next.animIndex);
				Matrix anim2 = Matrix::Lerp(curr2, next2, _blendAnimDesc.next.ratio);

				// 두 애니메이션 간 보간
				Matrix blendedAnim = Matrix::Lerp(anim1, anim2, _blendAnimDesc.blendRatio);
				finalTransform = bone->transform * blendedAnim;
			}
			else
			{
				// 단일 애니메이션 적용
				Matrix curr = model->GetAnimationTransform(bone->index, _blendAnimDesc.curr.currFrame, _blendAnimDesc.curr.animIndex);
				Matrix next = model->GetAnimationTransform(bone->index, _blendAnimDesc.curr.nextFrame, _blendAnimDesc.curr.animIndex);
				Matrix animTransform = Matrix::Lerp(curr, next, _blendAnimDesc.curr.ratio);
				finalTransform = bone->transform * animTransform;
			}

			Vec3 scale, translation;
			Quaternion rotation;
			finalTransform.Decompose(scale, rotation, translation);

			auto boneTransform = boneObject->transform();
			boneTransform->SetLocalPosition(translation);
			boneTransform->SetQTRotation(rotation);
			boneTransform->SetLocalScale(scale);
		}
	}
}

shared_ptr<GameObject> Animator::FindBoneObjectByIndex(int index)
{
	auto it = _boneObjects.find(index);
	if (it != _boneObjects.end())
	{
		return it->second.lock();
	}
	return nullptr;
}