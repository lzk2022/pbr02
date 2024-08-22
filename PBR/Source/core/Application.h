#pragma once
namespace core {
class Application final {
public:
	Application();
	void Init();
	void Load();
	void Run();
	void Clear();

public:
	inline static bool mIsExit = false;
private:
	void GetHardware();
	void CheckExit();
	void CheckSwitchLayer();
};
}

