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
		bool IsShowPlane = true;        // �Ƿ���ʾƽ��
		bool IsShowLightCluster = true;  // �Ƿ���ʾ���ռ�Ⱥ
		bool IsOrbit = true;            // �Ƿ���й������
		bool IsDrawDepthBuffer = false; // �Ƿ������Ȼ�����

		float LightClusterIntensity = 10.0f;    // ���ռ�Ⱥǿ��
		float SkyboxExposure = 1.0f;    // ��պ��ع��
		float SkyboxLod = 0.0f;         // ��պ�LOD����
		float SphereMetalness = 0.05f;             // ���������
		float SphereRoughness = 0.05f;             // ����ֲڶ�
		float SphereAO = 1.0f;              // ���廷�����ڱ�
		float PlaneRoughness = 0.1f;              // ƽ��ֲڶ�
		glm::vec4  SphereAlbedo{ 0.22f, 0.0f, 1.0f, 1.0f };  // ���巴����
		int   BlursNum = 3;              // ģ������
		int   ToneMappingMode = 3;              // ɫ��ӳ��ģʽ
	};
	InputFromUI gInputFromUI;

	static bool  gShowGrid = false;									// �Ƿ���ʾ����
	static float gGridCellSize = 2.0f;								// ����Ԫ��С
	static vec4  gThinLineColor = vec4(0.1f, 0.1f, 0.1f, 1.0f);		// ϸ����ɫ
	static vec4  gWideLineColor = vec4(0.2f, 0.2f, 0.2f, 1.0f);		// ������ɫ
	static float gOrbitSpeed = 0.3f;								// ����ٶ�
	static float gLightClusterIntensity = 10.0f;					// ���ռ�Ⱥǿ��
	constexpr GLuint gTileSize = 16;								// ��Ƭ��С
	constexpr GLuint gPlsNum = 28;									// ���Դ����
	static GLuint gNx = 0, gNy = 0;									// ���ڳߴ�

	void Scene01::Init()
	{
		LOG_TRACK;
		mTitle = "ǰ����Ⱦ����";
		PrecomputeIBL("texture\\HDRI\\cosmic\\");

		// �����Դ�������еĸ�����Դ
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

		// ��������ɫ���е�ͳһ�������UBO�����ظ��Ľ�������
		AddUBO(mResourceManager.Get<Shader>(02)->getId());
		AddUBO(mResourceManager.Get<Shader>(03)->getId());
		AddUBO(mResourceManager.Get<Shader>(04)->getId());

		// �����м�֡�������FBO��
		AddFBO(Window::mWidth, Window::mHeight);
		AddFBO(Window::mWidth, Window::mHeight);
		AddFBO(Window::mWidth, Window::mHeight);
		AddFBO(Window::mWidth / 2, Window::mHeight / 2);

		mFBO[0].AddDepStTexture();           // �����Ⱥ�ģ�帽��
		mFBO[1].AddColorTexture(2, true);   // ��Ӵ���MIP��ͼ����ɫ����
		mFBO[1].AddDepStRenderBuffer(true); // �����Ⱥ�ģ�帽��
		mFBO[2].AddColorTexture(2);         // ��Ӵ���MIP��ͼ����ɫ����
		mFBO[3].AddColorTexture(2);         // ��Ӵ���MIP��ͼ����ɫ����

		// �������ʵ��
		mCamera = CreateEntity("Camera", ETag::MainCamera);
		mCamera.GetComponent<Transform>().Translate(vec3(0.0f, 6.0f, -16.0f));  // �������λ��
		mCamera.GetComponent<Transform>().Rotate(world::up, 180.0f, Space::Local);  // ��ת���
		mCamera.AddComponent<Camera>(View::Perspective);  // ���͸����������
		mCamera.AddComponent<Spotlight>(vec3(1.0f, 0.553f, 0.0f), 3.8f);  // ��Ӿ۹�����
		mCamera.GetComponent<Spotlight>().SetCutoff(12.0f);  // ���þ۹�ƵĹ�׶�Ƕ�

		// ������պ�ʵ��
		mSkybox = CreateEntity("Skybox", ETag::Skybox);
		mSkybox.AddComponent<Mesh>(Primitive::Cube);  // ����������������
		if ( true) {
			auto& mat = mSkybox.AddComponent<Material>(mResourceManager.Get<Material>(12));
			mat.SetTexture(0, mPrefilteredMap);  // ���ò�������
			mat.BindUniform(0, &gInputFromUI.SkyboxExposure);  // �󶨲���ͳһ����
			mat.BindUniform(1, &gInputFromUI.SkyboxLod);  // �󶨲���ͳһ����
		}

		// ��������ʵ��
		auto sphereMesh = mResourceManager.Get<Mesh>(-1);  // ��ȡ����������Դ
		mSphere = CreateEntity("Sphere");
		mSphere.AddComponent<Mesh>(sphereMesh);  // ��������������
		mSphere.GetComponent<Transform>().Translate(world::up * 8.5f);  // ��������λ��
		mSphere.GetComponent<Transform>().Scale(2.0f);  // ��������
		SetMaterial(mSphere.AddComponent<Material>(mResourceManager.Get<Material>(14)), 0);  // �����������

		// ����ƽ��ʵ��
		mPlane = CreateEntity("Plane");
		mPlane.AddComponent<Mesh>(Primitive::Plane);  // ���ƽ���������
		mPlane.GetComponent<Transform>().Translate(world::down * 4.0f);  // ����ƽ��λ��
		mPlane.GetComponent<Transform>().Scale(3.0f);  // ����ƽ��
		SetMaterial(mPlane.AddComponent<Material>(mResourceManager.Get<Material>(14)), 2);  // ����ƽ�����

		// ��������ʯʵ��
		mRunestone = CreateEntity("Runestone");
		mRunestone.GetComponent<Transform>().Scale(0.02f);  // ���ŷ���ʯ
		mRunestone.GetComponent<Transform>().Translate(world::down * 4.0f);  // ���÷���ʯλ��
		// ����ģ�Ͳ����ò���
		if (true) {
			auto& model = mRunestone.AddComponent<Model>("model\\runestone\\runestone.fbx", Quality::Auto);  // ���ģ�����
			SetMaterial(model.SetMaterial("pillars", mResourceManager.Get<Material>(14)), 31);  // ����ģ�Ͳ���
			SetMaterial(model.SetMaterial("platform", mResourceManager.Get<Material>(14)), 32);  // ����ģ�Ͳ���
		}

		// ���������ʵ��
		mDirectLight = CreateEntity("Directional Light");
		mDirectLight.GetComponent<Transform>().Rotate(world::left, 45.0f, Space::Local);	// ��ת����ⷽ��
		mDirectLight.AddComponent<DirectionLight>(color::white, 0.2f);						// ��Ӷ�������

		// ���ھ�̬��Դ��ֻ����`Init()`������һ��ͳһ������
		if ( true) {
			auto& ubo = mUBO[1];
			auto& dl = mDirectLight.GetComponent<DirectionLight>();
			auto& dt = mDirectLight.GetComponent<Transform>();
			ubo.SetUniform(0, GetValPtr(dl.mColor));  // ����ͳһ����
			ubo.SetUniform(1, GetValPtr(-dt.mForward));  // ����ͳһ����
			ubo.SetUniform(2, GetValPtr(dl.mIntensity));  // ����ͳһ����
		}

		// ���������ʵ��
		mOrbitLight = CreateEntity("Orbit Light");
		mOrbitLight.GetComponent<Transform>().Translate(0.0f, 8.0f, 4.5f);  // ���ù����λ��
		mOrbitLight.GetComponent<Transform>().Scale(0.3f);  // ���Ź����
		mOrbitLight.AddComponent<PointLight>(color::lime, 0.8f);  // ��ӵ��Դ���
		mOrbitLight.GetComponent<PointLight>().SetAttenuation(0.09f, 0.032f);  // ���õ��Դ˥��
		mOrbitLight.AddComponent<Mesh>(sphereMesh);  // ��������������
		if (true) {
			auto& mat = mOrbitLight.AddComponent<Material>(mResourceManager.Get<Material>(13));
			auto& pl = mOrbitLight.GetComponent<PointLight>();
			mat.SetUniform(3, pl.mColor);  // ���ò���ͳһ����
			mat.SetUniform(4, pl.mIntensity);  // ���ò���ͳһ����
			mat.SetUniform(5, 2.0f);  // ���ò���ͳһ���������ⱶ��������ɫ���Ͷȣ�
		}

		// ����һ����СΪ8x8������64����Ԫ����ѡ��ÿ���߽絥Ԫ����Ϊһ�����Դ���ܹ�28�����Դ��
		for (int i = 0, index = 0; i < 64; i++) {
			int row = i / 8;	// ������ [0, 7]
			int col = i % 8;	// ������ [0, 7]
			if (bool onBorder = row == 0 || row == 7 || col == 0 || col == 7; !onBorder) {
				continue;		// �����м�ĵ�Ԫ��
			}

			float ksi = Math::RandomGenerator<float>();  // ���������ksi
			vec3 color = Math::HSL2RGB(ksi, 0.7f + ksi * 0.3f, 0.4f + ksi * 0.2f);  // ���������ɫ
			vec3 position = vec3(row - 3.5f, 1.5f, col - 3.5f) * 9.0f;  // ������Դλ��

			auto& pl = mPointLights[index];  // ��ȡ���Դʵ�������
			pl = CreateEntity("Point Light " + std::to_string(index));  // �������Դʵ��
			pl.GetComponent<Transform>().Translate(position - world::origin);  // ���õ��Դλ��
			pl.GetComponent<Transform>().Scale(0.8f);  // ���ŵ��Դ
			pl.AddComponent<PointLight>(color, 1.5f);  // ��ӵ��Դ���
			pl.AddComponent<Mesh>(sphereMesh);  // ��������������

			auto& mat = pl.AddComponent<Material>(mResourceManager.Get<Material>(13));  // ��ȡ��������
			auto& ppl = pl.GetComponent<PointLight>();  // ��ȡ���Դ�������

			ppl.SetAttenuation(0.09f, 0.032f);  // ���õ��Դ˥��
			mat.BindUniform(3, &ppl.mColor);  // �󶨲���ͳһ����
			mat.SetUniform(4, ppl.mIntensity);  // ���ò���ͳһ����
			mat.SetUniform(5, 7.0f);  // ���ò���ͳһ���������ⱶ��������ɫ���Ͷȣ�
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
		// �������������ͳһ�������UBO��
		if (true) {
			auto& ubo = mUBO[0];
			ubo.SetUniform(0, GetValPtr(mainCamera.mpTransform->mPosition));
			ubo.SetUniform(1, GetValPtr(mainCamera.mpTransform->mForward));
			ubo.SetUniform(2, GetValPtr(mainCamera.GetViewMatrix()));
			ubo.SetUniform(3, GetValPtr(mainCamera.GetProjectionMatrix()));
		}

		// �����ֵ�Ͳ��Դ��ͳһ�������UBO��
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

		// ���¹����Դ��ͳһ�������UBO��
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

		// ���µ��Դ��������ͳһ�������UBO��
		if (true) {
			auto& ubo = mUBO[4];
			auto& pl = mPointLights[0].GetComponent<PointLight>();
			ubo.SetUniform(0, GetValPtr(gLightClusterIntensity));
			ubo.SetUniform(1, GetValPtr(pl.mLinear));
			ubo.SetUniform(2, GetValPtr(pl.mQuadratic));
		}

		// �����������������Թ����Դ������ת
		if (gInputFromUI.IsOrbit) {
			mOrbitLight.GetComponent<Transform>().Rotate(world::up, gOrbitSpeed, Space::World);
		}

		// ------------------------------ ���Ԥͨ�� ------------------------------
		mFBO[0].Bind();
		mFBO[0].Clear(-1);
		Renderer::SetDepthTest(true);		// ������Ȳ���
		Renderer::SetDepthPrepass(true);	// ��������Z����
		// ������Ⱦ��Դ�����Բ���Ҫ��Ԥͨ������Ⱦ����
		Renderer::Submit(mSphere.mId, mRunestone.mId);
		Renderer::Submit(gInputFromUI.IsShowPlane ? mPlane.mId : entt::null);
		Renderer::Submit(mSkybox.mId);
		Renderer::Render();
		Renderer::SetDepthPrepass(false);  // �ر�����Z����

		if (gInputFromUI.IsDrawDepthBuffer) {
			Renderer::SetDepthTest(false);  // �ر���Ȳ���
			mFBO[0].UnBind();  // ����Ĭ��֡������
			Renderer::Clear();
			mFBO[0].Draw(-1);
			return;
		}

		// ------------------------------ ���ɹ����޳� ------------------------------
		mFBO[0].GetDepthTexture().Bind(0);
		mPlIndex->Clear();  // ��չ�Դ����������

		auto cullShader = mResourceManager.Get<CShader>(10);
		cullShader->SetUniform(0, gPlsNum);  // ���ù�Դ����
		cullShader->SetUniform(1, glm::inverse(mainCamera.GetProjectionMatrix()));

		// �ڴ˲����У�����ֻ���ȼ�����ɫ�������¹�Դ����SSBO
		// ע�⣬`SyncWait()` Ӧ�÷�����ʵ��ʹ�ü�����ɫ������Ĵ��븽����
		// �Ա����ڼ����ص�����²���Ҫ�صȴ�
		cullShader->Bind();
		cullShader->Dispatch(gNx, gNy, 1);
		cullShader->SyncWait();
		cullShader->UnBind();

		// ------------------------------ MRT ��Ⱦͨ�� ------------------------------
		// �ڹ����޳����ʵ����ɫͨ������ʱ����֪��ÿ����Ƭ�н����׹��յ����пɼ���Դ��������
		// ��˲�����Ҫ��Ƭ����ɫ����ѭ��ÿ����Դ���ڴ�ͨ���У�������Ȼӵ�г�����ʵ��ļ������ݣ�
		// ���MSAA�������������ڴ�֮�����ǽ��޷�Ӧ��MSAA��
		mFBO[1].Clear();
		mFBO[1].Bind();
		Renderer::SetMSAA(true);  // ���ö��ز��������
		Renderer::SetDepthTest(true);  // ������Ȳ���
		Renderer::Submit(mSphere.mId, mRunestone.mId);
		Renderer::Submit(mSphere.mId);
		Renderer::Submit(gInputFromUI.IsShowPlane ? mPlane.mId : entt::null);

		// ���Բ��ɵ�ԲȦ
		if (gInputFromUI.IsShowLightCluster) {
			for (int i = 0; i < gPlsNum; ++i) {
				Renderer::Submit(mPointLights[i].mId);
			}
		}
		// ��ת��Բ
		Renderer::Submit(mOrbitLight.mId);
		Renderer::Submit(mSkybox.mId);
		Renderer::Render();
		mFBO[1].UnBind();

		// ------------------------------ MSAA ����ͨ�� ------------------------------
		mFBO[2].Clear();
		FBO::CopyColor(mFBO[1], 0, mFBO[2], 0);
		FBO::CopyColor(mFBO[1], 1, mFBO[2], 1);

		// ------------------------------ Ӧ�ø�˹ģ�� ------------------------------
		FBO::CopyColor(mFBO[2], 1, mFBO[3], 0);  // ����������Ŀ�꣨����ڹ��ˣ�
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

		// ------------------------------ ����ͨ�� ------------------------------
		mFBO[2].GetColorTexture(0).Bind(0);  // ����ɫ����
		mFBO[3].GetColorTexture(0).Bind(1);  // �󶨷�������
		auto bilinearSampler = mResourceManager.Get<Sampler>(99);
		bilinearSampler->Bind(1);  // �ϲ�����������˫���Թ��ˣ�
		auto postprocessShader = mResourceManager.Get<Shader>(05);
		postprocessShader->Bind();
		postprocessShader->SetUniform(0, gInputFromUI.ToneMappingMode);  // ����ɫ��ӳ��ģʽ

		Renderer::Clear();
		Mesh::DrawQuad();  // ����ȫ���ı���
		postprocessShader->UnBind();
		bilinearSampler->UnBind(1);
	}

	void Scene01::OnImGuiRender()
	{
		using namespace ImGui;

		const ImVec4 textColor = ImVec4(0.4f, 0.8f, 0.4f, 1.0f);
		const ImGuiColorEditFlags colorFlags		// ��ɫѡ������־
			= ImGuiColorEditFlags_NoSidePreview		// �޲���Ԥ��
			| ImGuiColorEditFlags_PickerHueWheel	// ʹ��ɫ���ֽ���ѡ��
			| ImGuiColorEditFlags_DisplayRGB		// ��ʾ RGB ֵ
			| ImGuiColorEditFlags_NoPicker;			// ������ɫѡ����

		const char toneMappingGuide[] = "���·�ѡ��һ��ɫ��ӳ�������";  // ɫ��ӳ�����ָ��
		const char toneMappingNote[] = "���Ƶ�ACES��Ƭɫ��ӳ����ܲ����������˵ģ���������������ȷ�ġ�";  // ɫ��ӳ��ע��

		const char* toneMappings[] = {
			"Simple Reinhard", "Reinhard-Jodie (Shadertoy)", "Uncharted 2 Hable Filmic", "Approximated ACES (UE4)"
		};  // ɫ��ӳ��ѡ��

		static bool showSphereGizmo = false;		// ��ʾ�����������
		static bool showPlaneGizmo = false;			// ��ʾƽ���������
		static bool editSphereAlbedo = false;		// �༭���巴����
		static bool editFlashlightColor = false;	// �༭�ֵ�Ͳ��ɫ

		// �µļ�������
		if (UI::NewInspector() && BeginTabBar("InspectorTab", ImGuiTabBarFlags_None)) {
			Indent(5.0f);

			// ����ѡ�
			if (BeginTabItem(UI::ToU8("����"))) {
				Checkbox(UI::ToU8("��ʾƽ��"), &gInputFromUI.IsShowPlane);						Separator();

				Checkbox(UI::ToU8("��ʾ��Դ��"), &gInputFromUI.IsShowLightCluster);				Separator();
				Checkbox(UI::ToU8("��Դ����"), &gInputFromUI.IsOrbit);                         Separator();
				Checkbox(UI::ToU8("��ʾ�����������"), &showSphereGizmo);						Separator();
				Checkbox(UI::ToU8("��ʾƽ���������"), &showPlaneGizmo);							Separator();
				Checkbox(UI::ToU8("���ӻ���Ȼ�����"), &gInputFromUI.IsDrawDepthBuffer);			Separator();

				PushItemWidth(130.0f);
				SliderFloat(UI::ToU8("��Դ��ǿ��"), &gInputFromUI.LightClusterIntensity, 3.0f, 20.0f);		Separator();
				SliderFloat(UI::ToU8("��պ��ع�"), &gInputFromUI.SkyboxExposure, 1.2f, 8.0f);				Separator();
				SliderFloat(UI::ToU8("��պ�LOD"), &gInputFromUI.SkyboxLod, 0.0f, 7.0f);					Separator();
				PopItemWidth();
				Spacing();


				if (Button(UI::ToU8("����ɫ"), ImVec2(130.0f, 0.0f))) {
					UpdatePLColors();  // ���µ��Դ��ɫ
				}

				SameLine(0.0f, 10.0f);
				Text(UI::ToU8("ˢ�¹�Դ��RGB"));
				if (IsItemHovered()) {
					SetTooltip(UI::ToU8("�������������ɫ��"));  // ��ʾ
				}

				Spacing();
				Separator();
				EndTabItem();
			}

			// ����ѡ�
			if (BeginTabItem(UI::ToU8("����"))) {
				PushItemWidth(100.0f);
				SliderFloat(UI::ToU8("���������"), &gInputFromUI.SphereMetalness, 0.05f, 1.0f);
				SliderFloat(UI::ToU8("����ֲڶ�"), &gInputFromUI.SphereRoughness, 0.05f, 1.0f);
				SliderFloat(UI::ToU8("����AO"), &gInputFromUI.SphereAO, 0.0f, 1.0f);
				SliderFloat(UI::ToU8("ƽ��ֲڶ�"), &gInputFromUI.PlaneRoughness, 0.1f, 0.3f);
				PopItemWidth();
				Separator();

				Checkbox(UI::ToU8("�༭���巴����"), &editSphereAlbedo);  // �༭���巴����
				if (editSphereAlbedo) {
					Spacing();
					Indent(10.0f);
					ColorPicker3("##Sphere Albedo", utils::GetValPtr(gInputFromUI.SphereAlbedo), colorFlags);  // ���巴������ɫѡ����
					ImGui::Unindent(10.0f);
				}

				Checkbox(UI::ToU8("�༭�ֵ�Ͳ��ɫ"), &editFlashlightColor);  // �༭�ֵ�Ͳ��ɫ
				if (editFlashlightColor) {
					Spacing();
					Indent(10.0f);
					ColorPicker3("##Flash", utils::GetValPtr(mCamera.GetComponent<Spotlight>().mColor), colorFlags);  // �ֵ�Ͳ��ɫѡ����
					ImGui::Unindent(10.0f);
				}
				ImGui::EndTabItem();
			}


			// HDR/Bloom ѡ�
			if (BeginTabItem(UI::ToU8("HDR/����"))) {
				PushItemWidth(180.0f);
				Text(UI::ToU8("����Ч�������� (��Դ��)"));
				SliderInt("##Bloom", &gInputFromUI.BlursNum, 3, 5);  // ����Ч��������
				PopItemWidth();
				Separator();
				PushStyleColor(ImGuiCol_Text, textColor);
				PushTextWrapPos(280.0f);
				TextWrapped(UI::ToU8(toneMappingGuide));  // ɫ��ӳ��ָ��
				TextWrapped(UI::ToU8(toneMappingNote));   // ɫ��ӳ��ע��
				PopTextWrapPos();
				PopStyleColor();
				Separator();
				PushItemWidth(295.0f);
				Combo(" ", &gInputFromUI.ToneMappingMode, toneMappings, 4);  // ɫ��ӳ��ģʽѡ��
				PopItemWidth();
				EndTabItem();
			}

			EndTabBar();
			Unindent(5.0f);
			UI::EndInspector();
		}

		// ��ʾ�����������
		if (showSphereGizmo) {
		    UI::DrawGizmo(mCamera, mSphere, Gizmo::Translate);
		}

		// �����ʾƽ�沢����ʾƽ��������ߣ�����ʾƽ���������
		if (showPlaneGizmo && gInputFromUI.IsShowPlane) {
		    UI::DrawGizmo(mCamera, mPlane, Gizmo::Translate);
		}
	}

	void Scene01::PrecomputeIBL(const std::string& hdri)
	{
		Renderer::SetSeamlessCubemap(true);
		Renderer::SetDepthTest(false);
		Renderer::SetFaceCulling(true);

		auto shaderIrradiance = CShader("core\\irradiance_map.glsl");	// ���ն���ͼ��ɫ��
		auto shaderPrefilter = CShader("core\\prefilter_envmap.glsl");  // Ԥ���˻�����ͼ��ɫ��
		auto shaderEnvBRDF = CShader("core\\environment_BRDF.glsl");	// ����BRDF LUT��ɫ��

		auto mapEnv = std::make_shared<Texture>(hdri, ".hdr", 2048, 0);  // ����������ͼ�������
		mapEnv->Bind(0);  // �󶨻�����ͼ������Ԫ0

		mIrradianceMap = std::make_shared<Texture>(GL_TEXTURE_CUBE_MAP, 128, 128, 6, GL_RGBA16F, 1);		// �������ն���ͼ
		mPrefilteredMap = std::make_shared<Texture>(GL_TEXTURE_CUBE_MAP, 2048, 2048, 6, GL_RGBA16F, 8);		// ����Ԥ���˻�����ͼ
		mBRDF_LUT = std::make_shared<Texture>(GL_TEXTURE_2D, 1024, 1024, 1, GL_RGBA16F, 1);					// ��������BRDF LUT
		
		LOG_INFO("Ԥ������������ն�ͼ:{0}", hdri);
		mIrradianceMap->BindILS(0, 0, GL_WRITE_ONLY);  // �󶨷��ն���ͼ��ͼ��洢����

		if (true) {  // �󶨷��ն���ͼ��ɫ��
			shaderIrradiance.Bind();
			shaderIrradiance.Dispatch(128 / 32, 128 / 32, 6);  // �ַ���������
			shaderIrradiance.SyncWait(GL_TEXTURE_FETCH_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);  // �ȴ�ͬ��

			auto irradianceFence = Sync(0);  // ͬ�����
			irradianceFence.WaitClientSync();
			mIrradianceMap->UnbindILS(0);  // �����ն���ͼ
		}

		LOG_INFO("Ԥ���㾵��Ԥ���˻�����ͼ:{0}", hdri);
		Texture::Copy(*mapEnv, 0, *mPrefilteredMap, 0);  // ���ƻ�������

		const GLuint maxLevel = mPrefilteredMap->mLevel - 1;	// ��󼶱�
		GLuint resolution = mPrefilteredMap->mWidth / 2;		// �ֱ���
		shaderPrefilter.Bind();									// ��Ԥ���˻�����ͼ��ɫ��
		for (unsigned int level = 1; level <= maxLevel; level++, resolution /= 2) {
			float roughness = level / static_cast<float>(maxLevel);			// ����ֲڶ�
			GLuint numGroups = glm::max<GLuint>(resolution / 32, 1);		// ���������

			mPrefilteredMap->BindILS(level, 1, GL_WRITE_ONLY);				// ��Ԥ���˻�����ͼ��ͼ��洢����
			shaderPrefilter.SetUniform(0, roughness);						// ���ôֲڶȲ���
			shaderPrefilter.Dispatch(numGroups, numGroups, 6);				// �ַ���������
			shaderPrefilter.SyncWait(GL_TEXTURE_FETCH_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);  // �ȴ�ͬ��

			auto prefilterFence = Sync(level);		// ͬ�����
			prefilterFence.WaitClientSync();		// �ͻ��˵ȴ�ͬ��
			mPrefilteredMap->UnbindILS(1);			// ���Ԥ���˻�����ͼ
		}

		LOG_INFO("Ԥ���㾵�滷�� BRDF LUT:{0}", hdri);		// �����־��Ԥ���㾵�滷��BRDF LUT
		mBRDF_LUT->BindILS(0, 2, GL_WRITE_ONLY);		// �󶨻���BRDF LUT��ͼ��洢����

		if (true) {				// �󶨻���BRDF LUT��ɫ��
			shaderEnvBRDF.Bind();
			shaderEnvBRDF.Dispatch(1024 / 32, 1024 / 32, 1);	// �ַ���������
			shaderEnvBRDF.SyncWait(GL_ALL_BARRIER_BITS);		// �ȴ�ͬ��
			Sync::WaitFinish();									// �ȴ��������
			mBRDF_LUT->UnbindILS(2);							// ��󻷾�BRDF LUT
		}
	}
	void Scene01::SetMaterial(Material& pbrMaterial, int id)
	{
		LOG_TRACK;
		pbrMaterial.SetTexture(pbr_t::irradiance_map, mIrradianceMap);  // ������������ն���ͼ
		pbrMaterial.SetTexture(pbr_t::prefiltered_map, mPrefilteredMap);  // ����Ԥ���˻�����ͼ
		pbrMaterial.SetTexture(pbr_t::BRDF_LUT, mBRDF_LUT);  // ���û���BRDF LUT

		std::string path = "model\\runestone\\";
		switch (id){
		case 0: {
			LOG_INFO("�����������");
			pbrMaterial.BindUniform(pbr_u::albedo, &gInputFromUI.SphereAlbedo);  // �󶨷�����Uniform
			pbrMaterial.BindUniform(pbr_u::metalness, &gInputFromUI.SphereMetalness);  // �󶨽�����Uniform
			pbrMaterial.BindUniform(pbr_u::roughness, &gInputFromUI.SphereRoughness);  // �󶨴ֲڶ�Uniform
			pbrMaterial.BindUniform(pbr_u::ao, &gInputFromUI.SphereAO);  // �󶨻������ڱ�Uniform
			break;
		}
		case 2: {
			LOG_INFO("����ƽ�����");
			pbrMaterial.SetTexture(pbr_t::albedo, std::make_shared<Texture>("texture\\common\\checkboard.png"));  // ���÷�������ͼ
			pbrMaterial.SetUniform(pbr_u::metalness, 0.1f);  // ���ý�����Uniform
			pbrMaterial.BindUniform(pbr_u::roughness, &gInputFromUI.PlaneRoughness);  // �󶨴ֲڶ�Uniform
			pbrMaterial.SetUniform(pbr_u::uv_scale, vec2(8.0f));  // ����UV����Uniform
			break;
		}
		case 31: {
			LOG_INFO("���÷���ʯ������");
			pbrMaterial.SetTexture(pbr_t::albedo, std::make_shared<Texture>(path+"pillars_albedo.png"));  // ���÷�������ͼ
			pbrMaterial.SetTexture(pbr_t::normal, std::make_shared<Texture>(path+"pillars_normal.png"));  // ���÷�����ͼ
			pbrMaterial.SetTexture(pbr_t::metallic, std::make_shared<Texture>(path+"pillars_metallic.png"));  // ���ý�������ͼ
			pbrMaterial.SetTexture(pbr_t::roughness, std::make_shared<Texture>(path+"pillars_roughness.png"));  // ���ôֲڶ���ͼ
			break;
		}
		case 32: {	// ����ʯƽ̨����
			LOG_INFO("���÷���ʯƽ̨����");
			pbrMaterial.SetTexture(pbr_t::albedo, std::make_shared<Texture>(path+"platform_albedo.png"));  // ���÷�������ͼ
			pbrMaterial.SetTexture(pbr_t::normal, std::make_shared<Texture>(path+"platform_normal.png"));  // ���÷�����ͼ
			pbrMaterial.SetTexture(pbr_t::metallic, std::make_shared<Texture>(path+"platform_metallic.png"));  // ���ý�������ͼ
			pbrMaterial.SetTexture(pbr_t::roughness, std::make_shared<Texture>(path+"platform_roughness.png"));  // ���ôֲڶ���ͼ
			pbrMaterial.SetTexture(pbr_t::emission, std::make_shared<Texture>(path+"platform_emissive.png"));  // ���÷�����ͼ
			break;
		}
		default:
			LOG_ERROR("���ò��ʲ�������");
			break;
		}
	}
	void Scene01::SetPLBuffers()
	{
		gNx = (Window::mWidth + gTileSize - 1) / gTileSize;  // ����X�����������
		gNy = (Window::mHeight + gTileSize - 1) / gTileSize;  // ����Y�����������
		GLuint n_tiles = gNx * gNy;  // �����ܵ�������

		// �������ǵķ�Ƭǰ����Ⱦ�����ܹ���Ҫ4��SSBO����ɫ��λ�úͷ�Χ��GLSL��ɫ������ֻ���ģ�
		// ����Ӧ�ò�ʱ��CPU�˸��£�������ǽ����ֶ����ݴ洢��ָ�룬ʹ��һ���Գ־�ӳ�䡣
		// ָ��ֻ�ܻ�ȡһ�Σ������ͷţ���������д�������Զ�ˢ�µ�GPU����һ���棬����SSBO�ɼ���
		// ��ɫ�����£����ڱ���δ���޳��Ĺ�Դ����������Ӧ�ö�CPU�ɼ������ʹ�ö�̬�洢λ��

		GLbitfield cpuAccess = GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT | GL_MAP_WRITE_BIT;
		GLbitfield gpuAccess = GL_DYNAMIC_STORAGE_BIT;

		mPlColor = std::make_unique<SSBO>(0, gPlsNum * sizeof(vec4), cpuAccess);			// ��ɫSSBO
		mPlPosition = std::make_unique<SSBO>(1, gPlsNum * sizeof(vec4), cpuAccess);			// λ��SSBO
		mPlRange = std::make_unique<SSBO>(2, gPlsNum * sizeof(float), cpuAccess);			// ��ΧSSBO
		mPlIndex = std::make_unique<SSBO>(3, gPlsNum * n_tiles * sizeof(int), gpuAccess);	// ����SSBO

		// �ڷ�Ƭǰ����Ⱦ���У���Դ�޳����������ھ�̬�Ͷ�̬��Դ��������Ҫ�û�ÿ֡����SSBO��������
		// Ϊ�����������ֻ��28����̬���Դִ�й�Դ�޳�����Ϊ��Դ����������㹻�����SSBOֻ��Ҫ����һ�Ρ�

		mPlPosition->Acquire(cpuAccess);  // ��ȡλ��SSBO
		mPlColor->Acquire(cpuAccess);     // ��ȡ��ɫSSBO
		mPlRange->Acquire(cpuAccess);     // ��ȡ��ΧSSBO

		auto* const posit_ptr = reinterpret_cast<vec4* const>(mPlPosition->GetData());  // ��ȡλ������ָ��
		auto* const color_ptr = reinterpret_cast<vec4* const>(mPlColor->GetData());     // ��ȡ��ɫ����ָ��
		auto* const range_ptr = reinterpret_cast<float* const>(mPlRange->GetData());    // ��ȡ��Χ����ָ��

		for (int i = 0; i < gPlsNum; i++) {
			auto& pt = mPointLights[i].GetComponent<Transform>();  // ��ȡ���Դ��Transform���
			auto& pl = mPointLights[i].GetComponent<PointLight>();  // ��ȡ���Դ��PointLight���

			posit_ptr[i] = vec4(pt.mPosition, 1.0f);  // ����λ������
			color_ptr[i] = vec4(pl.mColor, 1.0f);     // ������ɫ����
			range_ptr[i] = pl.mRange;                 // ���·�Χ����
		}
	}
	void Scene01::UpdatePLColors()
	{
		LOG_TRACK;
		auto pColor = reinterpret_cast<vec4* const>(mPlColor->GetData());  // ��ȡ���Դ��ɫ����ָ��
		for (int i = 0; i < gPlsNum; ++i) {
			auto hue = Math::RandomGenerator<float>();  // �������ɫ��
			auto color = Math::HSV2RGB(vec3(hue, 1.0f, 1.0f));  // ��ɫ��ת��ΪRGB��ɫ
			pColor[i] = vec4(color, 1.0f);  // ������ɫ�����е�ֵ
			mPointLights[i].GetComponent<PointLight>().mColor = color;  // ���µ��Դ����е���ɫ����
		}
	}
}