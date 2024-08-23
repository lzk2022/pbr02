//#include <WinUser.h>
#include "Application.h"
#include "../utils/Log.h"
#include "../core/Window.h"
#include "../scene/Renderer.h"
#include "../scene/UI.h"
#include "../core/Input.h"
#include "../core/Clock.h"
#include "../utils/Global.h"
#include "../core/Event.h"

using namespace scene;
namespace core {
	Application::Application()
	{
		LOG_TRACK;
	}
	void Application::Init()
	{
		LOG_TRACK;
		Window::Init();
		UI::Init();
		Event::RegisterCallBacks();
		GetHardware();
	}
	void Application::Load()
	{
		LOG_TRACK;
		Clock::Reset();
		Input::Clear();
		Input::HideCursor();
		// Renderer::Attach("欢迎界面");
		Renderer::Attach("前向渲染界面");
	}


	void Application::Run()
	{
		LOG_TRACK;
		CheckExit();
		CheckSwitchLayer();
		Clock::Update();
		Renderer::DrawScene();
		Renderer::DrawImGui();
		Renderer::Flush();
	}
	void Application::Clear()
	{
		LOG_TRACK;
		Window::Clear();
	}
	void Application::GetHardware()
	{
		LOG_TRACK;
		LOG_INFO("获取硬件规格");

		gHardware.glVendor		= std::string(reinterpret_cast<const char*>(glGetString(GL_VENDOR))); 
		gHardware.glRenderer	= std::string(reinterpret_cast<const char*>(glGetString(GL_RENDERER))); 
		gHardware.glVersion		= std::string(reinterpret_cast<const char*>(glGetString(GL_VERSION))); 
		gHardware.glslVersion	= std::string(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION))); 

		// 纹理大小限制、纹理单元和图像单元的最大数量
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &gHardware.glTexsize);         
		glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &gHardware.glTexsize3d);          
		glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &gHardware.glTexsizeCubemap);      
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &gHardware.glMaxImageUnits);   
		glGetIntegerv(GL_MAX_IMAGE_UNITS, &gHardware.glMaxTextureUnits);      

		// 每个着色器阶段中原子计数器的最大数量
		glGetIntegerv(GL_MAX_VERTEX_ATOMIC_COUNTERS, &gHardware.glMaxvAtcs);	// 顶点着色器中的原子计数器
		glGetIntegerv(GL_MAX_FRAGMENT_ATOMIC_COUNTERS, &gHardware.glMaxfAtcs);  // 片段着色器中的原子计数器
		glGetIntegerv(GL_MAX_COMPUTE_ATOMIC_COUNTERS, &gHardware.glMaxcAtcs);	// 计算着色器中的原子计数器

		// 每个着色器阶段中统一缓冲块的最大数量
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &gHardware.glMaxvUbos);		// 顶点着色器中的统一缓冲块
		glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS, &gHardware.glMaxgUbos);	// 几何着色器中的统一缓冲块
		glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &gHardware.glMaxfUbos);	// 片段着色器中的统一缓冲块
		glGetIntegerv(GL_MAX_COMPUTE_UNIFORM_BLOCKS, &gHardware.glMaxcUbos);	// 计算着色器中的统一缓冲块

		// 片段着色器和计算着色器中着色器存储块的最大数量
		glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &gHardware.glMaxfSsbos);	// 片段着色器存储块
		glGetIntegerv(GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS, &gHardware.glMaxcSsbos);	// 计算着色器存储块

		// 计算着色器中的工作组的最大数量
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &gHardware.workNumX);  // X方向工作组数量
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &gHardware.workNumY);  // Y方向工作组数量
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &gHardware.workNumZ);  // Z方向工作组数量

		// 计算着色器工作组的大小限制
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &gHardware.localSizeX);  // X方向工作组大小
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &gHardware.localSizeY);  // Y方向工作组大小
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &gHardware.localSizeZ);  // Z方向工作组大小

		// 计算着色器中的最大线程数
		glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &gHardware.maxInvocations);  // 最大调用次数

		// 用户自定义帧缓冲中可绘制的最大颜色缓冲区数
		GLint maxColorAttachments, maxDrawBuffers;
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);				// 最大颜色附件数
		glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);						// 最大绘制缓冲区数
		gHardware.glMaxColorBuffs = std::min(maxColorAttachments, maxDrawBuffers);  // 计算最大颜色缓冲区数
	}
	void Application::CheckExit()
	{
		// 检查用户是否请求退出
		if (Input::GetKeyDown(KB_ESCAPE)) {
			mIsExit = Window::OnExitRequest();  // 用户请求退出应用程序
			Input::SetKeyDown(KB_ESCAPE, false);  // 释放ESC键
		}
	}
	void Application::CheckSwitchLayer()
	{
		if (Input::GetKeyDown(KB_ENTER)) {
			if (Renderer::CurrScene()->mTitle != "欢迎界面") {
				Window::OnLayerSwitch();  // 切换ImGui图层
			}
			Input::SetKeyDown(KB_ENTER, false);  // 释放Enter键
		}
	}
}