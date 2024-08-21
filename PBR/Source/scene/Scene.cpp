#include "Scene.h"
#include "../utils/Log.h"
#include "../utils/Global.h"
#include "Renderer.h"
#include "UI.h"

namespace scene {
	Scene::Scene(const std::string& title)
	{
		LOG_TRACK;
		mResourceManager = ResourceManager();
	}
	Scene::~Scene()
	{
	}
	void Scene::Init(void)
	{
		LOG_TRACK;
		mTWelcomeScreen = std::make_unique<Texture>("texture\\common\\welcome.png", 1);
		mTWelcomeScreenId = (void*)(intptr_t)(mTWelcomeScreen->getId());
	}

	void Scene::OnSceneRender(void)
	{
		LOG_TRACK;
		Renderer::Clear();
	}
	void Scene::OnImGuiRender(void)
	{
		LOG_TRACK;
		UI::DrawWelcomeScreen(mTWelcomeScreenId);
	}
	std::string Scene::Title() const
	{
		return mTitle;
	}
	void Scene::AddUBO(GLuint shaderId)
	{
		LOG_TRACK;
		const GLenum props[] = { GL_BUFFER_BINDING };
		GLint n_blocks = 0;
		glGetProgramInterfaceiv(shaderId, GL_UNIFORM_BLOCK, GL_ACTIVE_RESOURCES, &n_blocks);

		for (int idx = 0; idx < n_blocks; ++idx) {
			GLint binding_index = 0;
			glGetProgramResourceiv(shaderId, GL_UNIFORM_BLOCK, idx, 1, props, 1, NULL, &binding_index);
			GLuint binding_point = static_cast<GLuint>(binding_index);

			// ͳһ�������UBO���İ󶨵���ڵ���10���������ڲ�ʹ��
			if (binding_point < 10) {
				mUBO.try_emplace(binding_point, shaderId, idx);  // �͵ع���UBO
			}
		}
	}
	void Scene::AddFBO(GLuint width, GLuint height){
		LOG_TRACK;
		GLuint key = mFBO.size();
		mFBO.try_emplace(key, width, height);  // �͵ع���FBO
	}
	Entity Scene::CreateEntity(const std::string& name, ETag tag)
	{
		LOG_TRACK;
		// ÿ��ʵ�嶼��һ���任����ͱ�ǩ���
		Entity entity = { name,mRegistry.create(),&mRegistry };
		entity.AddComponent<Transform>();
		entity.AddComponent<Tag>(tag);
		mDirectory.emplace(entity.mId, entity.mName);	// ��ʵ����ӵ�Ŀ¼��
		return entity;
	}
}
