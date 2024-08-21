#pragma once
#include <algorithm>
#include <iterator>
#include <type_traits>
#include <string>
#include <GLM/gtc/type_ptr.hpp>
namespace utils {
	// ������������������ģ��������͵�����ɶ���ʽ
	// ����ֵ�����������ַ�����ͼ
	template<typename T>
	constexpr auto type_name() noexcept {
		std::string_view p = __FUNCSIG__;
		return std::string_view(p.data() + 84, p.size() - 84 - 7);
	}

	// STL�����ķ�Χ�����װ����C++20�н�ʹ��ranges�⣩
	namespace ranges {
		// ��������ÿ��Ԫ��Ӧ�ú���
		template<typename Container, typename Func>
		Func for_each(Container& range, Func function) {
			return std::for_each(begin(range), end(range), function);
		}

		// �������в���ֵ
		template<typename Container, typename T>
		typename Container::const_iterator find(const Container& c, const T& value) {
			return std::find(c.begin(), c.end(), value);
		}

		// �������в�������������Ԫ��
		template<typename Container, typename Pred>
		typename Container::const_iterator find_if(const Container& c, Pred predicate) {
			return std::find_if(c.begin(), c.end(), predicate);
		}

		// ͳ������������Ԫ�ظ���
		template<typename Container, typename Pred>
		typename std::iterator_traits<typename Container::const_iterator>::difference_type
			count_if(const Container& c, Pred predicate) {
			return std::count_if(c.begin(), c.end(), predicate);
		}

		// ������������Ԫ�����Ϊָ��ֵ
		template<typename Container, typename T>
		constexpr void fill(const Container& c, const T& value) {
			std::fill(c.begin(), c.end(), value);
		}
	}

	std::string ToU8(const std::string str);

	// ���κ�ö����ת��Ϊ�������������
	template<typename EnumClass>
	constexpr auto ToIntegral(EnumClass e) {
		using T = std::underlying_type_t<EnumClass>;
		return static_cast<T>(e);
	}

	// ��ȡ�����������͵�const void*����ָ��
	template<typename T>
	const T* GetValPtr(const T& val) {
		return &val;
	}

	template <typename... Args>
	auto GetValPtr(Args&&... args) -> decltype(glm::value_ptr(std::forward<Args>(args)...)) {
		return glm::value_ptr(std::forward<Args>(args)...);
	}
}
