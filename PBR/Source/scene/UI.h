#pragma once
#include <string>
#include <imgui/imgui.h>
#include <glm/glm.hpp>
#include "../scene/entity.h"
namespace scene {
	enum class Gizmo : uint8_t {
		None,     // �޲���
		Translate, // ƽ�Ʋ���
		Rotate,    // ��ת����
		Scale,     // ���Ų���
		Bounds     // �߽�����
	};
	class UI {
		
	public:
		static void Init();
		static void DrawWelcomeScreen(ImTextureID id);
		static void DrawMenuBar(std::string& newTitle,const std::string& currTitle);
		static void DrawStatusBar(void);
		static void DrawToolTip(const char* desc, float spacing = 5.0f);
		static void DrawLoadScreen();
		static void DrawTest();
		static void BeginFrame();
		static void EndFrame();

		static bool NewInspector(void);		// ��ʼ�µļ������
		static void EndInspector(void);		// ������ǰ�������

		/********************************************************************************
		* @brief        ���Ʊ任���ߣ�Gizmo��
		*********************************************************************************
		* @param        camera: ���ʵ��
		* @param        target: Ŀ��ʵ��
		* @param        z:      Gizmo ����
		********************************************************************************/
		static void DrawGizmo(scene::Entity& camera, scene::Entity& target, Gizmo z);

		/********************************************************************************
		* @brief        ���Ʋʺ�ɫ��
		*********************************************************************************
		* @param        offset: ƫ����
		* @param        height: �߶�
		********************************************************************************/
		static void DrawRainbowBar(const ImVec2& offset, float height);

		static char* ToU8(const std::string str);

	public:
		static bool BeginTabItemX(const std::string& label);
		static void CheckboxX(const std::string& label, bool& value);

	public:
		static glm::ivec2 GetCursorPosition();
		
	private:
		inline static ImFont* mFontMain;
		inline static ImFont* mFontSub;
		inline static ImFont* mFontIcon;
		inline static ImVec2  mWindowCenter;
		inline static ImVec4 mColorRed =	ImVec4(1.0f, 0.0f, 0.0f, 1.0f);		// ��ɫ
		inline static ImVec4 mColorYellow = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);		// ��ɫ
		inline static ImVec4 mColorGreen =	ImVec4(0.0f, 1.0f, 0.0f, 1.0f);		// ��ɫ
		inline static ImVec4 mColorBlue =	ImVec4(0.0f, 0.0f, 1.0f, 1.0f);		// ��ɫ
		inline static ImVec4 mColorCyan =	ImVec4(0.0f, 1.0f, 1.0f, 1.0f);		// ��ɫ

	private:
		static void SetFonts();
		static void SetStyle();
		static void SetColor();
		static void DrawUsageWindow(bool* isShow);  // ����ʹ��˵������
		static void DrawAboutWindow(bool* isShow, const char* version);
		static int  DrawPopupModel(const char* title, const char* message, const ImVec2& size);
		static void AddMenuBarFont(std::string& newTitle, bool& isShowInstruction, bool& isShowAboutWindow);
		static void AddMenuBarInco(const std::string& currTitle, bool& isBackHome);
		static void DrawVerticalLine();

		
	};
}


