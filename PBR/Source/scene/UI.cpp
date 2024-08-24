#include "UI.h"
#define IMGUI_DISABLE_METRICS_WINDOW
#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imgui/imgui_impl_glut.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imstb_rectpack.h>
#include <imgui/imstb_textedit.h>
#include <imgui/imstb_truetype.h>
#include <ImGuizmo/ImGuizmo.h>
#include <IconFont/IconsForkAwesome.h>
#include <locale>
#include <codecvt>

#include "../utils/Log.h"
#include "../core/Window.h"
#include "../scene/Factory.h"
#include "../utils/Ext.h"
#include "../utils/Global.h"
#include "../core/Input.h"
#include "../core/Clock.h"
#include "../scene/Scene.h"

#pragma execution_character_set("utf-8")
using namespace core;
namespace scene {
    // 无背景、无装饰、无输入的窗口标志
    static const ImGuiWindowFlags gInvisibleWindowFlags = ImGuiWindowFlags_NoBackground
        | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs;

    void UI::Init()
    {
        std::string s = "大";
        std::string s1 = s;
        LOG_TRACK;
        ImGui::CreateContext();		// 创建ImGui上下文
        mWindowCenter = ImVec2(Window::Width(), Window::Height()) * 0.5f;
        SetFonts();
        ImGui::StyleColorsDark();  // 加载默认的深色主题
        SetStyle();
        SetColor();
        ImGui_ImplGlfw_InitForOpenGL(Window::mpWindow, false);
        ImGui_ImplOpenGL3_Init();
    }
    void UI::DrawWelcomeScreen(ImTextureID id)
    {
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();		// 获取背景绘制列表，用于在窗口背景上绘制内容
        // 在背景上添加一张图片、图片资源的唯一标识符，图片左上角坐标，图片右下角坐标
        drawList->AddImage(id, ImVec2(0.0f, 0.0f), ImVec2(Window::Width(), Window::Height()));
    }
    void UI::DrawMenuBar(std::string& newTitle,const std::string& currTitle)
    {
        using namespace ImGui;
        static bool sIsShowAboutWindow = false; // 是否显示关于窗口
        static bool sIsShowInstruction = false; // 是否显示使用说明窗口
        static bool sIsShowHomePopup = false; // 是否显示主菜单弹窗
        static bool sIsMusicOn = false; // 是否开启音乐

        //const auto& currSceneTitle1 = Renderer::CurrScene()->Title();
        SetNextWindowPos(ImVec2(0.0f, 0.0f));
        SetNextWindowSize(ImVec2(Window::Width(), 0.01f));
        SetNextWindowBgAlpha(0.0f);				// 设置窗口的背景透明度

        PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);			// 设置窗口边框大小
        PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 10));	    // 设置框架内边距
        PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12, 10));	    // 设置项目间距

        PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.0f, 0.0f, 0.0f, 0.75f));			    // 设置菜单栏背景色
        PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.0f, 0.0f, 0.0f, 0.55f));				// 设置弹出窗口背景色
        PushStyleColor(ImGuiCol_Header, ImVec4(0.22f, 0.39f, 0.61f, 0.8f));				// 设置标题颜色
        PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.24f, 0.54f, 0.89f, 0.8f));		// 设置标题悬停颜色

        Begin("菜单栏", 0, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
        
        if (BeginMenuBar()) {
            PushStyleColor(ImGuiCol_Border, ImVec4(0.7f, 0.7f, 0.7f, 0.3f));			// 设置边框颜色
            PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0.3f, 0.3f, 0.3f, 0.3f));		// 设置边框阴影颜色
            
            UI::AddMenuBarFont(newTitle, sIsShowInstruction, sIsShowAboutWindow);
            UI::AddMenuBarInco(currTitle, sIsShowHomePopup);
            
            PopStyleColor(2);		// 恢复边框颜色
            EndMenuBar();			// 结束菜单栏
        }
        
        End();				// 结束菜单栏窗口
        PopStyleColor(4);	// 恢复窗口颜色
        PopStyleVar(3);		// 恢复窗口样式

        UI::DrawUsageWindow(&sIsShowInstruction);
        UI::DrawAboutWindow(&sIsShowAboutWindow, "v1.0");
    }
    void UI::DrawStatusBar(void)
    {
        using namespace ImGui;
        // 设置下一个窗口的位置为窗口底部
        SetNextWindowPos(ImVec2(0.0f, Window::Height() - 32.0f));
        SetNextWindowSize(ImVec2((float)Window::Width(), 32.0f));
        SetNextWindowBgAlpha(0.75f);    // 设置下一个窗口的背景透明度

        // 开始绘制状态栏窗口
        Begin("状体栏", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
        PushFont(mFontMain);  // 推入字体
        {
            SameLine(0.0f, 9.0f);  // 同一行绘制，间隔9.0f
            TextColored(mColorCyan, "Cursor");
            SameLine(0.0f, 5.0f);
            ImVec2 pos = GetMousePos();
            Text("(%d, %d)", (int)pos.x, (int)pos.y);
            UI::DrawToolTip("当前鼠标位置在窗口空间中"); 

            SameLine(0.0f, 15.0f); UI::DrawVerticalLine(); SameLine(0.0f, 15.0f);   // 绘制垂直分隔线
            int hours = (int)floor(Clock::Time() / 3600.0f);                    // 计算小时数
            int minutes = (int)floor(fmod(Clock::Time(), 3600.0f) / 60.0f);     // 计算分钟数
            int seconds = (int)floor(fmod(Clock::Time(), 60.0f));               // 计算秒数

            TextColored(mColorCyan, "Clock");
            SameLine(0.0f, 5.0f);
            Text("%02d:%02d:%02d", hours, minutes, seconds);
            DrawToolTip("自应用程序启动以来的时间");

            SameLine(0.0f, 15.0f); DrawVerticalLine(); SameLine(0.0f, 15.0f);
            SameLine(GetWindowWidth() - 355);  // 同一行绘制，位置为窗口宽度减去355
            TextColored(mColorCyan, "FPS");  
            SameLine(0.0f, 5.0f);
            int fps = (int)Clock::Fps();
            auto text_color = fps > 90 ? mColorGreen : (fps < 30 ? mColorRed : mColorYellow); 
            TextColored(text_color, "(%d, %.2f ms)", fps, Clock::Ms()); 
            DrawToolTip("每秒帧数/每帧毫秒数");  

            SameLine(0.0f, 15.0f); DrawVerticalLine(); SameLine(0.0f, 15.0f);  // 绘制垂直分隔线
            TextColored(mColorCyan, "Window"); 
            SameLine(0.0f, 5.0f);
            Text("(%d, %d)", Window::Width(), Window::Height());  // 显示窗口宽度和高度
        }
        PopFont();      // 弹出字体
        End();          // 结束窗口

    }
    void UI::DrawToolTip(const char* desc, float spacing)
    {
        using namespace ImGui;
        SameLine(0.0f, spacing);
        TextDisabled("(?)");        // 显示禁用文本 "(?)"

        if (IsItemHovered()) { 
            PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));      // 推送文本颜色样式
            PushStyleColor(ImGuiCol_PopupBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));   // 推送弹出背景颜色样式
            BeginTooltip();                             // 开始工具提示
            PushTextWrapPos(GetFontSize() * 35.0f);     // 设置文本换行位置
            TextUnformatted(desc);                      // 显示未格式化的描述文本
            PopTextWrapPos();                           // 弹出文本换行位置
            EndTooltip();                               // 结束工具提示
            PopStyleColor(2);                           // 弹出颜色样式
        }
    }
    void UI::DrawLoadScreen()
    {
        using namespace ImGui;
        const static float ksWinW = (float)Window::Width();
        const static float ksWinH = (float)Window::Height();
        const static float ksBarW = 268.0f; // 加载条宽度
        const static float ksBarH = 80.0f;  // 加载条高度

        SetNextWindowPos(ImVec2(0.0f, 0.0f));
        SetNextWindowSize(ImVec2(ksWinW, ksWinH));
        SetNextWindowBgAlpha(1.0f);

        PushFont(mFontMain);
        Begin("加载条", 0, ImGuiWindowFlags_NoDecoration);     // 开始绘制加载条窗口

        ImDrawList* drawList = GetWindowDrawList();  // 获取窗口绘制列表
        drawList->AddText(GetFont(), GetFontSize() * 1.3f, ImVec2(ksWinW - ksBarW, ksWinH - ksBarH) * 0.5f,
            ColorConvertFloat4ToU32(mColorYellow), "正在加载，请稍候……");
        
        // 绘制三角形的坐标，大小，颜色值
        float x = 505.0f;
        float y = 465.0f;
        const static float size = 20.0f;
        float r, g, b;

        // 循环绘制颜色渐变的三角形
        for (float i = 0.0f; i < 1.0f; i += 0.05f, x += size * 1.5f) {  
            r = (i <= 0.33f) ? 1.0f : ((i <= 0.66f) ? 1 - (i - 0.33f) * 3 : 0.0f);  // 计算红色分量
            g = (i <= 0.33f) ? i * 3 : 1.0f;                // 计算绿色分量
            b = (i > 0.66f) ? (i - 0.66f) * 3 : 0.0f;       // 计算蓝色分量

            drawList->AddTriangleFilled(ImVec2(x, y - 0.5f * size), ImVec2(x, y + 0.5f * size),
                ImVec2(x + size, y), IM_COL32(r * 255, g * 255, b * 255, 255.0f));  // 绘制填充三角形
        }

        End();      // 结束窗口
        PopFont();  // 弹出字体
    }
    void UI::DrawTest()
    {
        // 创建一个简单的窗口
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Text("你好，世界！");                           // 显示一些文本
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // 使用滑块编辑 1 个 float 值
            if (ImGui::Button("按键"))                              // 按钮点击时返回 true
                counter++;
            ImGui::SameLine();
            ImGui::Text("计数器 = %d", counter);
        }
    }
    void UI::BeginFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();  // 初始化ImGui的OpenGL3新帧
        ImGui_ImplGlfw_NewFrame();     // 初始化ImGui的GLFW新帧
        ImGui::NewFrame();             // 手动调用ImGui的新帧初始化
        ImGuizmo::BeginFrame();        // 初始化ImGuizmo新帧
    }
    void UI::EndFrame()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    bool UI::NewInspector(void) 
    {
        using namespace ImGui;
        static const float width = 256.0f * 1.25f;  // 检查器窗口的宽度，适用于1600x900分辨率
        static const float height = 612.0f * 1.25f;  // 检查器窗口的高度

        SetNextWindowPos(ImVec2(Window::mWidth - width, (Window::mHeight - height) * 0.5f));  // 设置窗口位置
        SetNextWindowSize(ImVec2(width, height));  // 设置窗口大小

        static ImGuiWindowFlags inspectorFlags = ImGuiWindowFlags_NoMove |  // 检查器窗口的标志，禁止移动、调整大小和折叠
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

        PushID("Inspector Window");  // 推送检查器窗口的ID

        if (Begin(ICON_FK_LOCATION_ARROW " 检测器", 0, inspectorFlags)) {  // 创建检查器窗口
            return true;  // 如果窗口创建成功，返回true
        }

        LOG_ERROR("由于裁剪问题，加载检查器失败...");
        LOG_ERROR("你是否绘制了全屏不透明窗口？");
        return false;
    }
    void UI::EndInspector(void)
    {
        ImGui::End();  // 结束ImGui窗口
        ImGui::PopID();  // 弹出检查器窗口的ID
    }
    void UI::DrawGizmo(scene::Entity& camera, scene::Entity& target, Gizmo z)
    {
        static const ImVec2 winPos = ImVec2(0.0f, 50.0f);  // 窗口位置
        static const ImVec2 winSize = ImVec2((float)Window::mWidth, (float)Window::mHeight - 82.0f);  // 窗口大小

        ImGuizmo::MODE mode = ImGuizmo::MODE::LOCAL;  // 默认Gizmo模式为局部模式
        ImGuizmo::OPERATION operation = ImGuizmo::OPERATION::TRANSLATE;  // 默认Gizmo操作为平移

        switch (z) {  // 根据传入的Gizmo类型设置操作
        case Gizmo::Translate: {
            operation = ImGuizmo::OPERATION::TRANSLATE;  // 设置操作为平移
            break;
        }
        case Gizmo::Rotate: {
            operation = ImGuizmo::OPERATION::ROTATE;  // 设置操作为旋转
            break;
        }
        case Gizmo::Scale: {
            operation = ImGuizmo::OPERATION::SCALE;  // 设置操作为缩放
            break;
        }
        case Gizmo::Bounds: case Gizmo::None: default: {
            return;  // 对于其他类型，不进行操作，直接返回
        }
        }

        auto& T = target.GetComponent<Transform>();  // 获取目标实体的Transform组件
        auto& C = camera.GetComponent<Camera>();  // 获取相机实体的Camera组件
        glm::mat4 V = C.GetViewMatrix();  // 获取相机的视图矩阵
        glm::mat4 P = C.GetProjectionMatrix();  // 获取相机的投影矩阵

        // 转换模型矩阵为左手坐标系，因为ImGuizmo假设使用左手坐标系
        static const glm::vec3 RvL = glm::vec3(1.0f, 1.0f, -1.0f);  // 右手到左手的缩放向量
        glm::mat4 transform = glm::scale(T.mTransform, RvL);  // 应用缩放

        ImGui::SetNextWindowPos(winPos);  // 设置下一个窗口的位置
        ImGui::SetNextWindowSize(winSize);  // 设置下一个窗口的大小
        ImGui::Begin("##Invisible Gizmo Window", 0, gInvisibleWindowFlags);  // 创建一个不可见窗口用于绘制Gizmo

        ImGuizmo::SetOrthographic(true);  // 设置Gizmo为正交模式
        ImGuizmo::SetDrawlist();  // 设置绘图列表
        ImGuizmo::SetRect(winPos.x, winPos.y, winSize.x, winSize.y);  // 设置Gizmo绘制区域
        ImGuizmo::Manipulate(value_ptr(V), value_ptr(P), operation, mode, value_ptr(transform));  // 操作Gizmo

        // 如果Gizmo正在被使用，则更新目标实体的Transform组件
        if (ImGuizmo::IsUsing()) {
            transform = glm::scale(transform, RvL);  // 转换回右手坐标系
            T.SetTransform(transform);  // 设置Transform组件
        }

        ImGui::End();  // 结束ImGui窗口
    }

    void UI::DrawRainbowBar(const ImVec2& offset, float height)
    {
        // 绘制给定高度的彩虹条
        // 彩虹条的宽度会自动调整以居中显示在窗口内
        // 偏移量是相对于窗口左上角的像素

        // 该函数借鉴并修改自unknown cheats论坛
        // 来源：https://www.unknowncheats.me/forum/2550901-post1.html

        float speed = 0.0006f;  // 彩虹条颜色变化的速度
        static float static_hue = 0.0f;  // 静态色相值

        ImDrawList* drawList = ImGui::GetWindowDrawList();  // 获取窗口绘图列表
        ImVec2 pos = ImGui::GetWindowPos() + offset;  // 计算彩虹条的位置
        float width = ImGui::GetWindowWidth() - offset.x * 2.0f;  // 计算彩虹条的宽度

        static_hue -= speed;  // 更新静态色相值
        if (static_hue < -1.0f) {
            static_hue += 1.0f;  // 确保色相值在0到1之间循环
        }

        for (int i = 0; i < width; i++) {  // 遍历彩虹条的宽度
            float hue = static_hue + (1.0f / width) * i;  // 计算当前像素的色相值
            if (hue < 0.0f) hue += 1.0f;  // 确保色相值在0到1之间
            ImColor color = ImColor::HSV(hue, 1.0f, 1.0f);  // 计算颜色
            drawList->AddRectFilled(ImVec2(pos.x + i, pos.y), ImVec2(pos.x + i + 1, pos.y + height), color);  // 绘制彩虹条的一个像素
        }
    }

    char* UI::ToU8(const std::string str)
    {
        const char* GBK_LOCALE_NAME = "CHS";

        std::wstring_convert<std::codecvt<wchar_t, char, mbstate_t>>
            conv(new std::codecvt<wchar_t, char, mbstate_t>(GBK_LOCALE_NAME));
        std::wstring wString = conv.from_bytes(str);            // string => wstring

        std::wstring_convert<std::codecvt_utf8<wchar_t>> convert;
        std::string utf8str = convert.to_bytes(wString);        // wstring => utf-8

        char* cstr = new char[utf8str.length() + 1];
        strcpy(cstr, utf8str.c_str());
        return cstr;
    }

    bool UI::BeginTabItemX(const std::string& label)
    {
        return ImGui::BeginTabItem(UI::ToU8(label));
    }

    void UI::CheckboxX(const std::string& label, bool& value)
    {
        ImGui::Checkbox(UI::ToU8(label), &value);
        ImGui::Separator();
    }

    glm::ivec2 UI::GetCursorPosition()
    {
        if (Window::mLayer == Layer::Scene) {  // 如果当前窗口层是场景层
            return Input::GetCursorPosition();  // 返回输入的光标位置
        }

        auto mousePos = ImGui::GetMousePos();  // 获取ImGui的鼠标位置
        return glm::ivec2(mousePos.x, mousePos.y);  // 返回鼠标位置
    }
    void UI::SetFonts()
    {
        // main:主、sub:次、icon:图标
        std::string fontMain = "C:\\Windows\\Fonts\\simkai.ttf";
        std::string fontSub = "C:\\Windows\\Fonts\\simkai.ttf";
        std::string fontIcon = gFontPath + FONT_ICON_FILE_NAME_FK;
        LOG_ASSERT_FILE(fontMain, fontSub, fontIcon);

        float fontSizeMain = 18.0f;
        float fontSizeIcon = 18.0f;
        float fontSizeSub = 17.0f;

        // 主字体配置
        ImFontConfig configMain;
        configMain.PixelSnapH = true;			// 启用像素对齐,这将使字体在水平方向上对齐到最近的像素
        configMain.OversampleH = 4;				// 水平方向上字体过采样,渲染时每个像素的采样点数为4
        configMain.OversampleV = 4;				// 垂直方向上字体过采样,渲染时每个像素的采样点数为4
        configMain.RasterizerMultiply = 1.2f;	// 光栅化乘数，1.0是正常强度，1.2会使字体稍微变粗
        configMain.GlyphExtraSpacing.x = 0.0;	// 字符间额外间距，0.0表示没有额外间距

        // 次字体配置
        ImFontConfig configSub;
        configSub.PixelSnapH = true;
        configSub.OversampleH = 4;
        configSub.OversampleV = 4;
        configSub.RasterizerMultiply = 1.25f;
        configSub.GlyphExtraSpacing.x = 0.0f;

        // 图标字体配置
        ImFontConfig configIcon;
        // 启用合并模式,这将使新加载的字体与现有字体合并，从而可以在同一个字体集中使用多个字体
        configIcon.MergeMode = true;
        configIcon.PixelSnapH = true;
        configIcon.OversampleV = 4;
        configIcon.OversampleH = 4;
        configIcon.RasterizerMultiply = 1.5f;
        configIcon.GlyphOffset.y = 0.0f;
        // 设置字符的最大水平进度,这将限制字体中字符的最大水平进度（宽度），通常用于确保图标字体与主字体的大小一致
        configIcon.GlyphMaxAdvanceX = fontSizeMain;
        configIcon.GlyphMinAdvanceX = fontSizeMain;

        static const ImWchar iconRanges[] = { ICON_MIN_FK,ICON_MAX_FK,0 };	// 字符范围，以零结尾
        ImGuiIO& io = ImGui::GetIO();				// 获取ImGui输入输出对象的引用
        mFontMain = io.Fonts->AddFontFromFileTTF(fontMain.c_str(), fontSizeMain, &configMain, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
        mFontIcon = io.Fonts->AddFontFromFileTTF(fontIcon.c_str(), fontSizeIcon, &configIcon, iconRanges);
        mFontSub = io.Fonts->AddFontFromFileTTF(fontSub.c_str(), fontSizeSub, &configSub, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());

        // 构建字体纹理
        unsigned char* pixels;
        int width, height, bytesPerPixel;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytesPerPixel);
    }
    void UI::SetStyle()
    {
        // 设置自定义样式参数
        ImGuiStyle& style = ImGui::GetStyle();	// 获取ImGUI样式对象引用
        // 设置边框
        style.WindowBorderSize = 0.0f;			// 窗口
        style.FrameBorderSize = 1.0;			// 框架（如按键、输入框等）
        style.PopupBorderSize = 1.0f;			// 弹出窗口（如下拉菜单、对话框等）
        style.ChildBorderSize = 1.0f;			// 子窗口（如嵌套窗口、子面板等
        style.TabBorderSize = 0.0f;				// 标签页（如选项卡）
        style.ScrollbarSize = 18.0f;			// 滚动条的宽度
        style.GrabMinSize = 10.f;				// 拖动控件（如滑块和进度条）的最小抓取区域
        // 设置内边距
        style.WindowPadding = ImVec2(8.0f, 8.0f);
        style.FramePadding = ImVec2(4.0f, 6.0f);
        style.ItemSpacing = ImVec2(10.0f, 10.0f);
        style.ItemInnerSpacing = ImVec2(10.0, 10.0f);
        style.IndentSpacing = 16.0f;			// 缩进边距
        // 设置圆角半径
        style.WindowRounding = 0.0f;
        style.ChildRounding = 0.0f;
        style.FrameRounding = 4.0f;
        style.PopupRounding = 0.0f;
        style.TabRounding = 4.0;
        style.GrabRounding = 4.0f;
        style.ScrollbarRounding = 12.0f;

        style.WindowMenuButtonPosition = ImGuiDir_Left;
        style.ColorButtonPosition = ImGuiDir_Right;
        style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
        style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
        style.SelectableTextAlign = ImVec2(0.0f, 0.0f);	// 可选择文本对齐
        style.AntiAliasedLines = true;			// 抗锯齿线条
        style.AntiAliasedFill = true;			// 抗锯齿填充
        style.AntiAliasedLinesUseTex = true;			// 使用纹理的抗锯齿线条
    }
    void UI::SetColor()
    {
        // 设置自定义颜色
        auto& color = ImGui::GetStyle().Colors;				// 获取样式中的颜色数组的引用
        // 设置窗口、子窗口、弹出窗口的背景颜色
        color[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.85f);
        color[ImGuiCol_ChildBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.85f);
        color[ImGuiCol_PopupBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.85f);
        // 设置帧、帧悬停、帧激活时的背景颜色
        color[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.75f);
        color[ImGuiCol_FrameBgHovered] = ImVec4(0.2f, 0.2f, 0.2f, 0.75f);
        color[ImGuiCol_FrameBgActive] = ImVec4(0.3f, 0.3f, 0.3f, 0.75f);
        // 设置标题、标题激活、标题折叠时的背景元素
        color[ImGuiCol_TitleBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.75f);
        color[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.3f, 0.0f, 0.9f);
        color[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.75f);
        // 设置滚动条、滚动条抓取、滚动条悬停、滚动条激活时的背景颜色
        color[ImGuiCol_ScrollbarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.75f);
        color[ImGuiCol_ScrollbarGrab] = ImVec4(0.2f, 0.2f, 0.2f, 0.9f);
        color[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.3f, 0.3f, 0.3f, 0.9f);
        color[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.4f, 0.4f, 0.4f, 0.9f);
        // 设置勾选标记、滚动条抓取、滚动条激活时的抓取颜色
        color[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        color[ImGuiCol_SliderGrab] = ImVec4(0.0f, 0.4f, 0.0f, 0.9f);
        color[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.5f, 0.0f, 0.9f);
        // 设置按钮、按钮悬停、按钮激活时的背景颜色
        color[ImGuiCol_Button] = ImVec4(0.0f, 0.3f, 0.0f, 0.9f);
        color[ImGuiCol_ButtonHovered] = ImVec4(0.0f, 0.55f, 0.0f, 0.9f);
        color[ImGuiCol_ButtonActive] = ImVec4(0.0f, 0.5f, 0.0f, 0.9f);
        // 设置表头、标题悬停、标题激活时的颜色
        color[ImGuiCol_Header] = ImVec4(0.5f, 0.0f, 1.0f, 0.5f);
        color[ImGuiCol_HeaderHovered] = ImVec4(0.5f, 0.0f, 1.0f, 0.8f);
        color[ImGuiCol_HeaderActive] = ImVec4(0.5f, 0.0f, 1.0f, 0.7f);
        // 设置选项卡、选项卡悬停、选项卡激活、选项卡未聚焦、选项卡未聚焦但激活时的颜色
        color[ImGuiCol_Tab] = ImVec4(0.0f, 0.3f, 0.0f, 0.8f);
        color[ImGuiCol_TabHovered] = ImVec4(0.0f, 0.4f, 0.0f, 0.8f);
        color[ImGuiCol_TabActive] = ImVec4(0.0f, 0.4f, 0.0f, 0.8f);
        color[ImGuiCol_TabUnfocused] = ImVec4(0.2f, 0.2f, 0.2f, 0.9f);
        color[ImGuiCol_TabUnfocusedActive] = ImVec4(0.2f, 0.2f, 0.2f, 0.9f);

    }
    void UI::DrawUsageWindow(bool* isShow)
    {
        if (!*isShow) return;
        using namespace ImGui;
        SetNextWindowSize(ImVec2(1280.0f / 2.82f, 720.0f / 1.6f));      // 设置窗口大小
        if (!Begin("使用方法", isShow, ImGuiWindowFlags_NoResize)) { 
            End();
            return;
        }
        Spacing();  // 添加空白

        const ImVec4 textColor = ImVec4(0.4f, 0.8f, 0.4f, 1.0f);  // 文本颜色
        const char instructions[] = "只需玩一下就能发现";       // 使用说明文本

        if (TreeNode("基础指南")) { 
            Spacing();
            Indent(10.0f);                              // 缩进
            PushStyleColor(ImGuiCol_Text, textColor);   // 设置文本颜色
            PushTextWrapPos(412.903f);                  // 设置文本换行位置
            TextWrapped(instructions);                  // 显示包裹的文本
            PopTextWrapPos();                           // 恢复文本换行位置
            PopStyleColor();                            // 恢复文本颜色
            ImGui::Unindent(10.0f);                            // 取消缩进
            TreePop();                                  // 关闭“基础指南”节点
        }

        if (TreeNode("鼠标")) {
            Spacing();
            BulletText("移动光标旋转相机。");
            BulletText("按住右键并滑动来缩放。");
            BulletText("按住左键以弧球模式旋转。");
            Spacing();
            TreePop();
        }

        // 定义一个用于显示彩色文本的lambda函数
        static auto colorText = [](const char* text) {      
            PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            BulletText(text);       // 显示彩色文本
            PopStyleColor();        // 恢复文本颜色
            SameLine(128.0f, 0.0f); // 在同一行显示
        };

        if (TreeNode("键盘")) {
            Spacing();
            colorText("Enter");
            Text("切换UI检查器开/关。"); 
            colorText("Escape"); 
            Text("确认退出窗口。");
            colorText("WASD");
            Text("在四个平面方向移动相机。");
            colorText("Space/Z");
            Text("向上/向下移动相机。");
            colorText("R");
            Text("重置相机到初始位置。");
            Spacing();
            TreePop();
        }

        End();  // 结束绘制窗口
    }
    void UI::DrawAboutWindow(bool* isShow, const char* version)
    {
        using namespace ImGui;
        if (!*isShow) return;
        if (!Begin("关于PBR项目", isShow, ImGuiWindowFlags_AlwaysAutoResize)) {
            End();
            return;
        }

        Text("PBR %s", version);    // 显示软件版本
        Separator();                // 绘制分隔符
        Text("由 LZK 编写的开源作品，2024年7月。");
        Text("一个用于在 OpenGL 中进行图形处理的简单渲染库。");
        Separator();

        static bool sIsShowContactInfo = false;         // 是否显示联系信息的标志
        Checkbox("如何联系我", &sIsShowContactInfo);   // 显示复选框

        if (sIsShowContactInfo) {
            SameLine(0.0f, 90.0f);          // 在同一行显示
            bool isCopyToClipboard = Button("COPY", ImVec2(48.0f, 0.0f));           // 显示“COPY”按钮
            ImVec2 childSize = ImVec2(0, GetTextLineHeightWithSpacing() * 2.2f);    // 子窗口大小
            BeginChildFrame(GetID("Contact"), childSize, ImGuiWindowFlags_NoMove);  // 开始子窗口

            if (isCopyToClipboard) LogToClipboard();    // 如果按下“COPY”按钮，将信息复制到剪贴板
            {
                Text("Email: liuzhikun2022@163.com");  // 显示邮箱
                Text("Github: https://github.com/lzk2022");  // 显示GitHub地址
            }
            if (isCopyToClipboard) LogFinish();  // 结束复制

            EndChildFrame();  // 结束子窗口
        }

        End();
    }

    int UI::DrawPopupModel(const char* title, const char* message, const ImVec2& size)
    {
        return 0;
    }
    void UI::AddMenuBarFont(std::string& newTitle,bool& isShowInstruction,bool& isShowAboutWindow)
    {
        using namespace ImGui;
        if (BeginMenu("打开")) {
            for (unsigned int i = 0; i < scene::factory::gkTitles.size(); i++) {	// 遍历所有场景标题
                std::string title = scene::factory::gkTitles[i];					// 当前场景标题
                std::ostringstream id;												// 标题ID流
                id << " " << std::setfill('0') << std::setw(2) << i;				// 格式化ID
                bool selected = (newTitle == title);								// 判断是否为当前选择的标题
                if (MenuItem(utils::ToU8(title).c_str(), id.str().c_str(), selected)) {  // 如果菜单项被选中
                    if (!selected) {												// 如果不是当前选择的标题
                        newTitle = title;											// 更新新标题
                    }
                }
            }
            EndMenu();
        }

        if (BeginMenu("选项")) {
            if (BeginMenu("窗口分辨率")) {
                MenuItem(" 1280 x 720", NULL, false);
                MenuItem(" 1600 x 900", NULL, true);
                MenuItem(" 1920 x 1080", NULL, false);
                MenuItem(" 2560 x 1440", NULL, false);
                EndMenu();
            }
            EndMenu();
        }

        if (BeginMenu("帮助")) {
            isShowInstruction |= MenuItem("使用说明", "F1");
            isShowAboutWindow |= MenuItem("关于", "F8");
            EndMenu();
        }
    }
    void UI::AddMenuBarInco(const std::string& currTitle,bool& isBackHome)
    {
        using namespace ImGui;
        static bool musicOn = true;            // 音乐是否开启
        // 菜单图标项
        SameLine(GetWindowWidth() - 303.0f);                    // 在同一行显示图标项
        static const ImVec4 tooltipBgColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);  // 工具提示背景颜色
        if (MenuItem(ICON_FK_HOME)) {                           // 主页 图标项
            if (utils::ToU8(currTitle) != "欢迎界面") {          // 如果当前场景不是欢迎屏幕
                isBackHome = true;                        // 显示主菜单弹窗
            }
        }
        else if (IsItemHovered()) {                             // 如果 主页 图标项被悬停
            PushStyleColor(ImGuiCol_PopupBg, tooltipBgColor);   // 设置工具提示背景颜色
            BeginTooltip();                                     // 开始工具提示
            TextUnformatted("返回主菜单");                       // 显示工具提示文本
            EndTooltip();                                       // 结束工具提示
            PopStyleColor();                                    // 恢复背景颜色
        }

        if (MenuItem(Window::mLayer == Layer::ImGui ? ICON_FK_PICTURE_O : ICON_FK_COFFEE)) {  // “图像”或“咖啡”图标项
            //Input::SetKeyDown(VK_RETURN, true);  // 按下回车键
        }
        else if (IsItemHovered()) {  // 如果“图像”或“咖啡”图标项被悬停
            PushStyleColor(ImGuiCol_PopupBg, tooltipBgColor);  // 设置工具提示背景颜色
            BeginTooltip();  // 开始工具提示
            TextUnformatted("返回界面模型 (Enter)");  // 显示工具提示文本
            EndTooltip();  // 结束工具提示
            PopStyleColor();  // 恢复背景颜色
        }

        if (MenuItem(musicOn ? ICON_FK_VOLUME_UP : ICON_FK_VOLUME_MUTE)) {  // “音量”图标项
            musicOn = !musicOn;  // 切换音乐开关
        }
        else if (IsItemHovered()) {  // 如果“音量”图标项被悬停
            PushStyleColor(ImGuiCol_PopupBg, tooltipBgColor);  
            BeginTooltip();  
            TextUnformatted("音乐 开/关"); 
            EndTooltip();  
            PopStyleColor();  
        }

        // 电源图标
        if (MenuItem(ICON_FK_POWER_OFF)) {
            Input::SetKeyDown(KB_ESCAPE, true);
        }
        else if (IsItemHovered()) {
            PushStyleColor(ImGuiCol_PopupBg, tooltipBgColor);
            BeginTooltip();
            TextUnformatted("退出程序 (Esc)");
            EndTooltip();
            PopStyleColor();
        }
    }
    void UI::DrawVerticalLine()
    {
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);  // 绘制垂直分隔线
    }
}
