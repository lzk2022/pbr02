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
        mThisFrame = static_cast<float>(glfwGetTime());  // ��ȡ��ǰ֡ʱ��

        mDeltaTime = mThisFrame - mLastFrame;  // ����ʱ����
        mLastFrame = mThisFrame;

        // ���ڹ̶�ʱ�������豸�����綨ʱ���������ʹ�� delta time �����ף������ܵ����㾫�������Ӱ�졣
        // ��������ʱ���ɷǳ���ĸ�����ʱ������ 99:59:59 ��Ҫ 359,999 �룩�����ҷ�����ӻ��ȥ�ǳ�С������
        // ��delta time����������������ʱ��������ۻ�����������Ľ����

        // ����ͨ��ʹ��˫���ȸ�������double�������ֻ���������⣬��Ϊ�����и��ߵľ��Ⱥͷֱ��ʣ�������������
        // ������һ���޶ȡ�Ϊ��ȷ���Ƚ��ԣ����Ǳ��뽫��ʵʱ����̶�����ʼʱ������бȽϡ�

        if constexpr (true) {
            mTime = mThisFrame;     // ������ʱ�䣨����������
        }
        else {
            mTime += mDeltaTime;    // ��Զ��Ҫ��������ע���������...
        }

        // ����֡����
        mFrameCount++;
        mDuration += mDeltaTime;

        if (mDuration >= 0.1f) {
            mFps = mFrameCount / mDuration;            // ����֡����
            mMs = 1000.0f * mDuration / mFrameCount;   // ����ÿ֡ƽ��������
            mFrameCount = 0;
            mDuration = 0.0f;
        }
    }
    std::string Clock::GetDateTimeUTC()
    {
        namespace chrono = std::chrono;

        // �� glfw �ڲ�ʱ�ӱ���ͬ��
        auto now = mStartTime + chrono::seconds((int)mTime);
        auto sysdate = date::floor<date::days>(now);
        auto systime = date::make_time(chrono::duration_cast<chrono::seconds>(now - sysdate));

        int hour = systime.hours().count();         // Сʱ
        int minute = systime.minutes().count();     // ����
        int second = systime.seconds().count();     // ����

        // ����������ǰ�� '0'
        static auto format = [](int i) {
            std::ostringstream sstr;
            sstr << std::setw(2) << std::setfill('0') << i;
            return sstr.str();
            };

        std::string YYYY_MM_DD = date::format("%Y-%m-%d", sysdate);                 // ��-��-��
        std::string HH24_MM_SS = format(hour) + format(minute) + format(second);    // ʱ����

        return YYYY_MM_DD + "-" + HH24_MM_SS;  // ��������������ʱ���ַ���
    }
}