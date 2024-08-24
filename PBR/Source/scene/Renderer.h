#pragma once
#include <string>
#include "Scene.h"
#include "../asset/Shader.h"
#include <queue>
#include <ECS/entt.hpp>
namespace scene {
class Renderer {
public:
	// 连接并加载一个场景
	static void Attach(const std::string& title);

	// 绘制场景
	static void DrawScene();
	// 绘制ImGui界面
	static void DrawImGui();
	static void Flush();
	static void Clear();
	static void Detach();
	static void Reset();
	static const Scene* CurrScene();

	static void SetSeamlessCubemap(bool enable);	// 启用或禁用无缝立方体贴图
	static void SetDepthTest(bool enable);
	static void SetFaceCulling(bool enable);
	static void SetAlphaBlend(bool enable);
	static void SetDepthPrepass(bool enable);		// 启用或禁用深度预处理
	static void SetFrontFace(bool ccw);				// 设置前面朝向，顺时针或逆时针
	static void SetMSAA(bool enable);				// 启用或禁用多重采样抗锯齿
	static void PrimitiveRestart(bool enable);

	static void Render(const std::shared_ptr<Shader> shader = nullptr);
	// 向渲染队列提交若干实体ID
	template<typename... Args>
	static void Submit(Args&&... args) {
		(mRenderQueue.push(args), ...);  // 将每个参数推入渲染队列
	}
private:
	inline static Scene* mLastScene = nullptr;		// 指向上一个场景的指针
	inline static Scene* mCurrScene = nullptr;		// 指向当前场景的指针
	inline static std::queue<entt::entity> mRenderQueue{};  // 渲染队列，存储要渲染的实体
};
}

