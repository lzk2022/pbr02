#include "Clock.h"
#include "../utils/Log.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <date/date.h>

namespace core {
    decltype(Clock::mStartTime) Clock::mStartTime = std::chrono::system_clock::now();
	void Clock::Reset()
	{
		LOG_TRACK;
		mStartTime = std::chrono::system_clock::now();
		mLastFrame = mThisFrame = mDeltaTime = mTime = 0.0f;
		mFps = mMs = 0.0f;
		mFrameCount = 0;
		mDuration = 0.0f;
	}
	void Clock::Update()
	{
        mThisFrame = static_cast<float>(glfwGetTime());  // 获取当前帧时间

        mDeltaTime = mThisFrame - mLastFrame;  // 计算时间间隔
        mLastFrame = mThisFrame;

        // 对于固定时间间隔的设备（例如定时器和秒表），使用 delta time 更容易，但会受到浮点精度问题的影响。
        // 当经过的时间变成非常大的浮点数时（例如 99:59:59 需要 359,999 秒），并且反复添加或减去非常小的数字
        // （delta time），舍入误差会随着时间的推移累积，导致意外的结果。

        // 可以通过使用双精度浮点数（double）来部分缓解这个问题，因为它具有更高的精度和分辨率，减少了舍入误差，
        // 但仍有一定限度。为了确保稳健性，我们必须将真实时间与固定的起始时间戳进行比较。

        if constexpr (true) {
            mTime = mThisFrame;     // 更新总时间（单调递增）
        }
        else {
            mTime += mDeltaTime;    // 永远不要这样做！注意舍入误差...
        }

        // 计算帧速率
        mFrameCount++;
        mDuration += mDeltaTime;

        if (mDuration >= 0.1f) {
            mFps = mFrameCount / mDuration;            // 计算帧速率
            mMs = 1000.0f * mDuration / mFrameCount;   // 计算每帧平均毫秒数
            mFrameCount = 0;
            mDuration = 0.0f;
        }
    }
    std::string Clock::GetDateTimeUTC()
    {
        namespace chrono = std::chrono;

        // 与 glfw 内部时钟保持同步
        auto now = mStartTime + chrono::seconds((int)mTime);
        auto sysdate = date::floor<date::days>(now);
        auto systime = date::make_time(chrono::duration_cast<chrono::seconds>(now - sysdate));

        int hour = systime.hours().count();         // 小时
        int minute = systime.minutes().count();     // 分钟
        int second = systime.seconds().count();     // 秒钟

        // 将单个数字前补 '0'
        static auto format = [](int i) {
            std::ostringstream sstr;
            sstr << std::setw(2) << std::setfill('0') << i;
            return sstr.str();
            };

        std::string YYYY_MM_DD = date::format("%Y-%m-%d", sysdate);                 // 年-月-日
        std::string HH24_MM_SS = format(hour) + format(minute) + format(second);    // 时分秒

        return YYYY_MM_DD + "-" + HH24_MM_SS;  // 返回完整的日期时间字符串
    }
}