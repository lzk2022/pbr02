#include "Scene01.h"
#include "../scene/Renderer.h"
#include "../asset/Shader.h"
#include "../component/Mesh.h"
#include "../component/Material.h"
#include "../asset/Sampler.h"
#include "../component/Transform.h"
#include "../component/Camera.h"
#include "../component/Light.h"
#include "../utils/Math.h"
#include "../core/Window.h"
#include "../core/Sync.h"
#include "../scene/UI.h"

using namespace utils;
using namespace component;
using namespace core;
using namespace scene;
namespace scene {
	struct InputFromUI
	{
		bool IsShowPlane = true;        // 是否显示平面
		bool IsShowLightCluster = true;  // 是否显示光照集群
		bool IsOrbit = true;            // 是否进行轨道控制
		bool IsDrawDepthBuffer = false; // 是否绘制深度缓冲区

		float LightClusterIntensity = 10.0f;    // 光照集群强度
		float SkyboxExposure = 1.0f;    // 天空盒曝光度
		float SkyboxLod = 0.0f;         // 天空盒LOD级别
		float SphereMetalness = 0.05f;             // 球体金属度
		float SphereRoughness = 0.05f;             // 球体粗糙度
		float SphereAO = 1.0f;              // 球体环境光遮蔽
		float PlaneRoughness = 0.1f;              // 平面粗糙度
		glm::vec4  SphereAlbedo{ 0.22f, 0.0f, 1.0f, 1.0f };  // 球体反照率
		int   BlursNum = 3;              // 模糊数量
		int   ToneMappingMode = 3;              // 色调映射模式
	};
	InputFromUI gInputFromUI;

	static bool  gShowGrid = false;									// 是否显示网格
	static float gGridCellSize = 2.0f;								// 网格单元大小
	static vec4  gThinLineColor = vec4(0.1f, 0.1f, 0.1f, 1.0f);		// 细线颜色
	static vec4  gWideLineColor = vec4(0.2f, 0.2f, 0.2f, 1.0f);		// 粗线颜色
	static float gOrbitSpeed = 0.3f;								// 轨道速度
	static float gLightClusterIntensity = 10.0f;					// 光照集群强度
	constexpr GLuint gTileSize = 16;								// 瓦片大小
	constexpr GLuint gPlsNum = 28;									// 点光源数量
	static GLuint gNx = 0, gNy = 0;									// 窗口尺寸

	void Scene01::Init()
	{
		LOG_TRACK;
		mTitle = "前向渲染界面";
		PrecomputeIBL("texture\\HDRI\\cosmic\\");

		// 添加资源管理器中的各种资源
		mResourceManager.Add(-1, std::make_shared<Mesh>(Primitive::Sphere));
		mResourceManager.Add(01, std::make_shared<Shader>("core\\infinite_grid.glsl"));
		mResourceManager.Add(02, std::make_shared<Shader>("core\\skybox.glsl"));
		mResourceManager.Add(03, std::make_shared<Shader>("core\\light.glsl"));
		mResourceManager.Add(04, std::make_shared<Shader>("scene_01\\pbr.glsl"));
		mResourceManager.Add(05, std::make_shared<Shader>("scene_01\\post_process.glsl"));
		mResourceManager.Add(00, std::make_shared<CShader>("core\\bloom.glsl"));
		mResourceManager.Add(10, std::make_shared<CShader>("scene_01\\cull.glsl"));
		mResourceManager.Add(12, std::make_shared<Material>(mResourceManager.Get<Shader>(02)));
		mResourceManager.Add(13, std::make_shared<Material>(mResourceManager.Get<Shader>(03)));
		mResourceManager.Add(14, std::make_shared<Material>(mResourceManager.Get<Shader>(04)));
		mResourceManager.Add(98, std::make_shared<Sampler>(FilterMode::Point));
		mResourceManager.Add(99, std::make_shared<Sampler>(FilterMode::Bilinear));

		// 创建从着色器中的统一缓冲对象（UBO），重复的将被跳过
		AddUBO(mResourceManager.Get<Shader>(02)->getId());
		AddUBO(mResourceManager.Get<Shader>(03)->getId());
		AddUBO(mResourceManager.Get<Shader>(04)->getId());

		// 创建中间帧缓冲对象（FBO）
		AddFBO(Window::mWidth, Window::mHeight);
		AddFBO(Window::mWidth, Window::mHeight);
		AddFBO(Window::mWidth, Window::mHeight);
		AddFBO(Window::mWidth / 2, Window::mHeight / 2);

		mFBO[0].AddDepStTexture();           // 添加深度和模板附件
		mFBO[1].AddColorTexture(2, true);   // 添加带有MIP贴图的颜色附件
		mFBO[1].AddDepStRenderBuffer(true); // 添加深度和模板附件
		mFBO[2].AddColorTexture(2);         // 添加带有MIP贴图的颜色附件
		mFBO[3].AddColorTexture(2);         // 添加带有MIP贴图的颜色附件

		// 创建相机实体
		mCamera = CreateEntity("Camera", ETag::MainCamera);
		mCamera.GetComponent<Transform>().Translate(vec3(0.0f, 6.0f, -16.0f));  // 设置相机位置
		mCamera.GetComponent<Transform>().Rotate(world::up, 180.0f, Space::Local);  // 旋转相机
		mCamera.AddComponent<Camera>(View::Perspective);  // 添加透视摄像机组件
		mCamera.AddComponent<Spotlight>(vec3(1.0f, 0.553f, 0.0f), 3.8f);  // 添加聚光灯组件
		mCamera.GetComponent<Spotlight>().SetCutoff(12.0f);  // 设置聚光灯的光锥角度

		// 创建天空盒实体
		mSkybox = CreateEntity("Skybox", ETag::Skybox);
		mSkybox.AddComponent<Mesh>(Primitive::Cube);  // 添加立方体网格组件
		if ( true) {
			auto& mat = mSkybox.AddComponent<Material>(mResourceManager.Get<Material>(12));
			mat.SetTexture(0, mPrefilteredMap);  // 设置材质纹理
			mat.BindUniform(0, &gInputFromUI.SkyboxExposure);  // 绑定材质统一变量
			mat.BindUniform(1, &gInputFromUI.SkyboxLod);  // 绑定材质统一变量
		}

		// 创建球体实体
		auto sphereMesh = mResourceManager.Get<Mesh>(-1);  // 获取球体网格资源
		mSphere = CreateEntity("Sphere");
		mSphere.AddComponent<Mesh>(sphereMesh);  // 添加球体网格组件
		mSphere.GetComponent<Transform>().Translate(world::up * 8.5f);  // 设置球体位置
		mSphere.GetComponent<Transform>().Scale(2.0f);  // 缩放球体
		SetMaterial(mSphere.AddComponent<Material>(mResourceManager.Get<Material>(14)), 0);  // 设置球体材质

		// 创建平面实体
		mPlane = CreateEntity("Plane");
		mPlane.AddComponent<Mesh>(Primitive::Plane);  // 添加平面网格组件
		mPlane.GetComponent<Transform>().Translate(world::down * 4.0f);  // 设置平面位置
		mPlane.GetComponent<Transform>().Scale(3.0f);  // 缩放平面
		SetMaterial(mPlane.AddComponent<Material>(mResourceManager.Get<Material>(14)), 2);  // 设置平面材质

		// 创建符文石实体
		mRunestone = CreateEntity("Runestone");
		mRunestone.GetComponent<Transform>().Scale(0.02f);  // 缩放符文石
		mRunestone.GetComponent<Transform>().Translate(world::down * 4.0f);  // 设置符文石位置
		// 加载模型并设置材质
		if (true) {
			auto& model = mRunestone.AddComponent<Model>("model\\runestone\\runestone.fbx", Quality::Auto);  // 添加模型组件
			SetMaterial(model.SetMaterial("pillars", mResourceManager.Get<Material>(14)), 31);  // 设置模型材质
			SetMaterial(model.SetMaterial("platform", mResourceManager.Get<Material>(14)), 32);  // 设置模型材质
		}

		// 创建定向光实体
		mDirectLight = CreateEntity("Directional Light");
		mDirectLight.GetComponent<Transform>().Rotate(world::left, 45.0f, Space::Local);	// 旋转定向光方向
		mDirectLight.AddComponent<DirectionLight>(color::white, 0.2f);						// 添加定向光组件

		// 对于静态光源，只需在`Init()`中设置一次统一缓冲区
		if ( true) {
			auto& ubo = mUBO[1];
			auto& dl = mDirectLight.GetComponent<DirectionLight>();
			auto& dt = mDirectLight.GetComponent<Transform>();
			ubo.SetUniform(0, GetValPtr(dl.mColor));  // 设置统一变量
			ubo.SetUniform(1, GetValPtr(-dt.mForward));  // 设置统一变量
			ubo.SetUniform(2, GetValPtr(dl.mIntensity));  // 设置统一变量
		}

		// 创建轨道光实体
		mOrbitLight = CreateEntity("Orbit Light");
		mOrbitLight.GetComponent<Transform>().Translate(0.0f, 8.0f, 4.5f);  // 设置轨道光位置
		mOrbitLight.GetComponent<Transform>().Scale(0.3f);  // 缩放轨道光
		mOrbitLight.AddComponent<PointLight>(color::lime, 0.8f);  // 添加点光源组件
		mOrbitLight.GetComponent<PointLight>().SetAttenuation(0.09f, 0.032f);  // 设置点光源衰减
		mOrbitLight.AddComponent<Mesh>(sphereMesh);  // 添加球体网格组件
		if (true) {
			auto& mat = mOrbitLight.AddComponent<Material>(mResourceManager.Get<Material>(13));
			auto& pl = mOrbitLight.GetComponent<PointLight>();
			mat.SetUniform(3, pl.mColor);  // 设置材质统一变量
			mat.SetUniform(4, pl.mIntensity);  // 设置材质统一变量
			mat.SetUniform(5, 2.0f);  // 设置材质统一变量（泛光倍增器，颜色饱和度）
		}

		// 创建一个大小为8x8的网格（64个单元），选择每个边界单元格作为一个点光源（总共28个点光源）
		for (int i = 0, index = 0; i < 64; i++) {
			int row = i / 8;	// 行索引 [0, 7]
			int col = i % 8;	// 列索引 [0, 7]
			if (bool onBorder = row == 0 || row == 7 || col == 0 || col == 7; !onBorder) {
				continue;		// 跳过中间的单元格
			}

			float ksi = Math::RandomGenerator<float>();  // 生成随机数ksi
			vec3 color = Math::HSL2RGB(ksi, 0.7f + ksi * 0.3f, 0.4f + ksi * 0.2f);  // 随机明亮颜色
			vec3 position = vec3(row - 3.5f, 1.5f, col - 3.5f) * 9.0f;  // 计算点光源位置

			auto& pl = mPointLights[index];  // 获取点光源实体的引用
			pl = CreateEntity("Point Light " + std::to_string(index));  // 创建点光源实体
			pl.GetComponent<Transform>().Translate(position - world::origin);  // 设置点光源位置
			pl.GetComponent<Transform>().Scale(0.8f);  // 缩放点光源
			pl.AddComponent<PointLight>(color, 1.5f);  // 添加点光源组件
			pl.AddComponent<Mesh>(sphereMesh);  // 添加球体网格组件

			auto& mat = pl.AddComponent<Material>(mResourceManager.Get<Material>(13));  // 获取材质引用
			auto& ppl = pl.GetComponent<PointLight>();  // 获取点光源组件引用

			ppl.SetAttenuation(0.09f, 0.032f);  // 设置点光源衰减
			mat.BindUniform(3, &ppl.mColor);  // 绑定材质统一变量
			mat.SetUniform(4, ppl.mIntensity);  // 设置材质统一变量
			mat.SetUniform(5, 7.0f);  // 设置材质统一变量（泛光倍增器，颜色饱和度）
			index++;
		}

		SetPLBuffers();
		
		Renderer::SetFaceCulling(true);
		Renderer::SetAlphaBlend(true);
		Renderer::SetSeamlessCubemap(true);

	}
	void Scene01::OnSceneRender()
	{
		auto& mainCamera = mCamera.GetComponent<Camera>();
		mainCamera.Update();
		// 更新主摄像机的统一缓冲对象（UBO）
		if (true) {
			auto& ubo = mUBO[0];
			ubo.SetUniform(0, GetValPtr(mainCamera.mpTransform->mPosition));
			ubo.SetUniform(1, GetValPtr(mainCamera.mpTransform->mForward));
			ubo.SetUniform(2, GetValPtr(mainCamera.GetViewMatrix()));
			ubo.SetUniform(3, GetValPtr(mainCamera.GetProjectionMatrix()));
		}

		// 更新手电筒光源的统一缓冲对象（UBO）
		if (true) {
			auto& ubo = mUBO[2]; 
			auto& spotlight = mCamera.GetComponent<Spotlight>();
			auto& transform = mCamera.GetComponent<Transform>();
			float inner_cos = spotlight.GetInnerCosine();
			float outer_cos = spotlight.GetOuterCosine();

			ubo.SetUniform(0, GetValPtr(spotlight.mColor));
			ubo.SetUniform(1, GetValPtr(transform.mPosition));
			ubo.SetUniform(2, GetValPtr(-transform.mForward));
			ubo.SetUniform(3, GetValPtr(spotlight.mIntensity));
			ubo.SetUniform(4, GetValPtr(inner_cos));
			ubo.SetUniform(5, GetValPtr(outer_cos));
			ubo.SetUniform(6, GetValPtr(spotlight.mRange));
		}

		// 更新轨道光源的统一缓冲对象（UBO）
		if (true) {
			auto& ubo = mUBO[3];
			auto& ot = mOrbitLight.GetComponent<Transform>();
			auto& ol = mOrbitLight.GetComponent<PointLight>();

			ubo.SetUniform(0, GetValPtr(ol.mColor));
			ubo.SetUniform(1, GetValPtr(ot.mPosition));
			ubo.SetUniform(2, GetValPtr(ol.mIntensity));
			ubo.SetUniform(3, GetValPtr(ol.mLinear));
			ubo.SetUniform(4, GetValPtr(ol.mQuadratic));
			ubo.SetUniform(5, GetValPtr(ol.mRange));
		}

		// 更新点光源缓冲区的统一缓冲对象（UBO）
		if (true) {
			auto& ubo = mUBO[4];
			auto& pl = mPointLights[0].GetComponent<PointLight>();
			ubo.SetUniform(0, GetValPtr(gLightClusterIntensity));
			ubo.SetUniform(1, GetValPtr(pl.mLinear));
			ubo.SetUniform(2, GetValPtr(pl.mQuadratic));
		}

		// 如果开启轨道动画，对轨道光源进行旋转
		if (gInputFromUI.IsOrbit) {
			mOrbitLight.GetComponent<Transform>().Rotate(world::up, gOrbitSpeed, Space::World);
		}

		// ------------------------------ 深度预通道 ------------------------------
		mFBO[0].Bind();
		mFBO[0].Clear(-1);
		Renderer::SetDepthTest(true);		// 开启深度测试
		Renderer::SetDepthPrepass(true);	// 启用早期Z测试
		// 独立渲染光源，所以不需要在预通道中渲染它们
		Renderer::Submit(mSphere.mId, mRunestone.mId);
		Renderer::Submit(gInputFromUI.IsShowPlane ? mPlane.mId : entt::null);
		Renderer::Submit(mSkybox.mId);
		Renderer::Render();
		Renderer::SetDepthPrepass(false);  // 关闭早期Z测试

		if (gInputFromUI.IsDrawDepthBuffer) {
			Renderer::SetDepthTest(false);  // 关闭深度测试
			mFBO[0].UnBind();  // 返回默认帧缓冲区
			Renderer::Clear();
			mFBO[0].Draw(-1);
			return;
		}

		// ------------------------------ 分派光照剔除 ------------------------------
		mFBO[0].GetDepthTexture().Bind(0);
		mPlIndex->Clear();  // 清空光源索引缓冲区

		auto cullShader = mResourceManager.Get<CShader>(10);
		cullShader->SetUniform(0, gPlsNum);  // 设置光源数量
		cullShader->SetUniform(1, glm::inverse(mainCamera.GetProjectionMatrix()));

		// 在此步骤中，我们只调度计算着色器来更新光源索引SSBO
		// 注意，`SyncWait()` 应该放置在实际使用计算着色器输出的代码附近，
		// 以避免在计算重的情况下不必要地等待
		cullShader->Bind();
		cullShader->Dispatch(gNx, gNy, 1);
		cullShader->SyncWait();
		cullShader->UnBind();

		// ------------------------------ MRT 渲染通道 ------------------------------
		// 在光照剔除后的实际着色通道，此时我们知道每个瓦片中将贡献光照的所有可见光源的索引，
		// 因此不再需要在片段着色器中循环每个光源。在此通道中，我们仍然拥有场景中实体的几何数据，
		// 因此MSAA将正常工作。在此之后，我们将无法应用MSAA。
		mFBO[1].Clear();
		mFBO[1].Bind();
		Renderer::SetMSAA(true);  // 启用多重采样抗锯齿
		Renderer::SetDepthTest(true);  // 开启深度测试
		Renderer::Submit(mSphere.mId, mRunestone.mId);
		Renderer::Submit(mSphere.mId);
		Renderer::Submit(gInputFromUI.IsShowPlane ? mPlane.mId : entt::null);

		// 多个圆组成的圆圈
		if (gInputFromUI.IsShowLightCluster) {
			for (int i = 0; i < gPlsNum; ++i) {
				Renderer::Submit(mPointLights[i].mId);
			}
		}
		// 旋转的圆
		Renderer::Submit(mOrbitLight.mId);
		Renderer::Submit(mSkybox.mId);
		Renderer::Render();
		mFBO[1].UnBind();

		// ------------------------------ MSAA 解析通道 ------------------------------
		mFBO[2].Clear();
		FBO::CopyColor(mFBO[1], 0, mFBO[2], 0);
		FBO::CopyColor(mFBO[1], 1, mFBO[2], 1);

		// ------------------------------ 应用高斯模糊 ------------------------------
		FBO::CopyColor(mFBO[2], 1, mFBO[3], 0);  // 降采样泛光目标（最近邻过滤）
		auto& ping = mFBO[3].GetColorTexture(0);
		auto& pong = mFBO[3].GetColorTexture(1);
		auto bloomShader = mResourceManager.Get<CShader>(00);
		bloomShader->Bind();
		ping.BindILS(0, 0, GL_READ_WRITE);
		pong.BindILS(0, 1, GL_READ_WRITE);

		for (int i = 0; i < 2 * gInputFromUI.BlursNum; ++i) {
			bloomShader->SetUniform(0, i % 2 == 0);
			bloomShader->Dispatch(ping.mWidth / 32, ping.mWidth / 18);
			bloomShader->SyncWait(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
		}

		// ------------------------------ 后处理通道 ------------------------------
		mFBO[2].GetColorTexture(0).Bind(0);  // 绑定颜色纹理
		mFBO[3].GetColorTexture(0).Bind(1);  // 绑定泛光纹理
		auto bilinearSampler = mResourceManager.Get<Sampler>(99);
		bilinearSampler->Bind(1);  // 上采样泛光纹理（双线性过滤）
		auto postprocessShader = mResourceManager.Get<Shader>(05);
		postprocessShader->Bind();
		postprocessShader->SetUniform(0, gInputFromUI.ToneMappingMode);  // 设置色调映射模式

		Renderer::Clear();
		Mesh::DrawQuad();  // 绘制全屏四边形
		postprocessShader->UnBind();
		bilinearSampler->UnBind(1);
	}

	void Scene01::OnImGuiRender()
	{
		using namespace ImGui;

		const ImVec4 textColor = ImVec4(0.4f, 0.8f, 0.4f, 1.0f);
		const ImGuiColorEditFlags colorFlags		// 颜色选择器标志
			= ImGuiColorEditFlags_NoSidePreview		// 无侧面预览
			| ImGuiColorEditFlags_PickerHueWheel	// 使用色调轮进行选择
			| ImGuiColorEditFlags_DisplayRGB		// 显示 RGB 值
			| ImGuiColorEditFlags_NoPicker;			// 禁用颜色选择器

		const char toneMappingGuide[] = "在下方选择一个色调映射操作。";  // 色调映射操作指南
		const char toneMappingNote[] = "近似的ACES胶片色调映射可能不是最吸引人的，但它是最物理正确的。";  // 色调映射注释

		const char* toneMappings[] = {
			"Simple Reinhard", "Reinhard-Jodie (Shadertoy)", "Uncharted 2 Hable Filmic", "Approximated ACES (UE4)"
		};  // 色调映射选项

		static bool showSphereGizmo = false;		// 显示球体操作工具
		static bool showPlaneGizmo = false;			// 显示平面操作工具
		static bool editSphereAlbedo = false;		// 编辑球体反照率
		static bool editFlashlightColor = false;	// 编辑手电筒颜色

		// 新的检查器面板
		if (UI::NewInspector() && BeginTabBar("InspectorTab", ImGuiTabBarFlags_None)) {
			Indent(5.0f);

			// 场景选项卡
			if (BeginTabItem(UI::ToU8("场景"))) {
				Checkbox(UI::ToU8("显示平面"), &gInputFromUI.IsShowPlane);						Separator();

				Checkbox(UI::ToU8("显示光源簇"), &gInputFromUI.IsShowLightCluster);				Separator();
				Checkbox(UI::ToU8("光源环绕"), &gInputFromUI.IsOrbit);                         Separator();
				Checkbox(UI::ToU8("显示球体操作工具"), &showSphereGizmo);						Separator();
				Checkbox(UI::ToU8("显示平面操作工具"), &showPlaneGizmo);							Separator();
				Checkbox(UI::ToU8("可视化深度缓冲区"), &gInputFromUI.IsDrawDepthBuffer);			Separator();

				PushItemWidth(130.0f);
				SliderFloat(UI::ToU8("光源簇强度"), &gInputFromUI.LightClusterIntensity, 3.0f, 20.0f);		Separator();
				SliderFloat(UI::ToU8("天空盒曝光"), &gInputFromUI.SkyboxExposure, 1.2f, 8.0f);				Separator();
				SliderFloat(UI::ToU8("天空盒LOD"), &gInputFromUI.SkyboxLod, 0.0f, 7.0f);					Separator();
				PopItemWidth();
				Spacing();


				if (Button(UI::ToU8("新颜色"), ImVec2(130.0f, 0.0f))) {
					UpdatePLColors();  // 更新点光源颜色
				}

				SameLine(0.0f, 10.0f);
				Text(UI::ToU8("刷新光源簇RGB"));
				if (IsItemHovered()) {
					SetTooltip(UI::ToU8("将随机创建新颜色。"));  // 提示
				}

				Spacing();
				Separator();
				EndTabItem();
			}

			// 材质选项卡
			if (BeginTabItem(UI::ToU8("材质"))) {
				PushItemWidth(100.0f);
				SliderFloat(UI::ToU8("球体金属度"), &gInputFromUI.SphereMetalness, 0.05f, 1.0f);
				SliderFloat(UI::ToU8("球体粗糙度"), &gInputFromUI.SphereRoughness, 0.05f, 1.0f);
				SliderFloat(UI::ToU8("球体AO"), &gInputFromUI.SphereAO, 0.0f, 1.0f);
				SliderFloat(UI::ToU8("平面粗糙度"), &gInputFromUI.PlaneRoughness, 0.1f, 0.3f);
				PopItemWidth();
				Separator();

				Checkbox(UI::ToU8("编辑球体反照率"), &editSphereAlbedo);  // 编辑球体反照率
				if (editSphereAlbedo) {
					Spacing();
					Indent(10.0f);
					ColorPicker3("##Sphere Albedo", utils::GetValPtr(gInputFromUI.SphereAlbedo), colorFlags);  // 球体反照率颜色选择器
					ImGui::Unindent(10.0f);
				}

				Checkbox(UI::ToU8("编辑手电筒颜色"), &editFlashlightColor);  // 编辑手电筒颜色
				if (editFlashlightColor) {
					Spacing();
					Indent(10.0f);
					ColorPicker3("##Flash", utils::GetValPtr(mCamera.GetComponent<Spotlight>().mColor), colorFlags);  // 手电筒颜色选择器
					ImGui::Unindent(10.0f);
				}
				ImGui::EndTabItem();
			}


			// HDR/Bloom 选项卡
			if (BeginTabItem(UI::ToU8("HDR/泛光"))) {
				PushItemWidth(180.0f);
				Text(UI::ToU8("泛光效果倍增器 (光源簇)"));
				SliderInt("##Bloom", &gInputFromUI.BlursNum, 3, 5);  // 泛光效果倍增器
				PopItemWidth();
				Separator();
				PushStyleColor(ImGuiCol_Text, textColor);
				PushTextWrapPos(280.0f);
				TextWrapped(UI::ToU8(toneMappingGuide));  // 色调映射指南
				TextWrapped(UI::ToU8(toneMappingNote));   // 色调映射注释
				PopTextWrapPos();
				PopStyleColor();
				Separator();
				PushItemWidth(295.0f);
				Combo(" ", &gInputFromUI.ToneMappingMode, toneMappings, 4);  // 色调映射模式选择
				PopItemWidth();
				EndTabItem();
			}

			EndTabBar();
			Unindent(5.0f);
			UI::EndInspector();
		}

		// 显示球体操作工具
		if (showSphereGizmo) {
		    UI::DrawGizmo(mCamera, mSphere, Gizmo::Translate);
		}

		// 如果显示平面并且显示平面操作工具，则显示平面操作工具
		if (showPlaneGizmo && gInputFromUI.IsShowPlane) {
		    UI::DrawGizmo(mCamera, mPlane, Gizmo::Translate);
		}
	}

	void Scene01::PrecomputeIBL(const std::string& hdri)
	{
		Renderer::SetSeamlessCubemap(true);
		Renderer::SetDepthTest(false);
		Renderer::SetFaceCulling(true);

		auto shaderIrradiance = CShader("core\\irradiance_map.glsl");	// 辐照度贴图着色器
		auto shaderPrefilter = CShader("core\\prefilter_envmap.glsl");  // 预过滤环境贴图着色器
		auto shaderEnvBRDF = CShader("core\\environment_BRDF.glsl");	// 环境BRDF LUT着色器

		auto mapEnv = std::make_shared<Texture>(hdri, ".hdr", 2048, 0);  // 创建环境贴图纹理对象
		mapEnv->Bind(0);  // 绑定环境贴图到纹理单元0

		mIrradianceMap = std::make_shared<Texture>(GL_TEXTURE_CUBE_MAP, 128, 128, 6, GL_RGBA16F, 1);		// 创建辐照度贴图
		mPrefilteredMap = std::make_shared<Texture>(GL_TEXTURE_CUBE_MAP, 2048, 2048, 6, GL_RGBA16F, 8);		// 创建预过滤环境贴图
		mBRDF_LUT = std::make_shared<Texture>(GL_TEXTURE_2D, 1024, 1024, 1, GL_RGBA16F, 1);					// 创建环境BRDF LUT
		
		LOG_INFO("预计算漫反射辐照度图:{0}", hdri);
		mIrradianceMap->BindILS(0, 0, GL_WRITE_ONLY);  // 绑定辐照度贴图到图像存储缓冲

		if (true) {  // 绑定辐照度贴图着色器
			shaderIrradiance.Bind();
			shaderIrradiance.Dispatch(128 / 32, 128 / 32, 6);  // 分发计算任务
			shaderIrradiance.SyncWait(GL_TEXTURE_FETCH_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);  // 等待同步

			auto irradianceFence = Sync(0);  // 同步标记
			irradianceFence.WaitClientSync();
			mIrradianceMap->UnbindILS(0);  // 解绑辐照度贴图
		}

		LOG_INFO("预计算镜面预过滤环境贴图:{0}", hdri);
		Texture::Copy(*mapEnv, 0, *mPrefilteredMap, 0);  // 复制基础级别

		const GLuint maxLevel = mPrefilteredMap->mLevel - 1;	// 最大级别
		GLuint resolution = mPrefilteredMap->mWidth / 2;		// 分辨率
		shaderPrefilter.Bind();									// 绑定预过滤环境贴图着色器
		for (unsigned int level = 1; level <= maxLevel; level++, resolution /= 2) {
			float roughness = level / static_cast<float>(maxLevel);			// 计算粗糙度
			GLuint numGroups = glm::max<GLuint>(resolution / 32, 1);		// 计算分组数

			mPrefilteredMap->BindILS(level, 1, GL_WRITE_ONLY);				// 绑定预过滤环境贴图到图像存储缓冲
			shaderPrefilter.SetUniform(0, roughness);						// 设置粗糙度参数
			shaderPrefilter.Dispatch(numGroups, numGroups, 6);				// 分发计算任务
			shaderPrefilter.SyncWait(GL_TEXTURE_FETCH_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);  // 等待同步

			auto prefilterFence = Sync(level);		// 同步标记
			prefilterFence.WaitClientSync();		// 客户端等待同步
			mPrefilteredMap->UnbindILS(1);			// 解绑预过滤环境贴图
		}

		LOG_INFO("预计算镜面环境 BRDF LUT:{0}", hdri);		// 输出日志，预计算镜面环境BRDF LUT
		mBRDF_LUT->BindILS(0, 2, GL_WRITE_ONLY);		// 绑定环境BRDF LUT到图像存储缓冲

		if (true) {				// 绑定环境BRDF LUT着色器
			shaderEnvBRDF.Bind();
			shaderEnvBRDF.Dispatch(1024 / 32, 1024 / 32, 1);	// 分发计算任务
			shaderEnvBRDF.SyncWait(GL_ALL_BARRIER_BITS);		// 等待同步
			Sync::WaitFinish();									// 等待计算完成
			mBRDF_LUT->UnbindILS(2);							// 解绑环境BRDF LUT
		}
	}
	void Scene01::SetMaterial(Material& pbrMaterial, int id)
	{
		LOG_TRACK;
		pbrMaterial.SetTexture(pbr_t::irradiance_map, mIrradianceMap);  // 设置漫反射辐照度贴图
		pbrMaterial.SetTexture(pbr_t::prefiltered_map, mPrefilteredMap);  // 设置预过滤环境贴图
		pbrMaterial.SetTexture(pbr_t::BRDF_LUT, mBRDF_LUT);  // 设置环境BRDF LUT

		std::string path = "model\\runestone\\";
		switch (id){
		case 0: {
			LOG_INFO("设置球体材质");
			pbrMaterial.BindUniform(pbr_u::albedo, &gInputFromUI.SphereAlbedo);  // 绑定反照率Uniform
			pbrMaterial.BindUniform(pbr_u::metalness, &gInputFromUI.SphereMetalness);  // 绑定金属度Uniform
			pbrMaterial.BindUniform(pbr_u::roughness, &gInputFromUI.SphereRoughness);  // 绑定粗糙度Uniform
			pbrMaterial.BindUniform(pbr_u::ao, &gInputFromUI.SphereAO);  // 绑定环境光遮蔽Uniform
			break;
		}
		case 2: {
			LOG_INFO("设置平面材质");
			pbrMaterial.SetTexture(pbr_t::albedo, std::make_shared<Texture>("texture\\common\\checkboard.png"));  // 设置反照率贴图
			pbrMaterial.SetUniform(pbr_u::metalness, 0.1f);  // 设置金属度Uniform
			pbrMaterial.BindUniform(pbr_u::roughness, &gInputFromUI.PlaneRoughness);  // 绑定粗糙度Uniform
			pbrMaterial.SetUniform(pbr_u::uv_scale, vec2(8.0f));  // 设置UV缩放Uniform
			break;
		}
		case 31: {
			LOG_INFO("设置符文石柱材质");
			pbrMaterial.SetTexture(pbr_t::albedo, std::make_shared<Texture>(path+"pillars_albedo.png"));  // 设置反照率贴图
			pbrMaterial.SetTexture(pbr_t::normal, std::make_shared<Texture>(path+"pillars_normal.png"));  // 设置法线贴图
			pbrMaterial.SetTexture(pbr_t::metallic, std::make_shared<Texture>(path+"pillars_metallic.png"));  // 设置金属度贴图
			pbrMaterial.SetTexture(pbr_t::roughness, std::make_shared<Texture>(path+"pillars_roughness.png"));  // 设置粗糙度贴图
			break;
		}
		case 32: {	// 符文石平台材质
			LOG_INFO("设置符文石平台材质");
			pbrMaterial.SetTexture(pbr_t::albedo, std::make_shared<Texture>(path+"platform_albedo.png"));  // 设置反照率贴图
			pbrMaterial.SetTexture(pbr_t::normal, std::make_shared<Texture>(path+"platform_normal.png"));  // 设置法线贴图
			pbrMaterial.SetTexture(pbr_t::metallic, std::make_shared<Texture>(path+"platform_metallic.png"));  // 设置金属度贴图
			pbrMaterial.SetTexture(pbr_t::roughness, std::make_shared<Texture>(path+"platform_roughness.png"));  // 设置粗糙度贴图
			pbrMaterial.SetTexture(pbr_t::emission, std::make_shared<Texture>(path+"platform_emissive.png"));  // 设置发光贴图
			break;
		}
		default:
			LOG_ERROR("设置材质参数错误");
			break;
		}
	}
	void Scene01::SetPLBuffers()
	{
		gNx = (Window::mWidth + gTileSize - 1) / gTileSize;  // 计算X方向的网格数
		gNy = (Window::mHeight + gTileSize - 1) / gTileSize;  // 计算Y方向的网格数
		GLuint n_tiles = gNx * gNy;  // 计算总的网格数

		// 对于我们的分片前向渲染器，总共需要4个SSBO：颜色、位置和范围在GLSL着色器中是只读的，
		// 它们应该不时从CPU端更新，因此我们将保持对数据存储的指针，使用一致性持久映射。
		// 指针只能获取一次，无需释放，并且所有写操作会自动刷新到GPU。另一方面，索引SSBO由计算
		// 着色器更新，用于保存未被剔除的光源的索引，不应该对CPU可见，因此使用动态存储位。

		GLbitfield cpuAccess = GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT;
		GLbitfield gpuAccess = GL_DYNAMIC_STORAGE_BIT;

		mPlColor = std::make_unique<SSBO>(0, gPlsNum * sizeof(vec4), cpuAccess);			// 颜色SSBO
		mPlPosition = std::make_unique<SSBO>(1, gPlsNum * sizeof(vec4), cpuAccess);			// 位置SSBO
		mPlRange = std::make_unique<SSBO>(2, gPlsNum * sizeof(float), cpuAccess);			// 范围SSBO
		mPlIndex = std::make_unique<SSBO>(3, gPlsNum * n_tiles * sizeof(int), gpuAccess);	// 索引SSBO

		// 在分片前向渲染器中，光源剔除可以适用于静态和动态光源，后者需要用户每帧更新SSBO缓冲区。
		// 为简单起见，我们只对28个静态点光源执行光源剔除，因为这对大多数情况已足够，因此SSBO只需要设置一次。

		mPlPosition->Acquire(cpuAccess);  // 获取位置SSBO
		mPlColor->Acquire(cpuAccess);     // 获取颜色SSBO
		mPlRange->Acquire(cpuAccess);     // 获取范围SSBO

		auto* const posit_ptr = reinterpret_cast<vec4* const>(mPlPosition->GetData());  // 获取位置数据指针
		auto* const color_ptr = reinterpret_cast<vec4* const>(mPlColor->GetData());     // 获取颜色数据指针
		auto* const range_ptr = reinterpret_cast<float* const>(mPlRange->GetData());    // 获取范围数据指针

		for (int i = 0; i < gPlsNum; i++) {
			auto& pt = mPointLights[i].GetComponent<Transform>();  // 获取点光源的Transform组件
			auto& pl = mPointLights[i].GetComponent<PointLight>();  // 获取点光源的PointLight组件

			posit_ptr[i] = vec4(pt.mPosition, 1.0f);  // 更新位置数据
			color_ptr[i] = vec4(pl.mColor, 1.0f);     // 更新颜色数据
			range_ptr[i] = pl.mRange;                 // 更新范围数据
		}
	}
	void Scene01::UpdatePLColors()
	{
		LOG_TRACK;
		auto pColor = reinterpret_cast<vec4* const>(mPlColor->GetData());  // 获取点光源颜色数据指针
		for (int i = 0; i < gPlsNum; ++i) {
			auto hue = Math::RandomGenerator<float>();  // 随机生成色调
			auto color = Math::HSV2RGB(vec3(hue, 1.0f, 1.0f));  // 将色调转换为RGB颜色
			pColor[i] = vec4(color, 1.0f);  // 更新颜色数组中的值
			mPointLights[i].GetComponent<PointLight>().mColor = color;  // 更新点光源组件中的颜色属性
		}
	}
}