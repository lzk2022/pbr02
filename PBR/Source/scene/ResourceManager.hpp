//#include "ResourceManager.h"
#include "../utils/Log.h"
namespace scene {
	template<typename T>
	inline void ResourceManager::Add(int key, const std::shared_ptr<T>& resource)
	{
		if (mRegistry.find(key) != mRegistry.end()) {
			LOG_ERROR("重复的键值已存在，无法添加资源...");
			return;
		}
		mRegistry.try_emplace(key, typeid(T));	// 在注册表中就地构造类型索引
		mResources.insert_or_assign(key, std::static_pointer_cast<void>(resource));
	}

	template<typename T>
	std::shared_ptr<T> ResourceManager::Get(int key) const
	{
		// 检查键值是否存在于注册表中
		if (mRegistry.find(key) == mRegistry.end()) {
			LOG_ERROR("无效的资源键值!");
			return nullptr;
		}
		// 检查类型 `T` 是否与注册的类型索引匹配,更新为 C++20 中的 spaceship <=> 操作符
		else if (mRegistry.at(key) != std::type_index(typeid(T))) {
			LOG_ERROR("资源类型不匹配!");
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