//#include "ResourceManager.h"
#include "../utils/Log.h"
namespace scene {
	template<typename T>
	inline void ResourceManager::Add(int key, const std::shared_ptr<T>& resource)
	{
		if (mRegistry.find(key) != mRegistry.end()) {
			LOG_ERROR("�ظ��ļ�ֵ�Ѵ��ڣ��޷������Դ...");
			return;
		}
		mRegistry.try_emplace(key, typeid(T));	// ��ע����о͵ع�����������
		mResources.insert_or_assign(key, std::static_pointer_cast<void>(resource));
	}

	template<typename T>
	std::shared_ptr<T> ResourceManager::Get(int key) const
	{
		// ����ֵ�Ƿ������ע�����
		if (mRegistry.find(key) == mRegistry.end()) {
			LOG_ERROR("��Ч����Դ��ֵ!");
			return nullptr;
		}
		// ������� `T` �Ƿ���ע�����������ƥ��,����Ϊ C++20 �е� spaceship <=> ������
		else if (mRegistry.at(key) != std::type_index(typeid(T))) {
			LOG_ERROR("��Դ���Ͳ�ƥ��!");
			return nullptr;
		}
		return std::static_pointer_cast<T>(mResources.at(key));
	}

	inline void ResourceManager::Del(int key)
	{
		if (mRegistry.find(key) != mRegistry.end()) {
			mRegistry.erase(key);
			mResources.erase(key);
		}
	}
	inline void ResourceManager::Clear()
	{
		mRegistry.clear();
		mResources.clear();
	}
}