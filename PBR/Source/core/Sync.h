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

		// 查询同步对象是否已被信号通知
		bool IsSign();

		// 客户端等待同步对象被信号通知，timeout为等待超时时间，默认为0.1毫秒（100000纳秒）
		void WaitClientSync(GLuint64 timeout = 1e5);

		// 等待服务器同步对象被通知
		void WaitServerSync();

		// 获取服务器等待超时时间 单位为纳秒
		static GLint64 GetServerTimeout();

		// 等待所有已发出的命令刷新到GPU
		static void WaitFlush();

		// 等待所有已发出的命令由GPU执行完毕
		static void WaitFinish();
	private:
		GLuint mId;					// 同步对象id
		GLsync mSync = nullptr;		// OpenGL的同步对象
	};
}

