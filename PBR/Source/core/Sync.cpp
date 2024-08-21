#include "Sync.h"
#include "../utils/Log.h"
namespace core {
	// 如果等待同步函数阻塞的时间超过此阈值，将会向控制台记录警告消息，通知用户等待时间过长。 10 秒
	constexpr GLuint64 gWarnThreshold = static_cast<GLuint64>(1e10);
	Sync::Sync(GLuint id) : mId(id)
	{
		LOG_TRACK;
		// 创建一个同步对象，当 GPU 上的所有先前命令都执行完毕时，这个同步对象会被标记为完成。
		// GL_SYNC_GPU_COMMANDS_COMPLETE 表示在 GPU 完成所有先前的命令后标记同步对象为完成。
		// 第二个参数必须为 0，目前没有其他有效值。
		mSync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		LOG_ASSERT(mSync != nullptr, "无法创建同步对象");
	}
	Sync::~Sync()
	{
		glDeleteSync(mSync);
		mSync = nullptr;
	}
	bool Sync::IsSign()
	{
		// 此函数查询同步对象的信号状态，而不会阻塞
		GLint status = GL_UNSIGNALED;
		glGetSynciv(mSync, GL_SYNC_STATUS, sizeof(GLint), NULL, &status);
		return (status == GL_SIGNALED) ? true : false;
	}
	void Sync::WaitClientSync(GLuint64 timeout)
	{
		LOG_TRACK;
		// 此函数将阻塞CPU，直到同步对象被信号通知
		// 第一次等待时自动刷新
		GLenum status = glClientWaitSync(mSync, GL_SYNC_FLUSH_COMMANDS_BIT, timeout);
		bool warned = false;
		GLuint64 waitTime = timeout;

		// 循环等待直到同步对象被信号通知或等待超时
		while (status == GL_TIMEOUT_EXPIRED){
			status = glClientWaitSync(mSync, 0, timeout);		// 后续调用不需要刷新位
			waitTime += timeout;

			if (!warned && waitTime > gWarnThreshold) {
				LOG_WARN("同步对象 {0} 在客户端上挂起超过10秒！",mId);
			}
		}
	}
	void Sync::WaitServerSync()
	{
		LOG_TRACK;
		glFlush();		// 确保同步对象刷新到GPU，避免无限循环
		bool warned = false;
		GLuint64 waitTime = 0;
		while (!IsSign()){
			glWaitSync(mSync, 0, GL_TIMEOUT_IGNORED);
			waitTime += GetServerTimeout();
			if (!warned && waitTime > gWarnThreshold) {
				LOG_ERROR("同步对象 {0} 在服务器端挂起超过10秒！", mId);
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
		// 每帧调用此函数可能会严重降低性能，就像过度怠速可能损坏您车辆的发动机一样
		// 等待到目前为止发出的所有命令完全由GPU执行
		glFinish();
	}
}