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
    static const ivec2 gNumVerts = ivec2(32, 32);       // 每个维度中的顶点数量
    static const vec2 gClothSz = vec2(16.0f, 12.0f);  // 布料/格子的尺寸

    static bool  gShowGrid = true;             // 是否显示网格
    static float gGridCellSize = 2.0f;              // 网格单元大小
    static vec4  gThinLineColor = vec4(0.1f, 0.1f, 0.1f, 1.0f);  // 细线颜色
    static vec4  gWideLineColor = vec4(0.2f, 0.2f, 0.2f, 1.0f);  // 粗线颜色
    static vec3  gDlDirection = vec3(0.0f, -1.0f, 1.0f);        // 方向光方向

    static float gSkyboxExposure = 1.0f;  // 天空盒曝光度
    static float gSkyboxLod = 0.0f;   // 天空盒LOD（细节级别）

    static bool  gIsRotateModel = false;  // 是否旋转模型
    static bool  gIsSimulate = false;  // 是否模拟
    static bool  gIsShowWireframe = false;  // 是否显示线框
    static vec4  gWireframeColor = vec4(1.0f);  // 线框颜色
    static int   gNumIndices = 0;      // 索引数量
    static uint  gRBuffer = 0;      // 读缓冲区
    static uint  gWBuffer = 1;      // 写缓冲区

    static vec4  gAlbedo = vec4(color::black, 1.0f);  // 反射率颜色
    static float gRoughness = 1.0f;                      // 粗糙度
    static float gAO = 1.0f;                      // 环境光遮蔽
    static vec3  gSheenColor = color::blue;               // 闪光颜色
    static vec3  gSubsurfColor = vec3(0.15f);               // 表面颜色
    static uvec2 gShadingModel = uvec2(3, 0);                // 着色模型

	void Scene02::Init()
	{
		LOG_TRACK;
		PrecomputeIBL("texture\\HDRI\\hotel_room_4k2.hdr");		// 预计算图像基于光照的数据
        // 添加着色器和材质到资源管理器
        mResourceManager.Add(01, std::make_shared<Shader>("core\\infinite_grid.glsl"));
        mResourceManager.Add(02, std::make_shared<Shader>("core\\skybox.glsl"));
        mResourceManager.Add(04, std::make_shared<Shader>("scene_02\\pbr.glsl"));
        mResourceManager.Add(05, std::make_shared<Shader>("scene_02\\post_process.glsl"));
        mResourceManager.Add(12, std::make_shared<Material>(mResourceManager.Get<Shader>(02)));
        mResourceManager.Add(14, std::make_shared<Material>(mResourceManager.Get<Shader>(04)));
        mResourceManager.Add(30, std::make_shared<CShader>("scene_02\\cloth_position.glsl"));
        mResourceManager.Add(31, std::make_shared<CShader>("scene_02\\cloth_normal.glsl"));

        // 添加UBO到场景中
        AddUBO(mResourceManager.Get<Shader>(04)->getId());
        AddUBO(mResourceManager.Get<Shader>(02)->getId());

        // 添加两个帧缓冲对象
        AddFBO(Window::mWidth, Window::mHeight);
        AddFBO(Window::mWidth, Window::mHeight);

        // 配置第一个帧缓冲对象
        mFBO[0].AddColorTexture(1, true);  // 添加一个彩色纹理附件，使用MRT
        mFBO[0].AddDepStRenderBuffer(true);  // 添加深度/模板渲染缓冲区

        // 配置第二个帧缓冲对象
        mFBO[1].AddColorTexture(1);  // 添加一个彩色纹理附件

        // 创建主摄像机实体并设置其位置和投影类型
        mCamera = CreateEntity("Camera", ETag::MainCamera);
        mCamera.GetComponent<Transform>().Translate(0.0f, 6.0f, 9.0f);
        mCamera.AddComponent<Camera>(View::Perspective);

        // 创建天空盒实体并添加立方体网格组件和材质
        mSkybox = CreateEntity("Skybox", ETag::Skybox);
        mSkybox.AddComponent<Mesh>(Primitive::Cube);
        auto& mat = mSkybox.AddComponent<Material>(mResourceManager.Get<Material>(12));
        mat.SetTexture(0, mMapPrefiltered);         // 设置天空盒材质纹理
        mat.BindUniform(0, &gSkyboxExposure);       // 绑定天空盒曝光度参数
        mat.BindUniform(1, &gSkyboxLod);            // 绑定天空盒LOD参数
        

        // 创建方向光实体并设置其旋转和颜色强度
        mDirectLight = CreateEntity("Directional Light");
        mDirectLight.GetComponent<Transform>().Rotate(45.0f, 180.0f, 0.0f, Space::World);
        mDirectLight.AddComponent<DirectionLight>(color::white, 0.5f);

        // 创建布料模型实体并设置其位置和缩放
        cloth_model = CreateEntity("Cloth Model");
        cloth_model.GetComponent<Transform>().Translate(world::up * 4.0f);
        cloth_model.GetComponent<Transform>().Scale(2.0f);

        // 加载布料模型并设置材质
        auto& model = cloth_model.AddComponent<Model>("model\\cloth\\cloth.obj", Quality::Auto);
        SetMaterial(model.SetMaterial("cloth", mResourceManager.Get<Material>(14)), true, false);   // 设置布料材质
        SetMaterial(model.SetMaterial("outside", mResourceManager.Get<Material>(14)), false, false);  // 设置外部材质
        

        // 设置动态布料模拟相关缓冲区和材质
        SetBuffers();
        auto cloth_mat = mResourceManager.Get<Material>(14);
        SetMaterial(*cloth_mat, true, true);  // 设置布料材质
        cloth_mat->SetUniform(1000U, world::identity);  // 设置布料材质的附加参数
        cloth_mat->BindUniform(pbr_u::shading_model, &gShadingModel);  // 绑定着色模型参数

        // 配置渲染器状态
        Renderer::PrimitiveRestart(true);  // 允许图元重启
        Renderer::SetMSAA(true);              // 开启多重采样抗锯齿
        Renderer::SetDepthTest(true);         // 开启深度测试
        Renderer::SetAlphaBlend(true);        // 开启Alpha混合
	}
    void Scene02::OnSceneRender()
    {
        auto& mainCamera = mCamera.GetComponent<Camera>();
        mainCamera.Update();  // 更新主摄像机的状态

        // 设置UBO参数
        mUBO[0].SetUniform(0, GetValPtr(mainCamera.mpTransform->mPosition));    // 设置摄像机位置参数
        mUBO[0].SetUniform(1, GetValPtr(mainCamera.mpTransform->mForward));     // 设置摄像机前向向量参数
        mUBO[0].SetUniform(2, GetValPtr(mainCamera.GetViewMatrix()));           // 设置摄像机视图矩阵参数
        mUBO[0].SetUniform(3, GetValPtr(mainCamera.GetProjectionMatrix()));     // 设置摄像机投影矩阵参数

        // 设置光源UBO参数
        auto& dl = mDirectLight.GetComponent<DirectionLight>();
        vec3 direction = -glm::normalize(gDlDirection);
        mUBO[1].SetUniform(0, GetValPtr(dl.mColor));      // 设置光源颜色参数
        mUBO[1].SetUniform(1, GetValPtr(direction));    // 设置光源方向参数
        mUBO[1].SetUniform(2, GetValPtr(dl.mIntensity));  // 设置光源强度参数

        //FBO& framebuffer_0 = mFBO[0];
        //FBO& framebuffer_1 = mFBO[1];

        // ------------------------------ 模拟与渲染过程 ------------------------------

        mFBO[0].Clear();  // 清除帧缓冲对象0
        mFBO[0].Bind();   // 绑定帧缓冲对象0

        // 由于布料依赖Alpha混合，需要先渲染天空盒
        Renderer::SetFaceCulling(true);     // 开启面剔除
        Renderer::Submit(mSkybox.mId);      // 提交天空盒渲染
        Renderer::Render();                 // 执行渲染命令
        Renderer::SetFaceCulling(false);    // 关闭面剔除

        if (gIsSimulate) {
            // 更新布料顶点位置
            auto simulation = mResourceManager.Get<CShader>(30);
            simulation->Bind();          // 绑定计算着色器

            for (int i = 0; i < 512; ++i) {
                simulation->Dispatch(gNumVerts.x / 32, gNumVerts.y / 32);
                simulation->SyncWait();             // 等待计算着色器完成
                std::swap(gRBuffer, gWBuffer);      // 交换读写缓冲区索引

                mPos[gRBuffer]->Reset(0);  // 重置布料位置缓冲区
                mPos[gWBuffer]->Reset(1);  // 重置布料位置缓冲区
                mVel[gRBuffer]->Reset(2);  // 重置布料速度缓冲区
                mVel[gWBuffer]->Reset(3);  // 重置布料速度缓冲区
            }

            // 更新布料顶点法线
            auto normal_cs = mResourceManager.Get<CShader>(31);
            normal_cs->Bind();  
            normal_cs->Dispatch(gNumVerts.x / 32, gNumVerts.y / 32);  
            normal_cs->SyncWait();

            mResourceManager.Get<Material>(14)->Bind();  // 绑定布料材质
            mVAO->Draw(GL_TRIANGLE_STRIP, gNumIndices);  // 绘制布料模型
        }
        else {
            if (gIsRotateModel) {
                cloth_model.GetComponent<Transform>().Rotate(world::up, 0.2f, Space::Local);  // 旋转模型
            }
            Renderer::Submit(cloth_model.mId);  // 提交布料模型渲染
            Renderer::Render();                 // 执行渲染命令
        }

        if (gShowGrid) {
            auto grid_shader = mResourceManager.Get<Shader>(01);
            grid_shader->Bind();  // 绑定网格着色器
            grid_shader->SetUniform(0, gGridCellSize);      // 设置网格单元大小参数
            grid_shader->SetUniform(1, gThinLineColor);     // 设置细线颜色参数
            grid_shader->SetUniform(2, gWideLineColor);     // 设置粗线颜色参数
            Mesh::DrawGrid();   // 绘制网格
        }

        mFBO[0].UnBind();  // 解绑帧缓冲对象0

        // ------------------------------ MSAA解析过程 ------------------------------
        mFBO[1].Clear(); 
        FBO::CopyColor(mFBO[0], 0, mFBO[1], 0);  

        // ------------------------------ 后处理过程 ------------------------------

        mFBO[1].GetColorTexture(0).Bind(0);         // 绑定帧缓冲对象1的颜色纹理
        auto shaderPostprocess = mResourceManager.Get<Shader>(05);
        shaderPostprocess->Bind();                 // 绑定后处理着色器
        shaderPostprocess->SetUniform(0, 3);       // 设置后处理着色器参数

        Renderer::Clear();  // 清除渲染状态
        Mesh::DrawQuad();   // 绘制四边形
        shaderPostprocess->UnBind();  // 解绑后处理着色器
    }
    void Scene02::OnImGuiRender()
    {
        using namespace ImGui;

        const ImGuiColorEditFlags color_flags = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoAlpha;
        const ImVec2 rainbow_offset = ImVec2(5.0f, 105.0f);  // 彩虹条偏移量
        const ImVec4 tabColorOff = ImVec4(0.0f, 0.3f, 0.6f, 1.0f);  // 选项卡未选中颜色
        const ImVec4 tab_color_on = ImVec4(0.0f, 0.4f, 0.8f, 1.0f);  // 选项卡选中颜色
        static bool simulation_clearcoat = false;  // 是否模拟透明清漆
        static float cloth_alpha = 1.0f;           // 布料透明度
        
        if (UI::NewInspector()) {
            Indent(5.0f);
            Text(UI::ToU8(" 方向光向量"));  // 方向光向量
            DragFloat3("###", GetValPtr(gDlDirection), 0.01f, -1.0f, 1.0f, "%.3f");  // 拖拽编辑方向光向量
            UI::DrawRainbowBar(rainbow_offset, 2.0f);  // 绘制彩虹条
            Spacing();

            // 创建类似天鹅绒的材质，将albedo设置为黑色，并使用明亮饱和的sheen颜色；
            // 对于棉布或牛仔布，使用albedo作为基础颜色，然后设置sheen颜色为相同色调的较亮颜色，
            // 或者albedo和入射光颜色的混合色，以模拟纤维的前向和后向散射；
            // 对于皮革和丝绸，由于几乎没有表面光泽和次表面散射，通常更适合使用标准的具有适当调整的各向异性模式的着色模型。
            PushItemWidth(130.0f);
            SliderFloat(UI::ToU8("天空盒曝光度"), &gSkyboxExposure, 0.5f, 4.0f);  // 调整天空盒曝光度
            SliderFloat(UI::ToU8("天空盒LOD"), &gSkyboxLod, 0.0f, 7.0f);            // 调整天空盒LOD
            PopItemWidth();
            Separator();

            BeginTabBar("InspectorTab", ImGuiTabBarFlags_None);

            if (BeginTabItem(UI::ToU8("静态模型"))) {
                PushItemWidth(130.0f);
                Checkbox(UI::ToU8("显示线框"), &gIsShowWireframe); SameLine();
                ColorEdit3(UI::ToU8("线框颜色"), GetValPtr(gWireframeColor), color_flags);  // 编辑线框颜色
                Checkbox(UI::ToU8("自动旋转"), &gIsRotateModel);                        // 自动旋转
                SliderFloat(UI::ToU8("粗糙度"), &gRoughness, 0.045f, 1.0f);              // 调整粗糙度
                SliderFloat(UI::ToU8("环境遮挡"), &gAO, 0.05f, 1.0f);              // 调整环境遮挡
                if (SliderFloat(UI::ToU8("透明度"), &cloth_alpha, 0.5f, 1.0f)) {     // 调整透明度
                    gAlbedo.a = cloth_alpha * 0.1f + 0.9f;
                }
                ColorEdit3("Albedo", GetValPtr(gAlbedo), color_flags);    SameLine();
                ColorEdit3("Sheen", GetValPtr(gSheenColor), color_flags);  SameLine();
                ColorEdit3("Subsurface", GetValPtr(gSubsurfColor), color_flags);
                PopItemWidth();
                EndTabItem();
            }

            if (gIsSimulate = BeginTabItem(UI::ToU8("仿真")); gIsSimulate) {
                Checkbox(UI::ToU8("显示线框"), &gIsShowWireframe); SameLine();
                ColorEdit3(UI::ToU8("线框颜色"), GetValPtr(gWireframeColor), color_flags);  // 编辑线框颜色
                Checkbox(UI::ToU8("应用透明清漆"), &simulation_clearcoat);              // 应用透明清漆
                gShadingModel = simulation_clearcoat ? uvec2(3, 1) : uvec2(3, 0);  // 设置着色模型

                if (ArrowButton("##1", ImGuiDir_Left)) {   // 向左箭头按钮
                    mResourceManager.Get<CShader>(30)->SetUniform(1, 20.0f * world::left);  // 设置风向
                }
                if (SameLine(); ArrowButton("##2", ImGuiDir_Right)) {  // 向右箭头按钮
                    mResourceManager.Get<CShader>(30)->SetUniform(1, 20.0f * world::right);  // 设置风向
                }
                if (SameLine(); ArrowButton("##3", ImGuiDir_Up)) {  // 向上箭头按钮
                    mResourceManager.Get<CShader>(30)->SetUniform(1, 20.0f * world::up);  // 设置风向
                }
                if (SameLine(); ArrowButton("##4", ImGuiDir_Down)) {  // 向下箭头按钮
                    mResourceManager.Get<CShader>(30)->SetUniform(1, world::zero);  // 重置风向
                }

                SameLine(); Text(UI::ToU8("风向"));  // 风向文本
                Spacing();
                if (Button(UI::ToU8("重置网格"), ImVec2(150.0f, 0.0f))) {  // 重置网格按钮
                    mPos[0]->SetData(&mInitPos[0]);  // 设置布料位置数据
                    mVel[0]->SetData(&mInitVel[0]);  // 设置布料速度数据
                    mPos[1]->SetData(NULL);         // 清空布料位置数据
                    mVel[1]->SetData(NULL);         // 清空布料速度数据
                }
                EndTabItem();
            }

            PushStyleColor(ImGuiCol_Tab, tabColorOff);
            PushStyleColor(ImGuiCol_TabHovered, tab_color_on);
            PushStyleColor(ImGuiCol_TabActive, tab_color_on);

            if (BeginTabItem(ICON_FK_TH_LARGE)) {
                PushItemWidth(130.0f);
                Checkbox(UI::ToU8("显示无限网格"), &gShowGrid);         // 显示无限网格
                SliderFloat(UI::ToU8("网格单元大小"), &gGridCellSize, 0.25f, 8.0f);  // 调整网格单元大小
                PopItemWidth();
                ColorEdit4(UI::ToU8("次要线颜色"), GetValPtr(gThinLineColor), color_flags);  // 编辑次要线颜色
                ColorEdit4(UI::ToU8("主要线颜色"), GetValPtr(gWideLineColor), color_flags);  // 编辑主要线颜色
                EndTabItem();
            }

            PopStyleColor(3);
            EndTabBar();

            Unindent(5.0f);
            UI::EndInspector();  // 结束检查器界面
        }

    }
	void Scene02::PrecomputeIBL(const std::string& hdri)
	{
        Renderer::SetSeamlessCubemap(true);
        Renderer::SetDepthTest(false);
        Renderer::SetFaceCulling(true);

        auto shaderIrradiance = CShader("core\\irradiance_map.glsl");       // 创建辐照度贴图着色器
        auto shaderPrefilter = CShader("core\\prefilter_envmap.glsl");      // 创建预过滤环境贴图着色器
        auto shaderEnvBRDF = CShader("core\\environment_BRDF.glsl");        // 创建环境BRDF贴图着色器

        auto mapEnv = std::make_shared<Texture>(hdri, 2048, 0);  // 创建环境贴图
        mapEnv->Bind(0);  // 绑定到纹理单元0

        mMapIrradiance = std::make_shared<Texture>(GL_TEXTURE_CUBE_MAP, 128, 128, 6, GL_RGBA16F, 1);        // 创建辐照度贴图
        mMapPrefiltered = std::make_shared<Texture>(GL_TEXTURE_CUBE_MAP, 2048, 2048, 6, GL_RGBA16F, 8);     // 创建预过滤环境贴图
        mMapBRDF_LUT = std::make_shared<Texture>(GL_TEXTURE_2D, 1024, 1024, 1, GL_RGBA16F, 1);              // 创建环境BRDF贴图

        LOG_INFO("预计算漫反射辐照度贴图: {0}", hdri); 
        mMapIrradiance->BindILS(0, 0, GL_WRITE_ONLY);  // 绑定到图像加载/存储纹理单元0

        shaderIrradiance.Bind();
        shaderIrradiance.Dispatch(128 / 32, 128 / 32, 6);  // 分发计算辐照度贴图
        shaderIrradiance.SyncWait(GL_TEXTURE_FETCH_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);  // 同步等待计算完成

        auto irradianceFence = Sync(0);  // 创建同步对象
        irradianceFence.WaitClientSync();  // 等待同步对象客户端完成
        mMapIrradiance->UnbindILS(0);  // 解绑辐照度贴图
        

        LOG_INFO("预计算镜面预过滤环境贴图: {0}", hdri);
        Texture::Copy(*mapEnv, 0, *mMapPrefiltered, 0);  // 复制基础级别

        const GLuint max_level = mMapPrefiltered->mLevel - 1;  // 最大级别
        GLuint resolution = mMapPrefiltered->mWidth / 2;           // 分辨率
        shaderPrefilter.Bind();  // 绑定预过滤着色器

        for (unsigned int level = 1; level <= max_level; level++, resolution /= 2) {
            float roughness = level / static_cast<float>(max_level);  // 粗糙度
            GLuint n_groups = glm::max<GLuint>(resolution / 32, 1);   // 计算组数

            mMapPrefiltered->BindILS(level, 1, GL_WRITE_ONLY);  // 绑定预过滤环境贴图到图像加载/存储纹理单元1
            shaderPrefilter.SetUniform(0, roughness);          // 设置粗糙度参数
            shaderPrefilter.Dispatch(n_groups, n_groups, 6);   // 分发计算预过滤环境贴图
            shaderPrefilter.SyncWait(GL_TEXTURE_FETCH_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);  // 同步等待计算完成

            auto prefilter_fence = Sync(level);  // 创建同步对象
            prefilter_fence.WaitClientSync();    // 等待同步对象客户端完成
            mMapPrefiltered->UnbindILS(1);       // 解绑预过滤环境贴图
        }

        LOG_INFO("预计算镜面环境BRDF贴图: {0}", hdri);
        mMapBRDF_LUT->BindILS(0, 2, GL_WRITE_ONLY);  // 绑定环境BRDF贴图到图像加载/存储纹理单元2

        shaderEnvBRDF.Bind();
        shaderEnvBRDF.Dispatch(1024 / 32, 1024 / 32, 1); 
        shaderEnvBRDF.SyncWait(GL_ALL_BARRIER_BITS);     
        Sync::WaitFinish();                               
        mMapBRDF_LUT->UnbindILS(2);                     
	}
    void Scene02::SetBuffers()
    {
        const GLuint numRows = gNumVerts.y;  // 网格行数
        const GLuint numCols = gNumVerts.x;  // 网格列数
        const GLuint n = numCols * numRows;  // 网格顶点数
        const float dx = gClothSz.x / (numCols - 1);  // 网格顶点间距（x方向）
        const float dy = gClothSz.y / (numRows - 1);  // 网格顶点间距（y方向）
        const float du = 1.0f / (numCols - 1);        // 纹理坐标增量（u方向）
        const float dv = 1.0f / (numRows - 1);        // 纹理坐标增量（v方向）

        mInitPos.reserve(n);   
        mInitVel.reserve(n);    
        mTextureCoord.reserve(n);  

        Transform texture = Transform();          
        texture.Rotate(world::right, -90.0f, Space::Local);                 // 绕本地坐标系的x轴旋转-90度
        texture.Translate(-gClothSz.x * 0.5f, 4.0f, gClothSz.y * 0.5f);     // 平移变换

        for (int row = 0; row < numRows; row++) {
            for (int col = 0; col < numCols; col++) {
                vec4 position = texture.mTransform * vec4(dx * col, dy * row, 0.0f, 1.0f);  // 计算位置
                vec4 velocity = vec4(0.0f);             // 初始速度为0
                vec2 uv = vec2(du * col, dv * row);     // 计算纹理坐标

                mInitPos.push_back(position);   
                mInitVel.push_back(velocity);   
                mTextureCoord.push_back(uv);  
            }
        }

        // 索引构建，每两行相邻行构成一个三角形带
        // 注意：三角形带的面方向由第一个三角形的顶点顺序决定，
        // 每个后续三角形的面方向将被反转以保持该顺序，这由OpenGL自动处理

        for (int row = 0; row < numRows - 1; row++) {
            mIndices.push_back(0xFFFFFF);  // 重启索引
            for (int col = 0; col < numCols; col++) {
                mIndices.push_back(row * numCols + col + numCols);
                mIndices.push_back(row * numCols + col);
            }
        }

        gNumIndices = static_cast<int>(mIndices.size());  // 设置索引数目

        LOG_ASSERT(sizeof(vec4) == 4 * sizeof(GLfloat), "GL中vec4的浮点数未紧密打包！");
        LOG_ASSERT(sizeof(vec2) == 2 * sizeof(GLfloat), "GL中vec2的浮点数未紧密打包！");

        mVAO = std::make_unique<VAO>();  
        mVBO = std::make_unique<VBO>(n * sizeof(vec2), &mTextureCoord[0]);              // 创建VBO（纹理坐标）
        mIBO = std::make_unique<IBO>(gNumIndices * sizeof(GLuint), &mIndices[0]);       // 创建IBO（索引缓冲）

        mPos[0] = std::make_unique<SSBO>(0, n * sizeof(vec4), GL_DYNAMIC_STORAGE_BIT);  // 创建SSBO（位置）
        mPos[1] = std::make_unique<SSBO>(1, n * sizeof(vec4), GL_DYNAMIC_STORAGE_BIT);  // 创建SSBO（位置备份）
        mVel[0] = std::make_unique<SSBO>(2, n * sizeof(vec4), GL_DYNAMIC_STORAGE_BIT);  // 创建SSBO（速度）
        mVel[1] = std::make_unique<SSBO>(3, n * sizeof(vec4), GL_DYNAMIC_STORAGE_BIT);  // 创建SSBO（速度备份）
        mNormal = std::make_unique<SSBO>(4, n * sizeof(vec4), GL_DYNAMIC_STORAGE_BIT);  // 创建SSBO（法线）

        mPos[0]->SetData(&mInitPos[0]);  // 设置位置数据
        mVel[0]->SetData(&mInitVel[0]);  // 设置速度数据

        mVAO->SetVBO(mPos[0]->mId, 0, 0, 3, sizeof(vec4), GL_FLOAT);  // 设置VAO的位置VBO
        mVAO->SetVBO(mNormal->mId, 1, 0, 3, sizeof(vec4), GL_FLOAT);  // 设置VAO的法线VBO
        mVAO->SetVBO(mVBO->mId, 2, 0, 2, sizeof(vec2), GL_FLOAT);     // 设置VAO的纹理坐标VBO
        mVAO->SetIBO(mIBO->mId);  // 设置VAO的索引缓冲

        auto simulationCS = mResourceManager.Get<CShader>(30);      // 获取计算着色器
        simulationCS->SetUniform(0, vec3(0.0f, -10.0f, 0.0f));      // 设置重力
        simulationCS->SetUniform(1, vec3(0.0f));                    // 设置风力
        simulationCS->SetUniform(2, dx);                            // 设置dx
        simulationCS->SetUniform(3, dy);                            // 设置dy
        simulationCS->SetUniform(4, sqrtf(dx * dx + dy * dy));      // 设置斜对角线长度
    }

    void Scene02::SetMaterial(Material& pbrMat, bool cloth, bool textured)
    {
        pbrMat.SetTexture(pbr_t::irradiance_map, mMapIrradiance);       // 设置PBR材质的漫反射辐照度贴图
        pbrMat.SetTexture(pbr_t::prefiltered_map, mMapPrefiltered);     // 设置PBR材质的预过滤环境贴图
        pbrMat.SetTexture(pbr_t::BRDF_LUT, mMapBRDF_LUT);               // 设置PBR材质的环境BRDF贴图

        pbrMat.BindUniform(0, &gIsShowWireframe);       // 绑定PBR材质的线框显示
        pbrMat.BindUniform(1, &gWireframeColor);        // 绑定PBR材质的线框颜色
        pbrMat.SetUniform(2, 0.05f);                    // 设置PBR材质的粗糙度
        pbrMat.BindUniform(3, &gSkyboxExposure);        // 绑定PBR材质的天空盒曝光度

        if (cloth) {
            pbrMat.SetUniform(pbr_u::shading_model, uvec2(3, 0));  // 设置PBR材质的着色模型为布料
            pbrMat.SetUniform(pbr_u::clearcoat, 1.0f);             // 设置PBR材质的透明度
            pbrMat.SetUniform(pbr_u::uv_scale, vec2(4.0f, 4.0f));  // 设置PBR材质的UV缩放
            std::string texPath = "texture\\fabric\\";              // 纹理路径

            if (textured) {
                pbrMat.SetTexture(pbr_t::albedo, std::make_shared<Texture>(texPath + "albedo.jpg"));        // 设置PBR材质的基色贴图
                pbrMat.SetTexture(pbr_t::normal, std::make_shared<Texture>(texPath + "normal.jpg"));        // 设置PBR材质的法线贴图
                pbrMat.SetTexture(pbr_t::roughness, std::make_shared<Texture>(texPath + "roughness.jpg"));  // 设置PBR材质的粗糙度贴图
                pbrMat.SetTexture(pbr_t::ao, std::make_shared<Texture>(texPath + "ao.jpg"));                // 设置PBR材质的环境光遮蔽贴图
                pbrMat.SetUniform(pbr_u::sheen_color, color::white);    // 设置PBR材质的光泽颜色
                pbrMat.SetUniform(pbr_u::subsurf_color, color::black);  // 设置PBR材质的次表面颜色
            }
            else {
                pbrMat.BindUniform(pbr_u::albedo, &gAlbedo);            // 绑定PBR材质的基色
                pbrMat.SetTexture(pbr_t::normal, std::make_shared<Texture>(texPath + "normal.jpg"));  // 设置PBR材质的法线贴图
                pbrMat.BindUniform(pbr_u::roughness, &gRoughness);          // 绑定PBR材质的粗糙度
                pbrMat.BindUniform(pbr_u::ao, &gAO);                        // 绑定PBR材质的环境光遮蔽
                pbrMat.BindUniform(pbr_u::sheen_color, &gSheenColor);       // 绑定PBR材质的光泽颜色
                pbrMat.BindUniform(pbr_u::subsurf_color, &gSubsurfColor);   // 绑定PBR材质的次表面颜色
            }
        }
        else {
            pbrMat.SetUniform(pbr_u::shading_model, uvec2(1, 0));  // 设置PBR材质的着色模型为金属
            pbrMat.SetUniform(pbr_u::metalness, 1.0f);             // 设置PBR材质的金属度
            pbrMat.SetUniform(pbr_u::roughness, 0.8f);             // 设置PBR材质的粗糙度
            pbrMat.SetUniform(pbr_u::ao, 1.0f);                     // 设置PBR材质的环境光遮蔽
        }
    }
}

