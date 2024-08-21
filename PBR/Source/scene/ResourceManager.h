#pragma once
#include <map>
#include <typeinfo>
#include <typeindex>
#include <memory>
namespace scene {
	class ResourceManager {

	public:
		ResourceManager() {};
		~ResourceManager() {};

		/********************************************************************************
		* @brief		添加资源到管理器中
		*********************************************************************************
		* @param        key:		资源的键值
		* @param		resource:	资源的引用
		********************************************************************************/
		template<typename T>
		void Add(int key, const std::shared_ptr<T>& resource);

		template<typename T>
		std::shared_ptr<T> Get(int key) const;

		void Del(int key);
		void Clear();
	private:
		std::map<int, std::type_index> mRegistry;			// 存储资源类型的注册表
		std::map<int, std::shared_ptr<void>> mResources;	// 存储资源的映射表

	};
}

// 模板文件应该修改文件后缀名，编码阶段为了方便写代码，用cpp后缀
#include "ResourceManager.hpp"		

