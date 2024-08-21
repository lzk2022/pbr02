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
#include <glm/gtc/quaternion.hpp>        // GLM��OpenGL��ѧ�� ��Ԫ��
#include <glm/gtx/quaternion.hpp>        // GLM��OpenGL��ѧ�� ��Ԫ�� ��չ

using namespace asset;
using namespace component;
namespace scene {
class Scene {
	public:
		explicit Scene(const std::string& title);	// ��ֹ���캯��������ʽת��
		virtual ~Scene();

		virtual void Init(void);

		// ������Ⱦ�ص�������д
		virtual void OnSceneRender(void);
		// ��ImGui���������Ⱦ�Ļص�������д
		virtual void OnImGuiRender(void);
		std::string Title() const;

	protected:
		// ���һ��Uniform�������ӳ�����
		void AddUBO(GLuint shaderId);

		// ���һ��֡�������ӳ�����
		void AddFBO(GLuint width, GLuint height);

		// ����һ��ʵ�壬������������ƺͱ�ǩ
		Entity CreateEntity(const std::string& name, ETag tag = ETag::Untagged);


	public:
		ResourceManager mResourceManager;		// ��Դ������
		std::map<GLuint, UBO> mUBO;				// uniform��������ӳ���ͨ���󶨵�����
		std::map<GLuint, FBO> mFBO;				// ֡��������ӳ���������˳������

		std::string mTitle;
	
	private:
		entt::registry mRegistry;							// ʵ�����ע���
		std::map<entt::entity, std::string> mDirectory;		// ʵ��������ӳ���
		friend class Renderer;								// ��Ԫ��Renderer�����Է���˽�г�Ա
		std::unique_ptr<Texture> mTWelcomeScreen;			// ��ӭ��Ļ������Դ
		ImTextureID mTWelcomeScreenId;						// ��ӭ�����GUI id
	};

	// ʹ��glm���е������;������͵ı�������
	using glm::vec2, glm::vec3, glm::vec4;
	using glm::mat2, glm::mat3, glm::mat4, glm::quat;
	using glm::ivec2, glm::ivec3, glm::ivec4;
	using glm::uvec2, glm::uvec3, glm::uvec4;
	using uint = unsigned int;

	namespace world {
		// ����ռ䳣����OpenGLʹ����������ϵ��
		inline constexpr vec3 origin{ 0.0f };  // ԭ��λ��
		inline constexpr vec3 zero{ 0.0f };  // ������
		inline constexpr vec3 unit{ 1.0f };  // ��λ����
		inline constexpr mat4 identity{ 1.0f };  // ��λ����
		inline constexpr quat eye{ 1.0f, 0.0f, 0.0f, 0.0f };  // ��Ԫ����λֵ
		inline constexpr vec3 up{ 0.0f, 1.0f, 0.0f };  // �Ϸ�������
		inline constexpr vec3 down{ 0.0f,-1.0f, 0.0f };  // �·�������
		inline constexpr vec3 forward{ 0.0f, 0.0f,-1.0f };  // ǰ��������
		inline constexpr vec3 backward{ 0.0f, 0.0f, 1.0f };  // ��������
		inline constexpr vec3 left{ -1.0f, 0.0f, 0.0f };  // ��������
		inline constexpr vec3 right{ 1.0f, 0.0f, 0.0f };  // �ҷ�������
	}

	namespace color {
		// һЩ���õ���ɫԤ��
		inline constexpr vec3 white{ 1.0f };         // ��ɫ
		inline constexpr vec3 black{ 0.0f };         // ��ɫ
		inline constexpr vec3 red{ 1.0f, 0.0f, 0.0f };  // ��ɫ
		inline constexpr vec3 green{ 0.0f, 1.0f, 0.0f };  // ��ɫ
		inline constexpr vec3 lime{ 0.5f, 1.0f, 0.0f };  // ����ɫ
		inline constexpr vec3 blue{ 0.0f, 0.0f, 1.0f };  // ��ɫ
		inline constexpr vec3 cyan{ 0.0f, 1.0f, 1.0f };  // ��ɫ
		inline constexpr vec3 yellow{ 1.0f, 1.0f, 0.0f };  // ��ɫ
		inline constexpr vec3 orange{ 1.0f, 0.5f, 0.0f };  // ��ɫ
		inline constexpr vec3 purple{ 0.5f, 0.0f, 1.0f };  // ��ɫ
	}
}
