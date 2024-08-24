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
    // �ޱ�������װ�Ρ�������Ĵ��ڱ�־
    static const ImGuiWindowFlags gInvisibleWindowFlags = ImGuiWindowFlags_NoBackground
        | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs;

    void UI::Init()
    {
        std::string s = "��";
        std::string s1 = s;
        LOG_TRACK;
        ImGui::CreateContext();		// ����ImGui������
        mWindowCenter = ImVec2(Window::Width(), Window::Height()) * 0.5f;
        SetFonts();
        ImGui::StyleColorsDark();  // ����Ĭ�ϵ���ɫ����
        SetStyle();
        SetColor();
        ImGui_ImplGlfw_InitForOpenGL(Window::mpWindow, false);
        ImGui_ImplOpenGL3_Init();
    }
    void UI::DrawWelcomeScreen(ImTextureID id)
    {
        ImDrawList* drawList = ImGui::GetBackgroundDrawList();		// ��ȡ���������б������ڴ��ڱ����ϻ�������
        // �ڱ��������һ��ͼƬ��ͼƬ��Դ��Ψһ��ʶ����ͼƬ���Ͻ����꣬ͼƬ���½�����
        drawList->AddImage(id, ImVec2(0.0f, 0.0f), ImVec2(Window::Width(), Window::Height()));
    }
    void UI::DrawMenuBar(std::string& newTitle,const std::string& currTitle)
    {
        using namespace ImGui;
        static bool sIsShowAboutWindow = false; // �Ƿ���ʾ���ڴ���
        static bool sIsShowInstruction = false; // �Ƿ���ʾʹ��˵������
        static bool sIsShowHomePopup = false; // �Ƿ���ʾ���˵�����
        static bool sIsMusicOn = false; // �Ƿ�������

        //const auto& currSceneTitle1 = Renderer::CurrScene()->Title();
        SetNextWindowPos(ImVec2(0.0f, 0.0f));
        SetNextWindowSize(ImVec2(Window::Width(), 0.01f));
        SetNextWindowBgAlpha(0.0f);				// ���ô��ڵı���͸����

        PushStyleVar(ImGuiStyleVar_WindowBorderSize, 1.0f);			// ���ô��ڱ߿��С
        PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 10));	    // ���ÿ���ڱ߾�
        PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(12, 10));	    // ������Ŀ���

        PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.0f, 0.0f, 0.0f, 0.75f));			    // ���ò˵�������ɫ
        PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.0f, 0.0f, 0.0f, 0.55f));				// ���õ������ڱ���ɫ
        PushStyleColor(ImGuiCol_Header, ImVec4(0.22f, 0.39f, 0.61f, 0.8f));				// ���ñ�����ɫ
        PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.24f, 0.54f, 0.89f, 0.8f));		// ���ñ�����ͣ��ɫ

        Begin("�˵���", 0, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
        
        if (BeginMenuBar()) {
            PushStyleColor(ImGuiCol_Border, ImVec4(0.7f, 0.7f, 0.7f, 0.3f));			// ���ñ߿���ɫ
            PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0.3f, 0.3f, 0.3f, 0.3f));		// ���ñ߿���Ӱ��ɫ
            
            UI::AddMenuBarFont(newTitle, sIsShowInstruction, sIsShowAboutWindow);
            UI::AddMenuBarInco(currTitle, sIsShowHomePopup);
            
            PopStyleColor(2);		// �ָ��߿���ɫ
            EndMenuBar();			// �����˵���
        }
        
        End();				// �����˵�������
        PopStyleColor(4);	// �ָ�������ɫ
        PopStyleVar(3);		// �ָ�������ʽ

        UI::DrawUsageWindow(&sIsShowInstruction);
        UI::DrawAboutWindow(&sIsShowAboutWindow, "v1.0");
    }
    void UI::DrawStatusBar(void)
    {
        using namespace ImGui;
        // ������һ�����ڵ�λ��Ϊ���ڵײ�
        SetNextWindowPos(ImVec2(0.0f, Window::Height() - 32.0f));
        SetNextWindowSize(ImVec2((float)Window::Width(), 32.0f));
        SetNextWindowBgAlpha(0.75f);    // ������һ�����ڵı���͸����

        // ��ʼ����״̬������
        Begin("״����", 0, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove);
        PushFont(mFontMain);  // ��������
        {
            SameLine(0.0f, 9.0f);  // ͬһ�л��ƣ����9.0f
            TextColored(mColorCyan, "Cursor");
            SameLine(0.0f, 5.0f);
            ImVec2 pos = GetMousePos();
            Text("(%d, %d)", (int)pos.x, (int)pos.y);
            UI::DrawToolTip("��ǰ���λ���ڴ��ڿռ���"); 

            SameLine(0.0f, 15.0f); UI::DrawVerticalLine(); SameLine(0.0f, 15.0f);   // ���ƴ�ֱ�ָ���
            int hours = (int)floor(Clock::Time() / 3600.0f);                    // ����Сʱ��
            int minutes = (int)floor(fmod(Clock::Time(), 3600.0f) / 60.0f);     // ���������
            int seconds = (int)floor(fmod(Clock::Time(), 60.0f));               // ��������

            TextColored(mColorCyan, "Clock");
            SameLine(0.0f, 5.0f);
            Text("%02d:%02d:%02d", hours, minutes, seconds);
            DrawToolTip("��Ӧ�ó�������������ʱ��");

            SameLine(0.0f, 15.0f); DrawVerticalLine(); SameLine(0.0f, 15.0f);
            SameLine(GetWindowWidth() - 355);  // ͬһ�л��ƣ�λ��Ϊ���ڿ�ȼ�ȥ355
            TextColored(mColorCyan, "FPS");  
            SameLine(0.0f, 5.0f);
            int fps = (int)Clock::Fps();
            auto text_color = fps > 90 ? mColorGreen : (fps < 30 ? mColorRed : mColorYellow); 
            TextColored(text_color, "(%d, %.2f ms)", fps, Clock::Ms()); 
            DrawToolTip("ÿ��֡��/ÿ֡������");  

            SameLine(0.0f, 15.0f); DrawVerticalLine(); SameLine(0.0f, 15.0f);  // ���ƴ�ֱ�ָ���
            TextColored(mColorCyan, "Window"); 
            SameLine(0.0f, 5.0f);
            Text("(%d, %d)", Window::Width(), Window::Height());  // ��ʾ���ڿ�Ⱥ͸߶�
        }
        PopFont();      // ��������
        End();          // ��������

    }
    void UI::DrawToolTip(const char* desc, float spacing)
    {
        using namespace ImGui;
        SameLine(0.0f, spacing);
        TextDisabled("(?)");        // ��ʾ�����ı� "(?)"

        if (IsItemHovered()) { 
            PushStyleColor(ImGuiCol_Text, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));      // �����ı���ɫ��ʽ
            PushStyleColor(ImGuiCol_PopupBg, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));   // ���͵���������ɫ��ʽ
            BeginTooltip();                             // ��ʼ������ʾ
            PushTextWrapPos(GetFontSize() * 35.0f);     // �����ı�����λ��
            TextUnformatted(desc);                      // ��ʾδ��ʽ���������ı�
            PopTextWrapPos();                           // �����ı�����λ��
            EndTooltip();                               // ����������ʾ
            PopStyleColor(2);                           // ������ɫ��ʽ
        }
    }
    void UI::DrawLoadScreen()
    {
        using namespace ImGui;
        const static float ksWinW = (float)Window::Width();
        const static float ksWinH = (float)Window::Height();
        const static float ksBarW = 268.0f; // ���������
        const static float ksBarH = 80.0f;  // �������߶�

        SetNextWindowPos(ImVec2(0.0f, 0.0f));
        SetNextWindowSize(ImVec2(ksWinW, ksWinH));
        SetNextWindowBgAlpha(1.0f);

        PushFont(mFontMain);
        Begin("������", 0, ImGuiWindowFlags_NoDecoration);     // ��ʼ���Ƽ���������

        ImDrawList* drawList = GetWindowDrawList();  // ��ȡ���ڻ����б�
        drawList->AddText(GetFont(), GetFontSize() * 1.3f, ImVec2(ksWinW - ksBarW, ksWinH - ksBarH) * 0.5f,
            ColorConvertFloat4ToU32(mColorYellow), "���ڼ��أ����Ժ򡭡�");
        
        // ���������ε����꣬��С����ɫֵ
        float x = 505.0f;
        float y = 465.0f;
        const static float size = 20.0f;
        float r, g, b;

        // ѭ��������ɫ�����������
        for (float i = 0.0f; i < 1.0f; i += 0.05f, x += size * 1.5f) {  
            r = (i <= 0.33f) ? 1.0f : ((i <= 0.66f) ? 1 - (i - 0.33f) * 3 : 0.0f);  // �����ɫ����
            g = (i <= 0.33f) ? i * 3 : 1.0f;                // ������ɫ����
            b = (i > 0.66f) ? (i - 0.66f) * 3 : 0.0f;       // ������ɫ����

            drawList->AddTriangleFilled(ImVec2(x, y - 0.5f * size), ImVec2(x, y + 0.5f * size),
                ImVec2(x + size, y), IM_COL32(r * 255, g * 255, b * 255, 255.0f));  // �������������
        }

        End();      // ��������
        PopFont();  // ��������
    }
    void UI::DrawTest()
    {
        // ����һ���򵥵Ĵ���
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Text("��ã����磡");                           // ��ʾһЩ�ı�
            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // ʹ�û���༭ 1 �� float ֵ
            if (ImGui::Button("����"))                              // ��ť���ʱ���� true
                counter++;
            ImGui::SameLine();
            ImGui::Text("������ = %d", counter);
        }
    }
    void UI::BeginFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();  // ��ʼ��ImGui��OpenGL3��֡
        ImGui_ImplGlfw_NewFrame();     // ��ʼ��ImGui��GLFW��֡
        ImGui::NewFrame();             // �ֶ�����ImGui����֡��ʼ��
        ImGuizmo::BeginFrame();        // ��ʼ��ImGuizmo��֡
    }
    void UI::EndFrame()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    bool UI::NewInspector(void) 
    {
        using namespace ImGui;
        static const float width = 256.0f * 1.25f;  // ��������ڵĿ�ȣ�������1600x900�ֱ���
        static const float height = 612.0f * 1.25f;  // ��������ڵĸ߶�

        SetNextWindowPos(ImVec2(Window::mWidth - width, (Window::mHeight - height) * 0.5f));  // ���ô���λ��
        SetNextWindowSize(ImVec2(width, height));  // ���ô��ڴ�С

        static ImGuiWindowFlags inspectorFlags = ImGuiWindowFlags_NoMove |  // ��������ڵı�־����ֹ�ƶ���������С���۵�
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

        PushID("Inspector Window");  // ���ͼ�������ڵ�ID

        if (Begin(ICON_FK_LOCATION_ARROW " �����", 0, inspectorFlags)) {  // �������������
            return true;  // ������ڴ����ɹ�������true
        }

        LOG_ERROR("���ڲü����⣬���ؼ����ʧ��...");
        LOG_ERROR("���Ƿ������ȫ����͸�����ڣ�");
        return false;
    }
    void UI::EndInspector(void)
    {
        ImGui::End();  // ����ImGui����
        ImGui::PopID();  // ������������ڵ�ID
    }
    void UI::DrawGizmo(scene::Entity& camera, scene::Entity& target, Gizmo z)
    {
        static const ImVec2 winPos = ImVec2(0.0f, 50.0f);  // ����λ��
        static const ImVec2 winSize = ImVec2((float)Window::mWidth, (float)Window::mHeight - 82.0f);  // ���ڴ�С

        ImGuizmo::MODE mode = ImGuizmo::MODE::LOCAL;  // Ĭ��GizmoģʽΪ�ֲ�ģʽ
        ImGuizmo::OPERATION operation = ImGuizmo::OPERATION::TRANSLATE;  // Ĭ��Gizmo����Ϊƽ��

        switch (z) {  // ���ݴ����Gizmo�������ò���
        case Gizmo::Translate: {
            operation = ImGuizmo::OPERATION::TRANSLATE;  // ���ò���Ϊƽ��
            break;
        }
        case Gizmo::Rotate: {
            operation = ImGuizmo::OPERATION::ROTATE;  // ���ò���Ϊ��ת
            break;
        }
        case Gizmo::Scale: {
            operation = ImGuizmo::OPERATION::SCALE;  // ���ò���Ϊ����
            break;
        }
        case Gizmo::Bounds: case Gizmo::None: default: {
            return;  // �����������ͣ������в�����ֱ�ӷ���
        }
        }

        auto& T = target.GetComponent<Transform>();  // ��ȡĿ��ʵ���Transform���
        auto& C = camera.GetComponent<Camera>();  // ��ȡ���ʵ���Camera���
        glm::mat4 V = C.GetViewMatrix();  // ��ȡ�������ͼ����
        glm::mat4 P = C.GetProjectionMatrix();  // ��ȡ�����ͶӰ����

        // ת��ģ�;���Ϊ��������ϵ����ΪImGuizmo����ʹ����������ϵ
        static const glm::vec3 RvL = glm::vec3(1.0f, 1.0f, -1.0f);  // ���ֵ����ֵ���������
        glm::mat4 transform = glm::scale(T.mTransform, RvL);  // Ӧ������

        ImGui::SetNextWindowPos(winPos);  // ������һ�����ڵ�λ��
        ImGui::SetNextWindowSize(winSize);  // ������һ�����ڵĴ�С
        ImGui::Begin("##Invisible Gizmo Window", 0, gInvisibleWindowFlags);  // ����һ�����ɼ��������ڻ���Gizmo

        ImGuizmo::SetOrthographic(true);  // ����GizmoΪ����ģʽ
        ImGuizmo::SetDrawlist();  // ���û�ͼ�б�
        ImGuizmo::SetRect(winPos.x, winPos.y, winSize.x, winSize.y);  // ����Gizmo��������
        ImGuizmo::Manipulate(value_ptr(V), value_ptr(P), operation, mode, value_ptr(transform));  // ����Gizmo

        // ���Gizmo���ڱ�ʹ�ã������Ŀ��ʵ���Transform���
        if (ImGuizmo::IsUsing()) {
            transform = glm::scale(transform, RvL);  // ת������������ϵ
            T.SetTransform(transform);  // ����Transform���
        }

        ImGui::End();  // ����ImGui����
    }

    void UI::DrawRainbowBar(const ImVec2& offset, float height)
    {
        // ���Ƹ����߶ȵĲʺ���
        // �ʺ����Ŀ�Ȼ��Զ������Ծ�����ʾ�ڴ�����
        // ƫ����������ڴ������Ͻǵ�����

        // �ú���������޸���unknown cheats��̳
        // ��Դ��https://www.unknowncheats.me/forum/2550901-post1.html

        float speed = 0.0006f;  // �ʺ�����ɫ�仯���ٶ�
        static float static_hue = 0.0f;  // ��̬ɫ��ֵ

        ImDrawList* drawList = ImGui::GetWindowDrawList();  // ��ȡ���ڻ�ͼ�б�
        ImVec2 pos = ImGui::GetWindowPos() + offset;  // ����ʺ�����λ��
        float width = ImGui::GetWindowWidth() - offset.x * 2.0f;  // ����ʺ����Ŀ��

        static_hue -= speed;  // ���¾�̬ɫ��ֵ
        if (static_hue < -1.0f) {
            static_hue += 1.0f;  // ȷ��ɫ��ֵ��0��1֮��ѭ��
        }

        for (int i = 0; i < width; i++) {  // �����ʺ����Ŀ��
            float hue = static_hue + (1.0f / width) * i;  // ���㵱ǰ���ص�ɫ��ֵ
            if (hue < 0.0f) hue += 1.0f;  // ȷ��ɫ��ֵ��0��1֮��
            ImColor color = ImColor::HSV(hue, 1.0f, 1.0f);  // ������ɫ
            drawList->AddRectFilled(ImVec2(pos.x + i, pos.y), ImVec2(pos.x + i + 1, pos.y + height), color);  // ���Ʋʺ�����һ������
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
        if (Window::mLayer == Layer::Scene) {  // �����ǰ���ڲ��ǳ�����
            return Input::GetCursorPosition();  // ��������Ĺ��λ��
        }

        auto mousePos = ImGui::GetMousePos();  // ��ȡImGui�����λ��
        return glm::ivec2(mousePos.x, mousePos.y);  // �������λ��
    }
    void UI::SetFonts()
    {
        // main:����sub:�Ρ�icon:ͼ��
        std::string fontMain = "C:\\Windows\\Fonts\\simkai.ttf";
        std::string fontSub = "C:\\Windows\\Fonts\\simkai.ttf";
        std::string fontIcon = gFontPath + FONT_ICON_FILE_NAME_FK;
        LOG_ASSERT_FILE(fontMain, fontSub, fontIcon);

        float fontSizeMain = 18.0f;
        float fontSizeIcon = 18.0f;
        float fontSizeSub = 17.0f;

        // ����������
        ImFontConfig configMain;
        configMain.PixelSnapH = true;			// �������ض���,�⽫ʹ������ˮƽ�����϶��뵽���������
        configMain.OversampleH = 4;				// ˮƽ���������������,��Ⱦʱÿ�����صĲ�������Ϊ4
        configMain.OversampleV = 4;				// ��ֱ���������������,��Ⱦʱÿ�����صĲ�������Ϊ4
        configMain.RasterizerMultiply = 1.2f;	// ��դ��������1.0������ǿ�ȣ�1.2��ʹ������΢���
        configMain.GlyphExtraSpacing.x = 0.0;	// �ַ�������࣬0.0��ʾû�ж�����

        // ����������
        ImFontConfig configSub;
        configSub.PixelSnapH = true;
        configSub.OversampleH = 4;
        configSub.OversampleV = 4;
        configSub.RasterizerMultiply = 1.25f;
        configSub.GlyphExtraSpacing.x = 0.0f;

        // ͼ����������
        ImFontConfig configIcon;
        // ���úϲ�ģʽ,�⽫ʹ�¼��ص���������������ϲ����Ӷ�������ͬһ�����弯��ʹ�ö������
        configIcon.MergeMode = true;
        configIcon.PixelSnapH = true;
        configIcon.OversampleV = 4;
        configIcon.OversampleH = 4;
        configIcon.RasterizerMultiply = 1.5f;
        configIcon.GlyphOffset.y = 0.0f;
        // �����ַ������ˮƽ����,�⽫�����������ַ������ˮƽ���ȣ���ȣ���ͨ������ȷ��ͼ��������������Ĵ�Сһ��
        configIcon.GlyphMaxAdvanceX = fontSizeMain;
        configIcon.GlyphMinAdvanceX = fontSizeMain;

        static const ImWchar iconRanges[] = { ICON_MIN_FK,ICON_MAX_FK,0 };	// �ַ���Χ�������β
        ImGuiIO& io = ImGui::GetIO();				// ��ȡImGui����������������
        mFontMain = io.Fonts->AddFontFromFileTTF(fontMain.c_str(), fontSizeMain, &configMain, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
        mFontIcon = io.Fonts->AddFontFromFileTTF(fontIcon.c_str(), fontSizeIcon, &configIcon, iconRanges);
        mFontSub = io.Fonts->AddFontFromFileTTF(fontSub.c_str(), fontSizeSub, &configSub, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());

        // ������������
        unsigned char* pixels;
        int width, height, bytesPerPixel;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height, &bytesPerPixel);
    }
    void UI::SetStyle()
    {
        // �����Զ�����ʽ����
        ImGuiStyle& style = ImGui::GetStyle();	// ��ȡImGUI��ʽ��������
        // ���ñ߿�
        style.WindowBorderSize = 0.0f;			// ����
        style.FrameBorderSize = 1.0;			// ��ܣ��簴���������ȣ�
        style.PopupBorderSize = 1.0f;			// �������ڣ��������˵����Ի���ȣ�
        style.ChildBorderSize = 1.0f;			// �Ӵ��ڣ���Ƕ�״��ڡ�������
        style.TabBorderSize = 0.0f;				// ��ǩҳ����ѡ���
        style.ScrollbarSize = 18.0f;			// �������Ŀ��
        style.GrabMinSize = 10.f;				// �϶��ؼ����绬��ͽ�����������Сץȡ����
        // �����ڱ߾�
        style.WindowPadding = ImVec2(8.0f, 8.0f);
        style.FramePadding = ImVec2(4.0f, 6.0f);
        style.ItemSpacing = ImVec2(10.0f, 10.0f);
        style.ItemInnerSpacing = ImVec2(10.0, 10.0f);
        style.IndentSpacing = 16.0f;			// �����߾�
        // ����Բ�ǰ뾶
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
        style.SelectableTextAlign = ImVec2(0.0f, 0.0f);	// ��ѡ���ı�����
        style.AntiAliasedLines = true;			// ���������
        style.AntiAliasedFill = true;			// ��������
        style.AntiAliasedLinesUseTex = true;			// ʹ������Ŀ��������
    }
    void UI::SetColor()
    {
        // �����Զ�����ɫ
        auto& color = ImGui::GetStyle().Colors;				// ��ȡ��ʽ�е���ɫ���������
        // ���ô��ڡ��Ӵ��ڡ��������ڵı�����ɫ
        color[ImGuiCol_WindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.85f);
        color[ImGuiCol_ChildBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.85f);
        color[ImGuiCol_PopupBg] = ImVec4(0.1f, 0.1f, 0.1f, 0.85f);
        // ����֡��֡��ͣ��֡����ʱ�ı�����ɫ
        color[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.75f);
        color[ImGuiCol_FrameBgHovered] = ImVec4(0.2f, 0.2f, 0.2f, 0.75f);
        color[ImGuiCol_FrameBgActive] = ImVec4(0.3f, 0.3f, 0.3f, 0.75f);
        // ���ñ��⡢���⼤������۵�ʱ�ı���Ԫ��
        color[ImGuiCol_TitleBg] = ImVec4(0.2f, 0.2f, 0.2f, 0.75f);
        color[ImGuiCol_TitleBgActive] = ImVec4(0.0f, 0.3f, 0.0f, 0.9f);
        color[ImGuiCol_TitleBgCollapsed] = ImVec4(0.0f, 0.0f, 0.0f, 0.75f);
        // ���ù�������������ץȡ����������ͣ������������ʱ�ı�����ɫ
        color[ImGuiCol_ScrollbarBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.75f);
        color[ImGuiCol_ScrollbarGrab] = ImVec4(0.2f, 0.2f, 0.2f, 0.9f);
        color[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.3f, 0.3f, 0.3f, 0.9f);
        color[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.4f, 0.4f, 0.4f, 0.9f);
        // ���ù�ѡ��ǡ�������ץȡ������������ʱ��ץȡ��ɫ
        color[ImGuiCol_CheckMark] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
        color[ImGuiCol_SliderGrab] = ImVec4(0.0f, 0.4f, 0.0f, 0.9f);
        color[ImGuiCol_SliderGrabActive] = ImVec4(0.0f, 0.5f, 0.0f, 0.9f);
        // ���ð�ť����ť��ͣ����ť����ʱ�ı�����ɫ
        color[ImGuiCol_Button] = ImVec4(0.0f, 0.3f, 0.0f, 0.9f);
        color[ImGuiCol_ButtonHovered] = ImVec4(0.0f, 0.55f, 0.0f, 0.9f);
        color[ImGuiCol_ButtonActive] = ImVec4(0.0f, 0.5f, 0.0f, 0.9f);
        // ���ñ�ͷ��������ͣ�����⼤��ʱ����ɫ
        color[ImGuiCol_Header] = ImVec4(0.5f, 0.0f, 1.0f, 0.5f);
        color[ImGuiCol_HeaderHovered] = ImVec4(0.5f, 0.0f, 1.0f, 0.8f);
        color[ImGuiCol_HeaderActive] = ImVec4(0.5f, 0.0f, 1.0f, 0.7f);
        // ����ѡ���ѡ���ͣ��ѡ����ѡ�δ�۽���ѡ�δ�۽�������ʱ����ɫ
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
        SetNextWindowSize(ImVec2(1280.0f / 2.82f, 720.0f / 1.6f));      // ���ô��ڴ�С
        if (!Begin("ʹ�÷���", isShow, ImGuiWindowFlags_NoResize)) { 
            End();
            return;
        }
        Spacing();  // ��ӿհ�

        const ImVec4 textColor = ImVec4(0.4f, 0.8f, 0.4f, 1.0f);  // �ı���ɫ
        const char instructions[] = "ֻ����һ�¾��ܷ���";       // ʹ��˵���ı�

        if (TreeNode("����ָ��")) { 
            Spacing();
            Indent(10.0f);                              // ����
            PushStyleColor(ImGuiCol_Text, textColor);   // �����ı���ɫ
            PushTextWrapPos(412.903f);                  // �����ı�����λ��
            TextWrapped(instructions);                  // ��ʾ�������ı�
            PopTextWrapPos();                           // �ָ��ı�����λ��
            PopStyleColor();                            // �ָ��ı���ɫ
            ImGui::Unindent(10.0f);                            // ȡ������
            TreePop();                                  // �رա�����ָ�ϡ��ڵ�
        }

        if (TreeNode("���")) {
            Spacing();
            BulletText("�ƶ������ת�����");
            BulletText("��ס�Ҽ������������š�");
            BulletText("��ס����Ի���ģʽ��ת��");
            Spacing();
            TreePop();
        }

        // ����һ��������ʾ��ɫ�ı���lambda����
        static auto colorText = [](const char* text) {      
            PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            BulletText(text);       // ��ʾ��ɫ�ı�
            PopStyleColor();        // �ָ��ı���ɫ
            SameLine(128.0f, 0.0f); // ��ͬһ����ʾ
        };

        if (TreeNode("����")) {
            Spacing();
            colorText("Enter");
            Text("�л�UI�������/�ء�"); 
            colorText("Escape"); 
            Text("ȷ���˳����ڡ�");
            colorText("WASD");
            Text("���ĸ�ƽ�淽���ƶ������");
            colorText("Space/Z");
            Text("����/�����ƶ������");
            colorText("R");
            Text("�����������ʼλ�á�");
            Spacing();
            TreePop();
        }

        End();  // �������ƴ���
    }
    void UI::DrawAboutWindow(bool* isShow, const char* version)
    {
        using namespace ImGui;
        if (!*isShow) return;
        if (!Begin("����PBR��Ŀ", isShow, ImGuiWindowFlags_AlwaysAutoResize)) {
            End();
            return;
        }

        Text("PBR %s", version);    // ��ʾ����汾
        Separator();                // ���Ʒָ���
        Text("�� LZK ��д�Ŀ�Դ��Ʒ��2024��7�¡�");
        Text("һ�������� OpenGL �н���ͼ�δ���ļ���Ⱦ�⡣");
        Separator();

        static bool sIsShowContactInfo = false;         // �Ƿ���ʾ��ϵ��Ϣ�ı�־
        Checkbox("�����ϵ��", &sIsShowContactInfo);   // ��ʾ��ѡ��

        if (sIsShowContactInfo) {
            SameLine(0.0f, 90.0f);          // ��ͬһ����ʾ
            bool isCopyToClipboard = Button("COPY", ImVec2(48.0f, 0.0f));           // ��ʾ��COPY����ť
            ImVec2 childSize = ImVec2(0, GetTextLineHeightWithSpacing() * 2.2f);    // �Ӵ��ڴ�С
            BeginChildFrame(GetID("Contact"), childSize, ImGuiWindowFlags_NoMove);  // ��ʼ�Ӵ���

            if (isCopyToClipboard) LogToClipboard();    // ������¡�COPY����ť������Ϣ���Ƶ�������
            {
                Text("Email: liuzhikun2022@163.com");  // ��ʾ����
                Text("Github: https://github.com/lzk2022");  // ��ʾGitHub��ַ
            }
            if (isCopyToClipboard) LogFinish();  // ��������

            EndChildFrame();  // �����Ӵ���
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
        if (BeginMenu("��")) {
            for (unsigned int i = 0; i < scene::factory::gkTitles.size(); i++) {	// �������г�������
                std::string title = scene::factory::gkTitles[i];					// ��ǰ��������
                std::ostringstream id;												// ����ID��
                id << " " << std::setfill('0') << std::setw(2) << i;				// ��ʽ��ID
                bool selected = (newTitle == title);								// �ж��Ƿ�Ϊ��ǰѡ��ı���
                if (MenuItem(utils::ToU8(title).c_str(), id.str().c_str(), selected)) {  // ����˵��ѡ��
                    if (!selected) {												// ������ǵ�ǰѡ��ı���
                        newTitle = title;											// �����±���
                    }
                }
            }
            EndMenu();
        }

        if (BeginMenu("ѡ��")) {
            if (BeginMenu("���ڷֱ���")) {
                MenuItem(" 1280 x 720", NULL, false);
                MenuItem(" 1600 x 900", NULL, true);
                MenuItem(" 1920 x 1080", NULL, false);
                MenuItem(" 2560 x 1440", NULL, false);
                EndMenu();
            }
            EndMenu();
        }

        if (BeginMenu("����")) {
            isShowInstruction |= MenuItem("ʹ��˵��", "F1");
            isShowAboutWindow |= MenuItem("����", "F8");
            EndMenu();
        }
    }
    void UI::AddMenuBarInco(const std::string& currTitle,bool& isBackHome)
    {
        using namespace ImGui;
        static bool musicOn = true;            // �����Ƿ���
        // �˵�ͼ����
        SameLine(GetWindowWidth() - 303.0f);                    // ��ͬһ����ʾͼ����
        static const ImVec4 tooltipBgColor = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);  // ������ʾ������ɫ
        if (MenuItem(ICON_FK_HOME)) {                           // ��ҳ ͼ����
            if (utils::ToU8(currTitle) != "��ӭ����") {          // �����ǰ�������ǻ�ӭ��Ļ
                isBackHome = true;                        // ��ʾ���˵�����
            }
        }
        else if (IsItemHovered()) {                             // ��� ��ҳ ͼ�����ͣ
            PushStyleColor(ImGuiCol_PopupBg, tooltipBgColor);   // ���ù�����ʾ������ɫ
            BeginTooltip();                                     // ��ʼ������ʾ
            TextUnformatted("�������˵�");                       // ��ʾ������ʾ�ı�
            EndTooltip();                                       // ����������ʾ
            PopStyleColor();                                    // �ָ�������ɫ
        }

        if (MenuItem(Window::mLayer == Layer::ImGui ? ICON_FK_PICTURE_O : ICON_FK_COFFEE)) {  // ��ͼ�񡱻򡰿��ȡ�ͼ����
            //Input::SetKeyDown(VK_RETURN, true);  // ���»س���
        }
        else if (IsItemHovered()) {  // �����ͼ�񡱻򡰿��ȡ�ͼ�����ͣ
            PushStyleColor(ImGuiCol_PopupBg, tooltipBgColor);  // ���ù�����ʾ������ɫ
            BeginTooltip();  // ��ʼ������ʾ
            TextUnformatted("���ؽ���ģ�� (Enter)");  // ��ʾ������ʾ�ı�
            EndTooltip();  // ����������ʾ
            PopStyleColor();  // �ָ�������ɫ
        }

        if (MenuItem(musicOn ? ICON_FK_VOLUME_UP : ICON_FK_VOLUME_MUTE)) {  // ��������ͼ����
            musicOn = !musicOn;  // �л����ֿ���
        }
        else if (IsItemHovered()) {  // �����������ͼ�����ͣ
            PushStyleColor(ImGuiCol_PopupBg, tooltipBgColor);  
            BeginTooltip();  
            TextUnformatted("���� ��/��"); 
            EndTooltip();  
            PopStyleColor();  
        }

        // ��Դͼ��
        if (MenuItem(ICON_FK_POWER_OFF)) {
            Input::SetKeyDown(KB_ESCAPE, true);
        }
        else if (IsItemHovered()) {
            PushStyleColor(ImGuiCol_PopupBg, tooltipBgColor);
            BeginTooltip();
            TextUnformatted("�˳����� (Esc)");
            EndTooltip();
            PopStyleColor();
        }
    }
    void UI::DrawVerticalLine()
    {
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);  // ���ƴ�ֱ�ָ���
    }
}
