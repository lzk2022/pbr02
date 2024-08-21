#pragma once
#include <string>
#include <ecs/entt.hpp>
#include "../scene/ResourceManager.h"
#include "../asset/Texture.h"
#include <imgui/imgui.h>
#include "../asset/Buffer.h"
#include "../asset/FBO.h"
#include "Entity.h"
#include "../component/Component.h"
#include <glm/gtc/quaternion.hpp>        // GLM：OpenGL数学库 四元数
#include <glm/gtx/quaternion.hpp>        // GLM：OpenGL数学库 四元数 扩展

using namespace asset;
using namespace component;
namespace scene {
class Scene {
	public:
		explicit Scene(const std::string& title);	// 防止构造函数进行隐式转换
		virtual ~Scene();

		virtual void Init(void);

		// 场景渲染回调，可重写
		virtual void OnSceneRender(void);
		// 在ImGui界面进行渲染的回调，可重写
		virtual void OnImGuiRender(void);
		std::string Title() const;

	protected:
		// 添加一个Uniform缓冲对象到映射表中
		void AddUBO(GLuint shaderId);

		// 添加一个帧缓冲对象到映射表中
		void AddFBO(GLuint width, GLuint height);

		// 创建一个实体，并分配给定名称和标签
		Entity CreateEntity(const std::string& name, ETag tag = ETag::Untagged);


	public:
		ResourceManager mResourceManager;		// 资源管理器
		std::map<GLuint, UBO> mUBO;				// uniform缓冲对象的映射表，通过绑定点索引
		std::map<GLuint, FBO> mFBO;				// 帧缓冲对象的映射表，按创建顺序索引

		std::string mTitle;
	
	private:
		entt::registry mRegistry;							// 实体组件注册表
		std::map<entt::entity, std::string> mDirectory;		// 实体与名称映射表
		friend class Renderer;								// 友元类Renderer，可以访问私有成员
		std::unique_ptr<Texture> mTWelcomeScreen;			// 欢迎屏幕纹理资源
		ImTextureID mTWelcomeScreenId;						// 欢迎界面的GUI id
	};

	// 使用glm库中的向量和矩阵类型的别名声明
	using glm::vec2, glm::vec3, glm::vec4;
	using glm::mat2, glm::mat3, glm::mat4, glm::quat;
	using glm::ivec2, glm::ivec3, glm::ivec4;
	using glm::uvec2, glm::uvec3, glm::uvec4;
	using uint = unsigned int;

	namespace world {
		// 世界空间常量（OpenGL使用右手坐标系）
		inline constexpr vec3 origin{ 0.0f };  // 原点位置
		inline constexpr vec3 zero{ 0.0f };  // 零向量
		inline constexpr vec3 unit{ 1.0f };  // 单位向量
		inline constexpr mat4 identity{ 1.0f };  // 单位矩阵
		inline constexpr quat eye{ 1.0f, 0.0f, 0.0f, 0.0f };  // 四元数单位值
		inline constexpr vec3 up{ 0.0f, 1.0f, 0.0f };  // 上方向向量
		inline constexpr vec3 down{ 0.0f,-1.0f, 0.0f };  // 下方向向量
		inline constexpr vec3 forward{ 0.0f, 0.0f,-1.0f };  // 前方向向量
		inline constexpr vec3 backward{ 0.0f, 0.0f, 1.0f };  // 后方向向量
		inline constexpr vec3 left{ -1.0f, 0.0f, 0.0f };  // 左方向向量
		inline constexpr vec3 right{ 1.0f, 0.0f, 0.0f };  // 右方向向量
	}

	namespace color {
		// 一些常用的颜色预设
		inline constexpr vec3 white{ 1.0f };         // 白色
		inline constexpr vec3 black{ 0.0f };         // 黑色
		inline constexpr vec3 red{ 1.0f, 0.0f, 0.0f };  // 红色
		inline constexpr vec3 green{ 0.0f, 1.0f, 0.0f };  // 绿色
		inline constexpr vec3 lime{ 0.5f, 1.0f, 0.0f };  // 青柠色
		inline constexpr vec3 blue{ 0.0f, 0.0f, 1.0f };  // 蓝色
		inline constexpr vec3 cyan{ 0.0f, 1.0f, 1.0f };  // 青色
		inline constexpr vec3 yellow{ 1.0f, 1.0f, 0.0f };  // 黄色
		inline constexpr vec3 orange{ 1.0f, 0.5f, 0.0f };  // 橙色
		inline constexpr vec3 purple{ 0.5f, 0.0f, 1.0f };  // 紫色
	}
}
