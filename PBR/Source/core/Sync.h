#pragma once
#include <glad/glad.h>
namespace core {
	class Sync {
	public:
		Sync(GLuint id);
		~Sync();
		Sync(const Sync&) = delete;
		Sync(Sync&& other) = delete;
		Sync& operator=(const Sync&) = delete;
		Sync& operator=(Sync&& other) = delete;

		// ��ѯͬ�������Ƿ��ѱ��ź�֪ͨ
		bool IsSign();

		// �ͻ��˵ȴ�ͬ�������ź�֪ͨ��timeoutΪ�ȴ���ʱʱ�䣬Ĭ��Ϊ0.1���루100000���룩
		void WaitClientSync(GLuint64 timeout = 1e5);

		// �ȴ�������ͬ������֪ͨ
		void WaitServerSync();

		// ��ȡ�������ȴ���ʱʱ�� ��λΪ����
		static GLint64 GetServerTimeout();

		// �ȴ������ѷ���������ˢ�µ�GPU
		static void WaitFlush();

		// �ȴ������ѷ�����������GPUִ�����
		static void WaitFinish();
	private:
		GLuint mId;					// ͬ������id
		GLsync mSync = nullptr;		// OpenGL��ͬ������
	};
}

