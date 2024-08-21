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
		* @brief		�����Դ����������
		*********************************************************************************
		* @param        key:		��Դ�ļ�ֵ
		* @param		resource:	��Դ������
		********************************************************************************/
		template<typename T>
		void Add(int key, const std::shared_ptr<T>& resource);

		template<typename T>
		std::shared_ptr<T> Get(int key) const;

		void Del(int key);
		void Clear();
	private:
		std::map<int, std::type_index> mRegistry;			// �洢��Դ���͵�ע���
		std::map<int, std::shared_ptr<void>> mResources;	// �洢��Դ��ӳ���

	};
}

// ģ���ļ�Ӧ���޸��ļ���׺��������׶�Ϊ�˷���д���룬��cpp��׺
#include "ResourceManager.hpp"		

