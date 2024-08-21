#include "Sync.h"
#include "../utils/Log.h"
namespace core {
	// ����ȴ�ͬ������������ʱ�䳬������ֵ�����������̨��¼������Ϣ��֪ͨ�û��ȴ�ʱ������� 10 ��
	constexpr GLuint64 gWarnThreshold = static_cast<GLuint64>(1e10);
	Sync::Sync(GLuint id) : mId(id)
	{
		LOG_TRACK;
		// ����һ��ͬ�����󣬵� GPU �ϵ�������ǰ���ִ�����ʱ�����ͬ������ᱻ���Ϊ��ɡ�
		// GL_SYNC_GPU_COMMANDS_COMPLETE ��ʾ�� GPU ���������ǰ���������ͬ������Ϊ��ɡ�
		// �ڶ�����������Ϊ 0��Ŀǰû��������Чֵ��
		mSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		LOG_ASSERT(mSync != nullptr, "�޷�����ͬ������");
	}
	Sync::~Sync()
	{
		glDeleteSync(mSync);
		mSync = nullptr;
	}
	bool Sync::IsSign()
	{
		// �˺�����ѯͬ��������ź�״̬������������
		GLint status = GL_UNSIGNALED;
		glGetSynciv(mSync, GL_SYNC_STATUS, sizeof(GLint), NULL, &status);
		return (status == GL_SIGNALED) ? true : false;
	}
	void Sync::WaitClientSync(GLuint64 timeout)
	{
		LOG_TRACK;
		// �˺���������CPU��ֱ��ͬ�������ź�֪ͨ
		// ��һ�εȴ�ʱ�Զ�ˢ��
		GLenum status = glClientWaitSync(mSync, GL_SYNC_FLUSH_COMMANDS_BIT, timeout);
		bool warned = false;
		GLuint64 waitTime = timeout;

		// ѭ���ȴ�ֱ��ͬ�������ź�֪ͨ��ȴ���ʱ
		while (status == GL_TIMEOUT_EXPIRED){
			status = glClientWaitSync(mSync, 0, timeout);		// �������ò���Ҫˢ��λ
			waitTime += timeout;

			if (!warned && waitTime > gWarnThreshold) {
				LOG_WARN("ͬ������ {0} �ڿͻ����Ϲ��𳬹�10�룡",mId);
			}
		}
	}
	void Sync::WaitServerSync()
	{
		LOG_TRACK;
		glFlush();		// ȷ��ͬ������ˢ�µ�GPU����������ѭ��
		bool warned = false;
		GLuint64 waitTime = 0;
		while (!IsSign()){
			glWaitSync(mSync, 0, GL_TIMEOUT_IGNORED);
			waitTime += GetServerTimeout();
			if (!warned && waitTime > gWarnThreshold) {
				LOG_ERROR("ͬ������ {0} �ڷ������˹��𳬹�10�룡", mId);
				warned = true;
			}
		}
	}
	GLint64 Sync::GetServerTimeout()
	{
		LOG_TRACK;
		static GLint64 sMaxServerTimeout = -1;
		if (sMaxServerTimeout < 0) {
			glGetInteger64v(GL_MAX_SERVER_WAIT_TIMEOUT, &sMaxServerTimeout);
		}
		return sMaxServerTimeout;
	}
	void Sync::WaitFlush()
	{
		glFlush();
	}
	void Sync::WaitFinish()
	{
		// ÿ֡���ô˺������ܻ����ؽ������ܣ�������ȵ��ٿ������������ķ�����һ��
		// �ȴ���ĿǰΪֹ����������������ȫ��GPUִ��
		glFinish();
	}
}