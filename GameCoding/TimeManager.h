#pragma once

class TimeManager
{
protected:
	TimeManager() {}
	virtual ~TimeManager() {}
public:
	TimeManager(const TimeManager&) = delete;
	static TimeManager& GetInstance()
	{
		static TimeManager instance;
		return instance;
	}
	void Init();
	void Update();

	uint32 GetFps() { return _fps; }
	float GetDeltaTime() { return _isPaused ? 0.0f : _deltaTime; }
	float GetTotalTime() { return _totalTime; }
	void SetPause(bool pause) { _isPaused = pause; }
private:
	uint64	_frequency = 0;
	uint64	_prevCount = 0;
	float	_deltaTime = 0.f;

private:
	uint32	_frameCount = 0;
	float	_frameTime = 0.f;
	float	_totalTime = 0.f;
	uint32	_fps = 0;

private:
	bool _isPaused = false;
};

