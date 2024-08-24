#pragma once
#include <string>
#include "Scene.h"
#include "../asset/Shader.h"
#include <queue>
#include <ECS/entt.hpp>
namespace scene {
class Renderer {
public:
	// ���Ӳ�����һ������
	static void Attach(const std::string& title);

	// ���Ƴ���
	static void DrawScene();
	// ����ImGui����
	static void DrawImGui();
	static void Flush();
	static void Clear();
	static void Detach();
	static void Reset();
	static const Scene* CurrScene();

	static void SetSeamlessCubemap(bool enable);	// ���û�����޷���������ͼ
	static void SetDepthTest(bool enable);
	static void SetFaceCulling(bool enable);
	static void SetAlphaBlend(bool enable);
	static void SetDepthPrepass(bool enable);		// ���û�������Ԥ����
	static void SetFrontFace(bool ccw);				// ����ǰ�泯��˳ʱ�����ʱ��
	static void SetMSAA(bool enable);				// ���û���ö��ز��������
	static void PrimitiveRestart(bool enable);

	static void Render(const std::shared_ptr<Shader> shader = nullptr);
	// ����Ⱦ�����ύ����ʵ��ID
	template<typename... Args>
	static void Submit(Args&&... args) {
		(mRenderQueue.push(args), ...);  // ��ÿ������������Ⱦ����
	}
private:
	inline static Scene* mLastScene = nullptr;		// ָ����һ��������ָ��
	inline static Scene* mCurrScene = nullptr;		// ָ��ǰ������ָ��
	inline static std::queue<entt::entity> mRenderQueue{};  // ��Ⱦ���У��洢Ҫ��Ⱦ��ʵ��
};
}

