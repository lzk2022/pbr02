#pragma once
#include <chrono>
#include <string>
namespace core {
	class Clock {
	public:
		static void Reset();
		static void Update();
		// 获取当前的 UTC 时间并以字符串形式返回
		static std::string GetDateTimeUTC();

	public:
		static float DeltaTime() { return mDeltaTime; }
		static float Time() { return mTime; }
		static float Fps() { return mFps; }
		static float Ms() { return mMs; }

	public:
		static std::chrono::time_point<std::chrono::system_clock> mStartTime;   // 记录程序启动时的系统时间点
		inline static float mLastFrame = 0.0f;      // 上一帧的时间戳
		inline static float mThisFrame = 0.0f;      // 当前帧的时间戳
		inline static int mFrameCount = 0;          // 帧计数
		inline static float mDuration = 0.0f;       // 累计时间

		inline static float mDeltaTime = 0.0f;      // 时间间隔
		inline static float mTime = 0.0f;           // 总经过时间
		inline static float mFps = 0.0f;            // 每秒帧数（每0.1秒采样一次）
		inline static float mMs = 0.0f;             // 每帧毫秒数
	};
}

