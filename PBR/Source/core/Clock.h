#pragma once
#include <chrono>
#include <string>
namespace core {
	class Clock {
	public:
		static void Reset();
		static void Update();
		// ��ȡ��ǰ�� UTC ʱ�䲢���ַ�����ʽ����
		static std::string GetDateTimeUTC();

	public:
		static float DeltaTime() { return mDeltaTime; }
		static float Time() { return mTime; }
		static float Fps() { return mFps; }
		static float Ms() { return mMs; }

	public:
		static std::chrono::time_point<std::chrono::system_clock> mStartTime;   // ��¼��������ʱ��ϵͳʱ���
		inline static float mLastFrame = 0.0f;      // ��һ֡��ʱ���
		inline static float mThisFrame = 0.0f;      // ��ǰ֡��ʱ���
		inline static int mFrameCount = 0;          // ֡����
		inline static float mDuration = 0.0f;       // �ۼ�ʱ��

		inline static float mDeltaTime = 0.0f;      // ʱ����
		inline static float mTime = 0.0f;           // �ܾ���ʱ��
		inline static float mFps = 0.0f;            // ÿ��֡����ÿ0.1�����һ�Σ�
		inline static float mMs = 0.0f;             // ÿ֡������
	};
}

