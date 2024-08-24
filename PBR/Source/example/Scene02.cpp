#include <IconFont/IconsForkAwesome.h>

#include "Scene02.h"
#include "../scene/Renderer.h"
#include "../utils/Log.h"
#include "../core/Sync.h"
#include "../core/Window.h"
#include "../component/Light.h"
#include "../scene/UI.h"

using namespace core;
namespace scene {
    static const ivec2 gNumVerts = ivec2(32, 32);       // ÿ��ά���еĶ�������
    static const vec2 gClothSz = vec2(16.0f, 12.0f);  // ����/���ӵĳߴ�

    static bool  gShowGrid = true;             // �Ƿ���ʾ����
    static float gGridCellSize = 2.0f;              // ����Ԫ��С
    static vec4  gThinLineColor = vec4(0.1f, 0.1f, 0.1f, 1.0f);  // ϸ����ɫ
    static vec4  gWideLineColor = vec4(0.2f, 0.2f, 0.2f, 1.0f);  // ������ɫ
    static vec3  gDlDirection = vec3(0.0f, -1.0f, 1.0f);        // ����ⷽ��

    static float gSkyboxExposure = 1.0f;  // ��պ��ع��
    static float gSkyboxLod = 0.0f;   // ��պ�LOD��ϸ�ڼ���

    static bool  gIsRotateModel = false;  // �Ƿ���תģ��
    static bool  gIsSimulate = false;  // �Ƿ�ģ��
    static bool  gIsShowWireframe = false;  // �Ƿ���ʾ�߿�
    static vec4  gWireframeColor = vec4(1.0f);  // �߿���ɫ
    static int   gNumIndices = 0;      // ��������
    static uint  gRBuffer = 0;      // ��������
    static uint  gWBuffer = 1;      // д������

    static vec4  gAlbedo = vec4(color::black, 1.0f);  // ��������ɫ
    static float gRoughness = 1.0f;                      // �ֲڶ�
    static float gAO = 1.0f;                      // �������ڱ�
    static vec3  gSheenColor = color::blue;               // ������ɫ
    static vec3  gSubsurfColor = vec3(0.15f);               // ������ɫ
    static uvec2 gShadingModel = uvec2(3, 0);                // ��ɫģ��

	void Scene02::Init()
	{
		LOG_TRACK;
		PrecomputeIBL("texture\\HDRI\\hotel_room_4k2.hdr");		// Ԥ����ͼ����ڹ��յ�����
        // �����ɫ���Ͳ��ʵ���Դ������
        mResourceManager.Add(01, std::make_shared<Shader>("core\\infinite_grid.glsl"));
        mResourceManager.Add(02, std::make_shared<Shader>("core\\skybox.glsl"));
        mResourceManager.Add(04, std::make_shared<Shader>("scene_02\\pbr.glsl"));
        mResourceManager.Add(05, std::make_shared<Shader>("scene_02\\post_process.glsl"));
        mResourceManager.Add(12, std::make_shared<Material>(mResourceManager.Get<Shader>(02)));
        mResourceManager.Add(14, std::make_shared<Material>(mResourceManager.Get<Shader>(04)));
        mResourceManager.Add(30, std::make_shared<CShader>("scene_02\\cloth_position.glsl"));
        mResourceManager.Add(31, std::make_shared<CShader>("scene_02\\cloth_normal.glsl"));

        // ���UBO��������
        AddUBO(mResourceManager.Get<Shader>(04)->getId());
        AddUBO(mResourceManager.Get<Shader>(02)->getId());

        // �������֡�������
        AddFBO(Window::mWidth, Window::mHeight);
        AddFBO(Window::mWidth, Window::mHeight);

        // ���õ�һ��֡�������
        mFBO[0].AddColorTexture(1, true);  // ���һ����ɫ��������ʹ��MRT
        mFBO[0].AddDepStRenderBuffer(true);  // ������/ģ����Ⱦ������

        // ���õڶ���֡�������
        mFBO[1].AddColorTexture(1);  // ���һ����ɫ������

        // �����������ʵ�岢������λ�ú�ͶӰ����
        mCamera = CreateEntity("Camera", ETag::MainCamera);
        mCamera.GetComponent<Transform>().Translate(0.0f, 6.0f, 9.0f);
        mCamera.AddComponent<Camera>(View::Perspective);

        // ������պ�ʵ�岢�����������������Ͳ���
        mSkybox = CreateEntity("Skybox", ETag::Skybox);
        mSkybox.AddComponent<Mesh>(Primitive::Cube);
        auto& mat = mSkybox.AddComponent<Material>(mResourceManager.Get<Material>(12));
        mat.SetTexture(0, mMapPrefiltered);         // ������պв�������
        mat.BindUniform(0, &gSkyboxExposure);       // ����պ��ع�Ȳ���
        mat.BindUniform(1, &gSkyboxLod);            // ����պ�LOD����
        

        // ���������ʵ�岢��������ת����ɫǿ��
        mDirectLight = CreateEntity("Directional Light");
        mDirectLight.GetComponent<Transform>().Rotate(45.0f, 180.0f, 0.0f, Space::World);
        mDirectLight.AddComponent<DirectionLight>(color::white, 0.5f);

        // ��������ģ��ʵ�岢������λ�ú�����
        cloth_model = CreateEntity("Cloth Model");
        cloth_model.GetComponent<Transform>().Translate(world::up * 4.0f);
        cloth_model.GetComponent<Transform>().Scale(2.0f);

        // ���ز���ģ�Ͳ����ò���
        auto& model = cloth_model.AddComponent<Model>("model\\cloth\\cloth.obj", Quality::Auto);
        SetMaterial(model.SetMaterial("cloth", mResourceManager.Get<Material>(14)), true, false);   // ���ò��ϲ���
        SetMaterial(model.SetMaterial("outside", mResourceManager.Get<Material>(14)), false, false);  // �����ⲿ����
        

        // ���ö�̬����ģ����ػ������Ͳ���
        SetBuffers();
        auto cloth_mat = mResourceManager.Get<Material>(14);
        SetMaterial(*cloth_mat, true, true);  // ���ò��ϲ���
        cloth_mat->SetUniform(1000U, world::identity);  // ���ò��ϲ��ʵĸ��Ӳ���
        cloth_mat->BindUniform(pbr_u::shading_model, &gShadingModel);  // ����ɫģ�Ͳ���

        // ������Ⱦ��״̬
        Renderer::PrimitiveRestart(true);  // ����ͼԪ����
        Renderer::SetMSAA(true);              // �������ز��������
        Renderer::SetDepthTest(true);         // ������Ȳ���
        Renderer::SetAlphaBlend(true);        // ����Alpha���
	}
    void Scene02::OnSceneRender()
    {
        auto& mainCamera = mCamera.GetComponent<Camera>();
        mainCamera.Update();  // �������������״̬

        // ����UBO����
        mUBO[0].SetUniform(0, GetValPtr(mainCamera.mpTransform->mPosition));    // ���������λ�ò���
        mUBO[0].SetUniform(1, GetValPtr(mainCamera.mpTransform->mForward));     // ���������ǰ����������
        mUBO[0].SetUniform(2, GetValPtr(mainCamera.GetViewMatrix()));           // �����������ͼ�������
        mUBO[0].SetUniform(3, GetValPtr(mainCamera.GetProjectionMatrix()));     // ���������ͶӰ�������

        // ���ù�ԴUBO����
        auto& dl = mDirectLight.GetComponent<DirectionLight>();
        vec3 direction = -glm::normalize(gDlDirection);
        mUBO[1].SetUniform(0, GetValPtr(dl.mColor));      // ���ù�Դ��ɫ����
        mUBO[1].SetUniform(1, GetValPtr(direction));    // ���ù�Դ�������
        mUBO[1].SetUniform(2, GetValPtr(dl.mIntensity));  // ���ù�Դǿ�Ȳ���

        //FBO& framebuffer_0 = mFBO[0];
        //FBO& framebuffer_1 = mFBO[1];

        // ------------------------------ ģ������Ⱦ���� ------------------------------

        mFBO[0].Clear();  // ���֡�������0
        mFBO[0].Bind();   // ��֡�������0

        // ���ڲ�������Alpha��ϣ���Ҫ����Ⱦ��պ�
        Renderer::SetFaceCulling(true);     // �������޳�
        Renderer::Submit(mSkybox.mId);      // �ύ��պ���Ⱦ
        Renderer::Render();                 // ִ����Ⱦ����
        Renderer::SetFaceCulling(false);    // �ر����޳�

        if (gIsSimulate) {
            // ���²��϶���λ��
            auto simulation = mResourceManager.Get<CShader>(30);
            simulation->Bind();          // �󶨼�����ɫ��

            for (int i = 0; i < 512; ++i) {
                simulation->Dispatch(gNumVerts.x / 32, gNumVerts.y / 32);
                simulation->SyncWait();             // �ȴ�������ɫ�����
                std::swap(gRBuffer, gWBuffer);      // ������д����������

                mPos[gRBuffer]->Reset(0);  // ���ò���λ�û�����
                mPos[gWBuffer]->Reset(1);  // ���ò���λ�û�����
                mVel[gRBuffer]->Reset(2);  // ���ò����ٶȻ�����
                mVel[gWBuffer]->Reset(3);  // ���ò����ٶȻ�����
            }

            // ���²��϶��㷨��
            auto normal_cs = mResourceManager.Get<CShader>(31);
            normal_cs->Bind();  
            normal_cs->Dispatch(gNumVerts.x / 32, gNumVerts.y / 32);  
            normal_cs->SyncWait();

            mResourceManager.Get<Material>(14)->Bind();  // �󶨲��ϲ���
            mVAO->Draw(GL_TRIANGLE_STRIP, gNumIndices);  // ���Ʋ���ģ��
        }
        else {
            if (gIsRotateModel) {
                cloth_model.GetComponent<Transform>().Rotate(world::up, 0.2f, Space::Local);  // ��תģ��
            }
            Renderer::Submit(cloth_model.mId);  // �ύ����ģ����Ⱦ
            Renderer::Render();                 // ִ����Ⱦ����
        }

        if (gShowGrid) {
            auto grid_shader = mResourceManager.Get<Shader>(01);
            grid_shader->Bind();  // ��������ɫ��
            grid_shader->SetUniform(0, gGridCellSize);      // ��������Ԫ��С����
            grid_shader->SetUniform(1, gThinLineColor);     // ����ϸ����ɫ����
            grid_shader->SetUniform(2, gWideLineColor);     // ���ô�����ɫ����
            Mesh::DrawGrid();   // ��������
        }

        mFBO[0].UnBind();  // ���֡�������0

        // ------------------------------ MSAA�������� ------------------------------
        mFBO[1].Clear(); 
        FBO::CopyColor(mFBO[0], 0, mFBO[1], 0);  

        // ------------------------------ ������� ------------------------------

        mFBO[1].GetColorTexture(0).Bind(0);         // ��֡�������1����ɫ����
        auto shaderPostprocess = mResourceManager.Get<Shader>(05);
        shaderPostprocess->Bind();                 // �󶨺�����ɫ��
        shaderPostprocess->SetUniform(0, 3);       // ���ú�����ɫ������

        Renderer::Clear();  // �����Ⱦ״̬
        Mesh::DrawQuad();   // �����ı���
        shaderPostprocess->UnBind();  // ��������ɫ��
    }
    void Scene02::OnImGuiRender()
    {
        using namespace ImGui;

        const ImGuiColorEditFlags color_flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha;
        const ImVec2 rainbow_offset = ImVec2(5.0f, 105.0f);  // �ʺ���ƫ����
        const ImVec4 tabColorOff = ImVec4(0.0f, 0.3f, 0.6f, 1.0f);  // ѡ�δѡ����ɫ
        const ImVec4 tab_color_on = ImVec4(0.0f, 0.4f, 0.8f, 1.0f);  // ѡ�ѡ����ɫ
        static bool simulation_clearcoat = false;  // �Ƿ�ģ��͸������
        static float cloth_alpha = 1.0f;           // ����͸����
        
        if (UI::NewInspector()) {
            Indent(5.0f);
            Text(UI::ToU8(" ���������"));  // ���������
            DragFloat3("###", GetValPtr(gDlDirection), 0.01f, -1.0f, 1.0f, "%.3f");  // ��ק�༭���������
            UI::DrawRainbowBar(rainbow_offset, 2.0f);  // ���Ʋʺ���
            Spacing();

            // ������������޵Ĳ��ʣ���albedo����Ϊ��ɫ����ʹ���������͵�sheen��ɫ��
            // �����޲���ţ�в���ʹ��albedo��Ϊ������ɫ��Ȼ������sheen��ɫΪ��ͬɫ���Ľ�����ɫ��
            // ����albedo���������ɫ�Ļ��ɫ����ģ����ά��ǰ��ͺ���ɢ�䣻
            // ����Ƥ���˿�����ڼ���û�б������ʹα���ɢ�䣬ͨ�����ʺ�ʹ�ñ�׼�ľ����ʵ������ĸ�������ģʽ����ɫģ�͡�
            PushItemWidth(130.0f);
            SliderFloat(UI::ToU8("��պ��ع��"), &gSkyboxExposure, 0.5f, 4.0f);  // ������պ��ع��
            SliderFloat(UI::ToU8("��պ�LOD"), &gSkyboxLod, 0.0f, 7.0f);            // ������պ�LOD
            PopItemWidth();
            Separator();

            BeginTabBar("InspectorTab", ImGuiTabBarFlags_None);

            if (BeginTabItem(UI::ToU8("��̬ģ��"))) {
                PushItemWidth(130.0f);
                Checkbox(UI::ToU8("��ʾ�߿�"), &gIsShowWireframe); SameLine();
                ColorEdit3(UI::ToU8("�߿���ɫ"), GetValPtr(gWireframeColor), color_flags);  // �༭�߿���ɫ
                Checkbox(UI::ToU8("�Զ���ת"), &gIsRotateModel);                        // �Զ���ת
                SliderFloat(UI::ToU8("�ֲڶ�"), &gRoughness, 0.045f, 1.0f);              // �����ֲڶ�
                SliderFloat(UI::ToU8("�����ڵ�"), &gAO, 0.05f, 1.0f);              // ���������ڵ�
                if (SliderFloat(UI::ToU8("͸����"), &cloth_alpha, 0.5f, 1.0f)) {     // ����͸����
                    gAlbedo.a = cloth_alpha * 0.1f + 0.9f;
                }
                ColorEdit3("Albedo", GetValPtr(gAlbedo), color_flags);    SameLine();
                ColorEdit3("Sheen", GetValPtr(gSheenColor), color_flags);  SameLine();
                ColorEdit3("Subsurface", GetValPtr(gSubsurfColor), color_flags);
                PopItemWidth();
                EndTabItem();
            }

            if (gIsSimulate = BeginTabItem(UI::ToU8("����")); gIsSimulate) {
                Checkbox(UI::ToU8("��ʾ�߿�"), &gIsShowWireframe); SameLine();
                ColorEdit3(UI::ToU8("�߿���ɫ"), GetValPtr(gWireframeColor), color_flags);  // �༭�߿���ɫ
                Checkbox(UI::ToU8("Ӧ��͸������"), &simulation_clearcoat);              // Ӧ��͸������
                gShadingModel = simulation_clearcoat ? uvec2(3, 1) : uvec2(3, 0);  // ������ɫģ��

                if (ArrowButton("##1", ImGuiDir_Left)) {   // �����ͷ��ť
                    mResourceManager.Get<CShader>(30)->SetUniform(1, 20.0f * world::left);  // ���÷���
                }
                if (SameLine(); ArrowButton("##2", ImGuiDir_Right)) {  // ���Ҽ�ͷ��ť
                    mResourceManager.Get<CShader>(30)->SetUniform(1, 20.0f * world::right);  // ���÷���
                }
                if (SameLine(); ArrowButton("##3", ImGuiDir_Up)) {  // ���ϼ�ͷ��ť
                    mResourceManager.Get<CShader>(30)->SetUniform(1, 20.0f * world::up);  // ���÷���
                }
                if (SameLine(); ArrowButton("##4", ImGuiDir_Down)) {  // ���¼�ͷ��ť
                    mResourceManager.Get<CShader>(30)->SetUniform(1, world::zero);  // ���÷���
                }

                SameLine(); Text(UI::ToU8("����"));  // �����ı�
                Spacing();
                if (Button(UI::ToU8("��������"), ImVec2(150.0f, 0.0f))) {  // ��������ť
                    mPos[0]->SetData(&mInitPos[0]);  // ���ò���λ������
                    mVel[0]->SetData(&mInitVel[0]);  // ���ò����ٶ�����
                    mPos[1]->SetData(NULL);         // ��ղ���λ������
                    mVel[1]->SetData(NULL);         // ��ղ����ٶ�����
                }
                EndTabItem();
            }

            PushStyleColor(ImGuiCol_Tab, tabColorOff);
            PushStyleColor(ImGuiCol_TabHovered, tab_color_on);
            PushStyleColor(ImGuiCol_TabActive, tab_color_on);

            if (BeginTabItem(ICON_FK_TH_LARGE)) {
                PushItemWidth(130.0f);
                Checkbox(UI::ToU8("��ʾ��������"), &gShowGrid);         // ��ʾ��������
                SliderFloat(UI::ToU8("����Ԫ��С"), &gGridCellSize, 0.25f, 8.0f);  // ��������Ԫ��С
                PopItemWidth();
                ColorEdit4(UI::ToU8("��Ҫ����ɫ"), GetValPtr(gThinLineColor), color_flags);  // �༭��Ҫ����ɫ
                ColorEdit4(UI::ToU8("��Ҫ����ɫ"), GetValPtr(gWideLineColor), color_flags);  // �༭��Ҫ����ɫ
                EndTabItem();
            }

            PopStyleColor(3);
            EndTabBar();

            Unindent(5.0f);
            UI::EndInspector();  // �������������
        }

    }
	void Scene02::PrecomputeIBL(const std::string& hdri)
	{
        Renderer::SetSeamlessCubemap(true);
        Renderer::SetDepthTest(false);
        Renderer::SetFaceCulling(true);

        auto shaderIrradiance = CShader("core\\irradiance_map.glsl");       // �������ն���ͼ��ɫ��
        auto shaderPrefilter = CShader("core\\prefilter_envmap.glsl");      // ����Ԥ���˻�����ͼ��ɫ��
        auto shaderEnvBRDF = CShader("core\\environment_BRDF.glsl");        // ��������BRDF��ͼ��ɫ��

        auto mapEnv = std::make_shared<Texture>(hdri, 2048, 0);  // ����������ͼ
        mapEnv->Bind(0);  // �󶨵�����Ԫ0

        mMapIrradiance = std::make_shared<Texture>(GL_TEXTURE_CUBE_MAP, 128, 128, 6, GL_RGBA16F, 1);        // �������ն���ͼ
        mMapPrefiltered = std::make_shared<Texture>(GL_TEXTURE_CUBE_MAP, 2048, 2048, 6, GL_RGBA16F, 8);     // ����Ԥ���˻�����ͼ
        mMapBRDF_LUT = std::make_shared<Texture>(GL_TEXTURE_2D, 1024, 1024, 1, GL_RGBA16F, 1);              // ��������BRDF��ͼ

        LOG_INFO("Ԥ������������ն���ͼ: {0}", hdri); 
        mMapIrradiance->BindILS(0, 0, GL_WRITE_ONLY);  // �󶨵�ͼ�����/�洢����Ԫ0

        shaderIrradiance.Bind();
        shaderIrradiance.Dispatch(128 / 32, 128 / 32, 6);  // �ַ�������ն���ͼ
        shaderIrradiance.SyncWait(GL_TEXTURE_FETCH_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);  // ͬ���ȴ��������

        auto irradianceFence = Sync(0);  // ����ͬ������
        irradianceFence.WaitClientSync();  // �ȴ�ͬ������ͻ������
        mMapIrradiance->UnbindILS(0);  // �����ն���ͼ
        

        LOG_INFO("Ԥ���㾵��Ԥ���˻�����ͼ: {0}", hdri);
        Texture::Copy(*mapEnv, 0, *mMapPrefiltered, 0);  // ���ƻ�������

        const GLuint max_level = mMapPrefiltered->mLevel - 1;  // ��󼶱�
        GLuint resolution = mMapPrefiltered->mWidth / 2;           // �ֱ���
        shaderPrefilter.Bind();  // ��Ԥ������ɫ��

        for (unsigned int level = 1; level <= max_level; level++, resolution /= 2) {
            float roughness = level / static_cast<float>(max_level);  // �ֲڶ�
            GLuint n_groups = glm::max<GLuint>(resolution / 32, 1);   // ��������

            mMapPrefiltered->BindILS(level, 1, GL_WRITE_ONLY);  // ��Ԥ���˻�����ͼ��ͼ�����/�洢����Ԫ1
            shaderPrefilter.SetUniform(0, roughness);          // ���ôֲڶȲ���
            shaderPrefilter.Dispatch(n_groups, n_groups, 6);   // �ַ�����Ԥ���˻�����ͼ
            shaderPrefilter.SyncWait(GL_TEXTURE_FETCH_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);  // ͬ���ȴ��������

            auto prefilter_fence = Sync(level);  // ����ͬ������
            prefilter_fence.WaitClientSync();    // �ȴ�ͬ������ͻ������
            mMapPrefiltered->UnbindILS(1);       // ���Ԥ���˻�����ͼ
        }

        LOG_INFO("Ԥ���㾵�滷��BRDF��ͼ: {0}", hdri);
        mMapBRDF_LUT->BindILS(0, 2, GL_WRITE_ONLY);  // �󶨻���BRDF��ͼ��ͼ�����/�洢����Ԫ2

        shaderEnvBRDF.Bind();
        shaderEnvBRDF.Dispatch(1024 / 32, 1024 / 32, 1); 
        shaderEnvBRDF.SyncWait(GL_ALL_BARRIER_BITS);     
        Sync::WaitFinish();                               
        mMapBRDF_LUT->UnbindILS(2);                     
	}
    void Scene02::SetBuffers()
    {
        const GLuint numRows = gNumVerts.y;  // ��������
        const GLuint numCols = gNumVerts.x;  // ��������
        const GLuint n = numCols * numRows;  // ���񶥵���
        const float dx = gClothSz.x / (numCols - 1);  // ���񶥵��ࣨx����
        const float dy = gClothSz.y / (numRows - 1);  // ���񶥵��ࣨy����
        const float du = 1.0f / (numCols - 1);        // ��������������u����
        const float dv = 1.0f / (numRows - 1);        // ��������������v����

        mInitPos.reserve(n);   
        mInitVel.reserve(n);    
        mTextureCoord.reserve(n);  

        Transform texture = Transform();          
        texture.Rotate(world::right, -90.0f, Space::Local);                 // �Ʊ�������ϵ��x����ת-90��
        texture.Translate(-gClothSz.x * 0.5f, 4.0f, gClothSz.y * 0.5f);     // ƽ�Ʊ任

        for (int row = 0; row < numRows; row++) {
            for (int col = 0; col < numCols; col++) {
                vec4 position = texture.mTransform * vec4(dx * col, dy * row, 0.0f, 1.0f);  // ����λ��
                vec4 velocity = vec4(0.0f);             // ��ʼ�ٶ�Ϊ0
                vec2 uv = vec2(du * col, dv * row);     // ������������

                mInitPos.push_back(position);   
                mInitVel.push_back(velocity);   
                mTextureCoord.push_back(uv);  
            }
        }

        // ����������ÿ���������й���һ�������δ�
        // ע�⣺�����δ����淽���ɵ�һ�������εĶ���˳�������
        // ÿ�����������ε��淽�򽫱���ת�Ա��ָ�˳������OpenGL�Զ�����

        for (int row = 0; row < numRows - 1; row++) {
            mIndices.push_back(0xFFFFFF);  // ��������
            for (int col = 0; col < numCols; col++) {
                mIndices.push_back(row * numCols + col + numCols);
                mIndices.push_back(row * numCols + col);
            }
        }

        gNumIndices = static_cast<int>(mIndices.size());  // ����������Ŀ

        LOG_ASSERT(sizeof(vec4) == 4 * sizeof(GLfloat), "GL��vec4�ĸ�����δ���ܴ����");
        LOG_ASSERT(sizeof(vec2) == 2 * sizeof(GLfloat), "GL��vec2�ĸ�����δ���ܴ����");

        mVAO = std::make_unique<VAO>();  
        mVBO = std::make_unique<VBO>(n * sizeof(vec2), &mTextureCoord[0]);              // ����VBO���������꣩
        mIBO = std::make_unique<IBO>(gNumIndices * sizeof(GLuint), &mIndices[0]);       // ����IBO���������壩

        mPos[0] = std::make_unique<SSBO>(0, n * sizeof(vec4), GL_DYNAMIC_STORAGE_BIT);  // ����SSBO��λ�ã�
        mPos[1] = std::make_unique<SSBO>(1, n * sizeof(vec4), GL_DYNAMIC_STORAGE_BIT);  // ����SSBO��λ�ñ��ݣ�
        mVel[0] = std::make_unique<SSBO>(2, n * sizeof(vec4), GL_DYNAMIC_STORAGE_BIT);  // ����SSBO���ٶȣ�
        mVel[1] = std::make_unique<SSBO>(3, n * sizeof(vec4), GL_DYNAMIC_STORAGE_BIT);  // ����SSBO���ٶȱ��ݣ�
        mNormal = std::make_unique<SSBO>(4, n * sizeof(vec4), GL_DYNAMIC_STORAGE_BIT);  // ����SSBO�����ߣ�

        mPos[0]->SetData(&mInitPos[0]);  // ����λ������
        mVel[0]->SetData(&mInitVel[0]);  // �����ٶ�����

        mVAO->SetVBO(mPos[0]->mId, 0, 0, 3, sizeof(vec4), GL_FLOAT);  // ����VAO��λ��VBO
        mVAO->SetVBO(mNormal->mId, 1, 0, 3, sizeof(vec4), GL_FLOAT);  // ����VAO�ķ���VBO
        mVAO->SetVBO(mVBO->mId, 2, 0, 2, sizeof(vec2), GL_FLOAT);     // ����VAO����������VBO
        mVAO->SetIBO(mIBO->mId);  // ����VAO����������

        auto simulationCS = mResourceManager.Get<CShader>(30);      // ��ȡ������ɫ��
        simulationCS->SetUniform(0, vec3(0.0f, -10.0f, 0.0f));      // ��������
        simulationCS->SetUniform(1, vec3(0.0f));                    // ���÷���
        simulationCS->SetUniform(2, dx);                            // ����dx
        simulationCS->SetUniform(3, dy);                            // ����dy
        simulationCS->SetUniform(4, sqrtf(dx * dx + dy * dy));      // ����б�Խ��߳���
    }

    void Scene02::SetMaterial(Material& pbrMat, bool cloth, bool textured)
    {
        pbrMat.SetTexture(pbr_t::irradiance_map, mMapIrradiance);       // ����PBR���ʵ���������ն���ͼ
        pbrMat.SetTexture(pbr_t::prefiltered_map, mMapPrefiltered);     // ����PBR���ʵ�Ԥ���˻�����ͼ
        pbrMat.SetTexture(pbr_t::BRDF_LUT, mMapBRDF_LUT);               // ����PBR���ʵĻ���BRDF��ͼ

        pbrMat.BindUniform(0, &gIsShowWireframe);       // ��PBR���ʵ��߿���ʾ
        pbrMat.BindUniform(1, &gWireframeColor);        // ��PBR���ʵ��߿���ɫ
        pbrMat.SetUniform(2, 0.05f);                    // ����PBR���ʵĴֲڶ�
        pbrMat.BindUniform(3, &gSkyboxExposure);        // ��PBR���ʵ���պ��ع��

        if (cloth) {
            pbrMat.SetUniform(pbr_u::shading_model, uvec2(3, 0));  // ����PBR���ʵ���ɫģ��Ϊ����
            pbrMat.SetUniform(pbr_u::clearcoat, 1.0f);             // ����PBR���ʵ�͸����
            pbrMat.SetUniform(pbr_u::uv_scale, vec2(4.0f, 4.0f));  // ����PBR���ʵ�UV����
            std::string texPath = "texture\\fabric\\";              // ����·��

            if (textured) {
                pbrMat.SetTexture(pbr_t::albedo, std::make_shared<Texture>(texPath + "albedo.jpg"));        // ����PBR���ʵĻ�ɫ��ͼ
                pbrMat.SetTexture(pbr_t::normal, std::make_shared<Texture>(texPath + "normal.jpg"));        // ����PBR���ʵķ�����ͼ
                pbrMat.SetTexture(pbr_t::roughness, std::make_shared<Texture>(texPath + "roughness.jpg"));  // ����PBR���ʵĴֲڶ���ͼ
                pbrMat.SetTexture(pbr_t::ao, std::make_shared<Texture>(texPath + "ao.jpg"));                // ����PBR���ʵĻ������ڱ���ͼ
                pbrMat.SetUniform(pbr_u::sheen_color, color::white);    // ����PBR���ʵĹ�����ɫ
                pbrMat.SetUniform(pbr_u::subsurf_color, color::black);  // ����PBR���ʵĴα�����ɫ
            }
            else {
                pbrMat.BindUniform(pbr_u::albedo, &gAlbedo);            // ��PBR���ʵĻ�ɫ
                pbrMat.SetTexture(pbr_t::normal, std::make_shared<Texture>(texPath + "normal.jpg"));  // ����PBR���ʵķ�����ͼ
                pbrMat.BindUniform(pbr_u::roughness, &gRoughness);          // ��PBR���ʵĴֲڶ�
                pbrMat.BindUniform(pbr_u::ao, &gAO);                        // ��PBR���ʵĻ������ڱ�
                pbrMat.BindUniform(pbr_u::sheen_color, &gSheenColor);       // ��PBR���ʵĹ�����ɫ
                pbrMat.BindUniform(pbr_u::subsurf_color, &gSubsurfColor);   // ��PBR���ʵĴα�����ɫ
            }
        }
        else {
            pbrMat.SetUniform(pbr_u::shading_model, uvec2(1, 0));  // ����PBR���ʵ���ɫģ��Ϊ����
            pbrMat.SetUniform(pbr_u::metalness, 1.0f);             // ����PBR���ʵĽ�����
            pbrMat.SetUniform(pbr_u::roughness, 0.8f);             // ����PBR���ʵĴֲڶ�
            pbrMat.SetUniform(pbr_u::ao, 1.0f);                     // ����PBR���ʵĻ������ڱ�
        }
    }
}

