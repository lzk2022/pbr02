#include "Renderer.h"
#include "../utils/Log.h"
#include "../core/Window.h"
#include "../core/Sync.h"
#include "Factory.h"
#include <GLFW/glfw3.h>
#include "UI.h"
#include "../core/Input.h"
#include "../core/Sync.h"
#include "../core/Clock.h"
#include "../utils/Ext.h"

using namespace core;
namespace scene {
	static bool gDepthPrepass = false; // ���Ԥ�����־
	static uint gShadowIndex = 0U; // ��Ӱ����
	static std::unique_ptr<UBO> gRendererInput = nullptr; // ��Ⱦ������

	void Renderer::Attach(const std::string& title)
	{
		LOG_TRACK;
		LOG_INFO("�����µĳ�����{0}",title);

		Input::Clear();                     // �������
		Input::ShowCursor();                // ��ʾ���
		Window::Rename(title);

		Scene* newScene = factory::LoadScene(title);
		newScene->Init();
		mCurrScene = newScene;
		Sync::WaitFinish();
	}
	void Renderer::DrawScene()
	{
		mCurrScene->OnSceneRender();
	}
	void Renderer::DrawImGui()
	{
		LOG_TRACK;
		bool isSwitchScene = false;		// �Ƿ��л�����һ�������ı�־
		std::string nextSceneTitle;		// ��һ�������ı���

		UI::BeginFrame();
		UI::DrawMenuBar(nextSceneTitle, mCurrScene->Title());
		UI::DrawStatusBar();
		if (!nextSceneTitle.empty()) {
			isSwitchScene = true;
			Clear();
			UI::DrawLoadScreen();
		}
		else {
			mCurrScene->OnImGuiRender();
		}
		UI::EndFrame();

		Flush();
		if (isSwitchScene) {
			Detach();				// ���뵱ǰ�������������ã�
			Attach(nextSceneTitle);	// ������һ���������������ã�
		}
	}
	void Renderer::Flush()
	{
		LOG_TRACK;
		glfwSwapBuffers(Window::WindowPtr());	// ����GLFW������
		glfwPollEvents();						// ��ѯ�¼�
	}
	void Renderer::Clear()
	{
		// ����Ĭ��֡����������Ҫʹ�ú�ɫ��Ϊ�����ɫ����Ϊ����ϣ������ؿ�����Щ�����Ǳ���������ɫʹ������໺�������������ѡ�
		// ע�⣺�Զ���֡������Ӧʼ��ʹ�ú�ɫ�����ɫ����ȷ����Ⱦ�������ɾ������ǲ�ϣ�����κ���ֵ��0�⡣Ȼ������Ӧ����֡�������ϵ���
		glClearColor(0.0f, 0.0f, 1.0f, 1.0f);		// ���������ɫΪ����ɫ
		glClearDepth(1.0f);							// ����������Ϊ1.0
		glClearStencil(0);							// �������ģ��ֵΪ0��8λ������
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
	void Renderer::Detach()
	{
		LOG_TRACK;
		mLastScene = mCurrScene;	// ���浱ǰ����
		mCurrScene = nullptr;		// ��յ�ǰ����
		delete mLastScene;			// ɾ�����ĳ����������е����ж��󽫱�����
		mLastScene = nullptr;		// ���msLastScene
		Sync::WaitFinish();			// ����ֱ��������ȫж��
		Reset();
	}
	void Renderer::Reset()
	{
		// todo δ����
	}
	const Scene* Renderer::CurrScene()
	{
		return mCurrScene;
	}
	void Renderer::SetSeamlessCubemap(bool enable)
	{
		static bool sIsEnabled = false; // �޷���������ͼ���ñ�־
		if (enable && !sIsEnabled) {
			glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // �����޷���������ͼ
			sIsEnabled = true; // ���±�־
		}
		else if (!enable && sIsEnabled) {
			glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // �����޷���������ͼ
			sIsEnabled = false; // ���±�־
		}
	}
	void Renderer::SetDepthTest(bool enable)
	{
		static bool sIsEnabled = false;		// ��Ȳ������ñ�־
		if (enable && !sIsEnabled) {
			glEnable(GL_DEPTH_TEST);		// ������Ȳ���
			glDepthMask(GL_TRUE);			// ������Ȼ�����д��
			glDepthFunc(GL_LEQUAL);			// ������Ȳ��Ժ���
			glDepthRange(0.0f, 1.0f);		// ������ȷ�Χ
			sIsEnabled = true;				// ���±�־
		}
		else if (!enable && sIsEnabled) {
			glDisable(GL_DEPTH_TEST);		// ������Ȳ���
			sIsEnabled = false;				// ���±�־
		}
	}
	void Renderer::SetFaceCulling(bool enable)
	{
		static bool sIsEnabled = false;		// ���޳����ñ�־
		if (enable && !sIsEnabled) {
			glEnable(GL_CULL_FACE);			// �������޳�
			glFrontFace(GL_CCW);			// ������ʱ�뷽��Ϊ����
			glCullFace(GL_BACK);			// �����޳�����
			sIsEnabled = true;				// ���±�־
		}
		else if (!enable && sIsEnabled) {
			glDisable(GL_CULL_FACE);		// �������޳�
			sIsEnabled = false;				// ���±�־
		}
	}
	void Renderer::SetAlphaBlend(bool enable)
	{
		static bool isEnabled = false;		// Alpha������ñ�־
		if (enable && !isEnabled) {
			glEnable(GL_BLEND);				// ����Alpha���
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // ���û�Ϻ���
			glBlendEquation(GL_FUNC_ADD);	// ���û�Ϸ���
			isEnabled = true;				// ���±�־
		}
		else if (!enable && isEnabled) {
			glDisable(GL_BLEND);			// ����Alpha���
			isEnabled = false;				// ���±�־
		}
	}
	void Renderer::SetDepthPrepass(bool enable)
	{
		gDepthPrepass = enable; // �������Ԥ�����־
	}
	void Renderer::SetFrontFace(bool ccw)
	{
		glFrontFace(ccw ? GL_CCW : GL_CW); // ����ǰ�泯��
	}
	void Renderer::SetMSAA(bool enable)
	{
		// ���õ�MSAA����Ĭ��֡�������Ϲ������޶���ͨ����
		static GLint bufferCount = 0, sampleCount = 0, maxSampleCount = 0; // ����������������������
		if (sampleCount == 0) {
			glGetIntegerv(GL_SAMPLE_BUFFERS, &bufferCount); // ��ȡMSAA������
			glGetIntegerv(GL_SAMPLES, &sampleCount); // ��ȡMSAA������
			glGetIntegerv(GL_MAX_SAMPLES, &maxSampleCount); // ��ȡMSAA��������
			LOG_ASSERT(bufferCount > 0, "MSAA�����������ã������Ĵ���������"); // ���MSAA�������Ƿ����
			LOG_ASSERT(sampleCount == 4, "��Ч��MSAA��������С��ÿ����4������������"); // ���MSAA��������С�Ƿ���Ч

		}

		static bool isEnabled = false; // MSAA���ñ�־
		if (enable && !isEnabled) {
			glEnable(GL_MULTISAMPLE); // ���ö��ز���
			isEnabled = true; // ���±�־
		}
		else if (!enable && isEnabled) {
			glDisable(GL_MULTISAMPLE); // ���ö��ز���
			isEnabled = false; // ���±�־
		}
	}
	void Renderer::Render(const std::shared_ptr<Shader> shader)
	{
		auto& reg = mCurrScene->mRegistry; // ��ȡ��ǰ������ʵ��ע���
		auto meshGroup = reg.group<Mesh>(entt::get<Transform, Tag, Material>); // ��ȡ����Mesh��Transform��Tag��Material�����ʵ����
		auto modelGroup = reg.group<Model>(entt::get<Transform, Tag>); // ��ȡ����Model��Transform��Tag�����ʵ����

		if (!mRenderQueue.empty()) {
			constexpr float nearClip = 0.1f; // ��ƽ����þ���
			constexpr float farClip = 100.0f; // Զƽ����þ���

			glm::ivec2 resolution = glm::ivec2(Window::mWidth, Window::mHeight); // ��ȡ���ڵķֱ���
			glm::ivec2 cursorPos = UI::GetCursorPosition(); // ��ȡ�����λ��

			float totalTime = Clock::mTime; // ��ȡ��ʱ��
			float deltaTime = Clock::mDeltaTime; // ��ȡ֡���ʱ��
			static std::unique_ptr<UBO> gRendererInput1 = nullptr; // ��Ⱦ������

			// �ڵ�һ������ʱ������Ⱦ������UBO���ڲ�UBO�������Ż����ŵ�Attach��
			if (gRendererInput == nullptr) {
				const std::vector<GLuint> offset{ 0U, 8U, 16U, 20U, 24U, 28U, 32U, 36U }; // ƫ����
				const std::vector<GLuint> length{ 8U, 8U, 4U, 4U, 4U, 4U, 4U, 4U }; // ����
				const std::vector<GLuint> stride{ 8U, 8U, 4U, 4U, 4U, 4U, 4U, 4U }; // ���

				gRendererInput = std::make_unique<UBO>(1, offset, length, stride); // ��װ�ʲ�
			}

			gRendererInput->SetUniform(0U, utils::GetValPtr(resolution)); // ���÷ֱ���Uniform
			gRendererInput->SetUniform(1U, utils::GetValPtr(cursorPos)); // ���ù��λ��Uniform
			gRendererInput->SetUniform(2U, utils::GetValPtr(nearClip)); // ���ý�ƽ����þ���Uniform
			gRendererInput->SetUniform(3U, utils::GetValPtr(farClip)); // ����Զƽ����þ���Uniform
			gRendererInput->SetUniform(4U, utils::GetValPtr(totalTime)); // ������ʱ��Uniform
			gRendererInput->SetUniform(5U, utils::GetValPtr(deltaTime)); // ����֡���ʱ��Uniform
			gRendererInput->SetUniform(6U, utils::GetValPtr(static_cast<int>(gDepthPrepass))); // �������Ԥ����Uniform
			gRendererInput->SetUniform(7U, utils::GetValPtr(gShadowIndex)); // ������Ӱ����Uniform
		}

		while (!mRenderQueue.empty()){
			auto& entity = mRenderQueue.front();	// ��ȡ��Ⱦ�����еĵ�һ��ʵ��
			if (entity == entt::null) {				// ������ʵ��
				mRenderQueue.pop();					// ����Ⱦ�������Ƴ�
				continue;							// ������һ��ѭ��
			}

			if (meshGroup.contains(entity)) {		// ʵ����һ��ԭ������
				auto& transform = meshGroup.get<Transform>(entity);		// ��ȡTransform���
				auto& mesh = meshGroup.get<Mesh>(entity);				// ��ȡMesh���
				auto& material = meshGroup.get<Material>(entity);		// ��ȡMaterial���
				auto& tag = meshGroup.get<Tag>(entity);					// ��ȡTag���

				if (shader) {
					shader->SetUniform(1000U, transform.mTransform);	// �����Զ�����ɫ���ı任����Uniform
					shader->SetUniform(1001U, 0U);						// �����Զ�����ɫ���Ĳ���ID Uniform
					shader->Bind();										// ���Զ�����ɫ��
				}
				else {
					material.SetUniform(1000U, transform.mTransform);	// ���ò��ʵı任����Uniform
					material.SetUniform(1001U, 0U);						// ���ò���ID Uniform��ԭ������û�в���ID
					material.SetUniform(1002U, 0U);						// ������չUniform 1002
					material.SetUniform(1003U, 0U);						// ������չUniform 1003
					material.SetUniform(1004U, 0U);						// ������չUniform 1004
					material.SetUniform(1005U, 0U);						// ������չUniform 1005
					material.SetUniform(1006U, 0U);						// ������չUniform 1006
					material.SetUniform(1007U, 0U);						// ������չUniform 1007
					material.Bind();									// �󶨲��ʣ�������
				}
				if (tag.Contains(ETag::Skybox)) {
					SetFrontFace(false);				// ��պо��з���Ĳ���˳�򣬽���������
					mesh.Draw();						// ��������
					SetFrontFace(true);					// �ָ�ȫ�ֲ���˳��
				}
				else {
					mesh.Draw(); // ��������
				}
			}
			else if (modelGroup.contains(entity)) {		// ʵ����һ�������ģ��
				auto& transform = modelGroup.get<Transform>(entity); // ��ȡTransform���
				auto& model = modelGroup.get<Model>(entity); // ��ȡModel���

				for (auto& mesh : model.mMeshes) {
					GLuint materialId = mesh.mMaterialId; // ��ȡ����Ĳ���ID
					auto& material = model.mMaterials.at(materialId); // ��ȡ����

					if (shader) {
						shader->SetUniform(1000U, transform.mTransform); // �����Զ�����ɫ���ı任����Uniform
						shader->SetUniform(1001U, materialId); // �����Զ�����ɫ���Ĳ���ID Uniform
						shader->Bind(); // ���Զ�����ɫ��
					}
					else {
						material.SetUniform(1000U, transform.mTransform); // ���ò��ʵı任����Uniform
						material.SetUniform(1001U, materialId); // ���ò���ID Uniform
						material.SetUniform(1002U, 0U); // ������չUniform 1002
						material.SetUniform(1003U, 0U); // ������չUniform 1003
						material.SetUniform(1004U, 0U); // ������չUniform 1004
						material.SetUniform(1005U, 0U); // ������չUniform 1005
						material.SetUniform(1006U, 0U); // ������չUniform 1006
						material.SetUniform(1007U, 0U); // ������չUniform 1007
						material.Bind(); // �󶨲��ʣ�������
					}
					mesh.Draw(); // ��������
				}
			}
			// �ǿ�ʵ�������������ģ��������ܱ���Ϊ�ǿ���Ⱦ��
			else {
				Clear(); // �������������ʾ����ɫ��Ļ��UI�����Ƕ����ģ�
			}
			mRenderQueue.pop(); // ����Ⱦ�������Ƴ�ʵ��
		}
	}
}
