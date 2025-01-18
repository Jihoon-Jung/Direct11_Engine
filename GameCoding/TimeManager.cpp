#include "pch.h"
#include "TimeManager.h"

void TimeManager::Init()
{
	::QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&_frequency));
	::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&_prevCount)); // CPU Ŭ��
}

void TimeManager::Update()
{
    uint64 currentCount;
    ::QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&currentCount));

    // Editor deltaTime�� �׻� ���
    _editorDeltaTime = (currentCount - _prevCount) / static_cast<float>(_frequency);

    // Edit ����̰ų� Pause ����� ���� deltaTime�� 0���� ����
    EngineMode mode = ENGINE.GetEngineMode();
    EngineMode mode2 = EngineMode::Play;
    if (_isEnginePause || _isPaused || ENGINE.GetEngineMode() == EngineMode::Edit)
    {
        _deltaTime = 0.0f;
        _prevCount = currentCount; // ��� ��ȯ �� ū ��ŸŸ���� �߻��ϴ� �� ����
        return;
    }

    _deltaTime = (currentCount - _prevCount) / static_cast<float>(_frequency);
    _prevCount = currentCount;

    _frameCount++;
    _frameTime += _deltaTime;
    _totalTime += _deltaTime;

    if (_frameTime > 1.f)
    {
        _fps = static_cast<uint32>(_frameCount / _frameTime);
        _frameTime = 0.f;
        _frameCount = 0;
    }
}

float TimeManager::GetDeltaTime()
{
    if (ENGINE.GetEngineMode() == EngineMode::Edit || ENGINE.GetEngineMode() == EngineMode::Pause)
        return 0.0f;
    return _deltaTime;
}
