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
		// Renderer::Attach("��ӭ����");
		Renderer::Attach("ǰ����Ⱦ����");
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
		LOG_INFO("��ȡӲ�����");

		gHardware.glVendor		= std::string(reinterpret_cast<const char*>(glGetString(GL_VENDOR))); 
		gHardware.glRenderer	= std::string(reinterpret_cast<const char*>(glGetString(GL_RENDERER))); 
		gHardware.glVersion		= std::string(reinterpret_cast<const char*>(glGetString(GL_VERSION))); 
		gHardware.glslVersion	= std::string(reinterpret_cast<const char*>(glGetString(GL_SHADING_LANGUAGE_VERSION))); 

		// �����С���ơ�����Ԫ��ͼ��Ԫ���������
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &gHardware.glTexsize);         
		glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &gHardware.glTexsize3d);          
		glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, &gHardware.glTexsizeCubemap);      
		glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &gHardware.glMaxImageUnits);   
		glGetIntegerv(GL_MAX_IMAGE_UNITS, &gHardware.glMaxTextureUnits);      

		// ÿ����ɫ���׶���ԭ�Ӽ��������������
		glGetIntegerv(GL_MAX_VERTEX_ATOMIC_COUNTERS, &gHardware.glMaxvAtcs);	// ������ɫ���е�ԭ�Ӽ�����
		glGetIntegerv(GL_MAX_FRAGMENT_ATOMIC_COUNTERS, &gHardware.glMaxfAtcs);  // Ƭ����ɫ���е�ԭ�Ӽ�����
		glGetIntegerv(GL_MAX_COMPUTE_ATOMIC_COUNTERS, &gHardware.glMaxcAtcs);	// ������ɫ���е�ԭ�Ӽ�����

		// ÿ����ɫ���׶���ͳһ�������������
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &gHardware.glMaxvUbos);		// ������ɫ���е�ͳһ�����
		glGetIntegerv(GL_MAX_GEOMETRY_UNIFORM_BLOCKS, &gHardware.glMaxgUbos);	// ������ɫ���е�ͳһ�����
		glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_BLOCKS, &gHardware.glMaxfUbos);	// Ƭ����ɫ���е�ͳһ�����
		glGetIntegerv(GL_MAX_COMPUTE_UNIFORM_BLOCKS, &gHardware.glMaxcUbos);	// ������ɫ���е�ͳһ�����

		// Ƭ����ɫ���ͼ�����ɫ������ɫ���洢����������
		glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &gHardware.glMaxfSsbos);	// Ƭ����ɫ���洢��
		glGetIntegerv(GL_MAX_COMPUTE_SHADER_STORAGE_BLOCKS, &gHardware.glMaxcSsbos);	// ������ɫ���洢��

		// ������ɫ���еĹ�������������
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &gHardware.workNumX);  // X������������
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &gHardware.workNumY);  // Y������������
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &gHardware.workNumZ);  // Z������������

		// ������ɫ��������Ĵ�С����
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &gHardware.localSizeX);  // X���������С
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &gHardware.localSizeY);  // Y���������С
		glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &gHardware.localSizeZ);  // Z���������С

		// ������ɫ���е�����߳���
		glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &gHardware.maxInvocations);  // �����ô���

		// �û��Զ���֡�����пɻ��Ƶ������ɫ��������
		GLint maxColorAttachments, maxDrawBuffers;
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxColorAttachments);				// �����ɫ������
		glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawBuffers);						// �����ƻ�������
		gHardware.glMaxColorBuffs = std::min(maxColorAttachments, maxDrawBuffers);  // ���������ɫ��������
	}
	void Application::CheckExit()
	{
		// ����û��Ƿ������˳�
		if (Input::GetKeyDown(KB_ESCAPE)) {
			mIsExit = Window::OnExitRequest();  // �û������˳�Ӧ�ó���
			Input::SetKeyDown(KB_ESCAPE, false);  // �ͷ�ESC��
		}
	}
	void Application::CheckSwitchLayer()
	{
		if (Input::GetKeyDown(KB_ENTER)) {
			if (Renderer::CurrScene()->mTitle != "��ӭ����") {
				Window::OnLayerSwitch();  // �л�ImGuiͼ��
			}
			Input::SetKeyDown(KB_ENTER, false);  // �ͷ�Enter��
		}
	}
}