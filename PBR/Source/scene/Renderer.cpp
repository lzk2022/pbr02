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
	static bool gDepthPrepass = false; // 深度预处理标志
	static uint gShadowIndex = 0U; // 阴影索引
	static std::unique_ptr<UBO> gRendererInput = nullptr; // 渲染器输入

	void Renderer::Attach(const std::string& title)
	{
		LOG_TRACK;
		LOG_INFO("进入新的场景：{0}",title);

		Input::Clear();                     // 清除输入
		Input::ShowCursor();                // 显示光标
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
		bool isSwitchScene = false;		// 是否切换到下一个场景的标志
		std::string nextSceneTitle;		// 下一个场景的标题

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
			Detach();				// 分离当前场景（阻塞调用）
			Attach(nextSceneTitle);	// 附加下一个场景（阻塞调用）
		}
	}
	void Renderer::Flush()
	{
		LOG_TRACK;
		glfwSwapBuffers(Window::WindowPtr());	// 交换GLFW缓冲区
		glfwPollEvents();						// 轮询事件
	}
	void Renderer::Clear()
	{
		// 对于默认帧缓冲区，不要使用黑色作为清除颜色，因为我们希望清楚地看到哪些像素是背景，但黑色使调试许多缓冲区纹理变得困难。
		// 注意：自定义帧缓冲区应始终使用黑色清除颜色，以确保渲染缓冲区干净，我们不希望有任何脏值除0外。然而，你应该在帧缓冲区上调用
		glClearColor(0.0f, 0.0f, 1.0f, 1.0f);		// 设置清除颜色为深蓝色
		glClearDepth(1.0f);							// 设置清除深度为1.0
		glClearStencil(0);							// 设置清除模板值为0（8位整数）
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	}
	void Renderer::Detach()
	{
		LOG_TRACK;
		mLastScene = mCurrScene;	// 保存当前场景
		mCurrScene = nullptr;		// 清空当前场景
		delete mLastScene;			// 删除最后的场景，场景中的所有对象将被析构
		mLastScene = nullptr;		// 清空msLastScene
		Sync::WaitFinish();			// 阻塞直到场景完全卸载
		Reset();
	}
	void Renderer::Reset()
	{
		// todo 未开发
	}
	const Scene* Renderer::CurrScene()
	{
		return mCurrScene;
	}
	void Renderer::SetSeamlessCubemap(bool enable)
	{
		static bool sIsEnabled = false; // 无缝立方体贴图启用标志
		if (enable && !sIsEnabled) {
			glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // 启用无缝立方体贴图
			sIsEnabled = true; // 更新标志
		}
		else if (!enable && sIsEnabled) {
			glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS); // 禁用无缝立方体贴图
			sIsEnabled = false; // 更新标志
		}
	}
	void Renderer::SetDepthTest(bool enable)
	{
		static bool sIsEnabled = false;		// 深度测试启用标志
		if (enable && !sIsEnabled) {
			glEnable(GL_DEPTH_TEST);		// 启用深度测试
			glDepthMask(GL_TRUE);			// 启用深度缓冲区写入
			glDepthFunc(GL_LEQUAL);			// 设置深度测试函数
			glDepthRange(0.0f, 1.0f);		// 设置深度范围
			sIsEnabled = true;				// 更新标志
		}
		else if (!enable && sIsEnabled) {
			glDisable(GL_DEPTH_TEST);		// 禁用深度测试
			sIsEnabled = false;				// 更新标志
		}
	}
	void Renderer::SetFaceCulling(bool enable)
	{
		static bool sIsEnabled = false;		// 面剔除启用标志
		if (enable && !sIsEnabled) {
			glEnable(GL_CULL_FACE);			// 启用面剔除
			glFrontFace(GL_CCW);			// 设置逆时针方向为正面
			glCullFace(GL_BACK);			// 设置剔除背面
			sIsEnabled = true;				// 更新标志
		}
		else if (!enable && sIsEnabled) {
			glDisable(GL_CULL_FACE);		// 禁用面剔除
			sIsEnabled = false;				// 更新标志
		}
	}
	void Renderer::SetAlphaBlend(bool enable)
	{
		static bool isEnabled = false;		// Alpha混合启用标志
		if (enable && !isEnabled) {
			glEnable(GL_BLEND);				// 启用Alpha混合
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // 设置混合函数
			glBlendEquation(GL_FUNC_ADD);	// 设置混合方程
			isEnabled = true;				// 更新标志
		}
		else if (!enable && isEnabled) {
			glDisable(GL_BLEND);			// 禁用Alpha混合
			isEnabled = false;				// 更新标志
		}
	}
	void Renderer::SetDepthPrepass(bool enable)
	{
		gDepthPrepass = enable; // 设置深度预处理标志
	}
	void Renderer::SetFrontFace(bool ccw)
	{
		glFrontFace(ccw ? GL_CCW : GL_CW); // 设置前面朝向
	}
	void Renderer::SetMSAA(bool enable)
	{
		// 内置的MSAA仅在默认帧缓冲区上工作（无多重通道）
		static GLint bufferCount = 0, sampleCount = 0, maxSampleCount = 0; // 缓冲区，采样和最大采样数
		if (sampleCount == 0) {
			glGetIntegerv(GL_SAMPLE_BUFFERS, &bufferCount); // 获取MSAA缓冲区
			glGetIntegerv(GL_SAMPLES, &sampleCount); // 获取MSAA采样数
			glGetIntegerv(GL_MAX_SAMPLES, &maxSampleCount); // 获取MSAA最大采样数
			LOG_ASSERT(bufferCount > 0, "MSAA缓冲区不可用！检查你的窗口上下文"); // 检查MSAA缓冲区是否可用
			LOG_ASSERT(sampleCount == 4, "无效的MSAA缓冲区大小！每像素4个采样不可用"); // 检查MSAA缓冲区大小是否有效

		}

		static bool isEnabled = false; // MSAA启用标志
		if (enable && !isEnabled) {
			glEnable(GL_MULTISAMPLE); // 启用多重采样
			isEnabled = true; // 更新标志
		}
		else if (!enable && isEnabled) {
			glDisable(GL_MULTISAMPLE); // 禁用多重采样
			isEnabled = false; // 更新标志
		}
	}
	void Renderer::Render(const std::shared_ptr<Shader> shader)
	{
		auto& reg = mCurrScene->mRegistry; // 获取当前场景的实体注册表
		auto meshGroup = reg.group<Mesh>(entt::get<Transform, Tag, Material>); // 获取包含Mesh、Transform、Tag和Material组件的实体组
		auto modelGroup = reg.group<Model>(entt::get<Transform, Tag>); // 获取包含Model、Transform和Tag组件的实体组

		if (!mRenderQueue.empty()) {
			constexpr float nearClip = 0.1f; // 近平面剪裁距离
			constexpr float farClip = 100.0f; // 远平面剪裁距离

			glm::ivec2 resolution = glm::ivec2(Window::mWidth, Window::mHeight); // 获取窗口的分辨率
			glm::ivec2 cursorPos = UI::GetCursorPosition(); // 获取鼠标光标位置

			float totalTime = Clock::mTime; // 获取总时间
			float deltaTime = Clock::mDeltaTime; // 获取帧间隔时间
			static std::unique_ptr<UBO> gRendererInput1 = nullptr; // 渲染器输入

			// 在第一次运行时创建渲染器输入UBO（内部UBO）可以优化，放到Attach中
			if (gRendererInput == nullptr) {
				const std::vector<GLuint> offset{ 0U, 8U, 16U, 20U, 24U, 28U, 32U, 36U }; // 偏移量
				const std::vector<GLuint> length{ 8U, 8U, 4U, 4U, 4U, 4U, 4U, 4U }; // 长度
				const std::vector<GLuint> stride{ 8U, 8U, 4U, 4U, 4U, 4U, 4U, 4U }; // 跨距

				gRendererInput = std::make_unique<UBO>(1, offset, length, stride); // 包装资产
			}

			gRendererInput->SetUniform(0U, utils::GetValPtr(resolution)); // 设置分辨率Uniform
			gRendererInput->SetUniform(1U, utils::GetValPtr(cursorPos)); // 设置光标位置Uniform
			gRendererInput->SetUniform(2U, utils::GetValPtr(nearClip)); // 设置近平面剪裁距离Uniform
			gRendererInput->SetUniform(3U, utils::GetValPtr(farClip)); // 设置远平面剪裁距离Uniform
			gRendererInput->SetUniform(4U, utils::GetValPtr(totalTime)); // 设置总时间Uniform
			gRendererInput->SetUniform(5U, utils::GetValPtr(deltaTime)); // 设置帧间隔时间Uniform
			gRendererInput->SetUniform(6U, utils::GetValPtr(static_cast<int>(gDepthPrepass))); // 设置深度预处理Uniform
			gRendererInput->SetUniform(7U, utils::GetValPtr(gShadowIndex)); // 设置阴影索引Uniform
		}

		while (!mRenderQueue.empty()){
			auto& entity = mRenderQueue.front();	// 获取渲染队列中的第一个实体
			if (entity == entt::null) {				// 跳过空实体
				mRenderQueue.pop();					// 从渲染队列中移除
				continue;							// 继续下一个循环
			}

			if (meshGroup.contains(entity)) {		// 实体是一个原生网格
				auto& transform = meshGroup.get<Transform>(entity);		// 获取Transform组件
				auto& mesh = meshGroup.get<Mesh>(entity);				// 获取Mesh组件
				auto& material = meshGroup.get<Material>(entity);		// 获取Material组件
				auto& tag = meshGroup.get<Tag>(entity);					// 获取Tag组件

				if (shader) {
					shader->SetUniform(1000U, transform.mTransform);	// 设置自定义着色器的变换矩阵Uniform
					shader->SetUniform(1001U, 0U);						// 设置自定义着色器的材质ID Uniform
					shader->Bind();										// 绑定自定义着色器
				}
				else {
					material.SetUniform(1000U, transform.mTransform);	// 设置材质的变换矩阵Uniform
					material.SetUniform(1001U, 0U);						// 设置材质ID Uniform，原生网格没有材质ID
					material.SetUniform(1002U, 0U);						// 设置扩展Uniform 1002
					material.SetUniform(1003U, 0U);						// 设置扩展Uniform 1003
					material.SetUniform(1004U, 0U);						// 设置扩展Uniform 1004
					material.SetUniform(1005U, 0U);						// 设置扩展Uniform 1005
					material.SetUniform(1006U, 0U);						// 设置扩展Uniform 1006
					material.SetUniform(1007U, 0U);						// 设置扩展Uniform 1007
					material.Bind();									// 绑定材质，无需解绑
				}
				if (tag.Contains(ETag::Skybox)) {
					SetFrontFace(false);				// 天空盒具有反向的缠绕顺序，仅绘制内面
					mesh.Draw();						// 绘制网格
					SetFrontFace(true);					// 恢复全局缠绕顺序
				}
				else {
					mesh.Draw(); // 绘制网格
				}
			}
			else if (modelGroup.contains(entity)) {		// 实体是一个导入的模型
				auto& transform = modelGroup.get<Transform>(entity); // 获取Transform组件
				auto& model = modelGroup.get<Model>(entity); // 获取Model组件

				for (auto& mesh : model.mMeshes) {
					GLuint materialId = mesh.mMaterialId; // 获取网格的材质ID
					auto& material = model.mMaterials.at(materialId); // 获取材质

					if (shader) {
						shader->SetUniform(1000U, transform.mTransform); // 设置自定义着色器的变换矩阵Uniform
						shader->SetUniform(1001U, materialId); // 设置自定义着色器的材质ID Uniform
						shader->Bind(); // 绑定自定义着色器
					}
					else {
						material.SetUniform(1000U, transform.mTransform); // 设置材质的变换矩阵Uniform
						material.SetUniform(1001U, materialId); // 设置材质ID Uniform
						material.SetUniform(1002U, 0U); // 设置扩展Uniform 1002
						material.SetUniform(1003U, 0U); // 设置扩展Uniform 1003
						material.SetUniform(1004U, 0U); // 设置扩展Uniform 1004
						material.SetUniform(1005U, 0U); // 设置扩展Uniform 1005
						material.SetUniform(1006U, 0U); // 设置扩展Uniform 1006
						material.SetUniform(1007U, 0U); // 设置扩展Uniform 1007
						material.Bind(); // 绑定材质，无需解绑
					}
					mesh.Draw(); // 绘制网格
				}
			}
			// 非空实体必须包含网格或模型组件才能被认为是可渲染的
			else {
				Clear(); // 在这种情况下显示深蓝色屏幕（UI部分是独立的）
			}
			mRenderQueue.pop(); // 从渲染队列中移除实体
		}
	}
}
