#pragma once
#include <algorithm>
#include <iterator>
#include <type_traits>
#include <string>
#include <GLM/gtc/type_ptr.hpp>
namespace utils {
	// 返回类型名，适用于模板参数类型的人类可读格式
	// 返回值：类型名的字符串视图
	template<typename T>
	constexpr auto type_name() noexcept {
		std::string_view p = __FUNCSIG__;
		return std::string_view(p.data() + 84, p.size() - 84 - 7);
	}

	// STL容器的范围语义封装（在C++20中将使用ranges库）
	namespace ranges {
		// 对容器的每个元素应用函数
		template<typename Container, typename Func>
		Func for_each(Container& range, Func function) {
			return std::for_each(begin(range), end(range), function);
		}

		// 在容器中查找值
		template<typename Container, typename T>
		typename Container::const_iterator find(const Container& c, const T& value) {
			return std::find(c.begin(), c.end(), value);
		}

		// 在容器中查找满足条件的元素
		template<typename Container, typename Pred>
		typename Container::const_iterator find_if(const Container& c, Pred predicate) {
			return std::find_if(c.begin(), c.end(), predicate);
		}

		// 统计满足条件的元素个数
		template<typename Container, typename Pred>
		typename std::iterator_traits<typename Container::const_iterator>::difference_type
			count_if(const Container& c, Pred predicate) {
			return std::count_if(c.begin(), c.end(), predicate);
		}

		// 将容器的所有元素填充为指定值
		template<typename Container, typename T>
		constexpr void fill(const Container& c, const T& value) {
			std::fill(c.begin(), c.end(), value);
		}
	}

	std::string ToU8(const std::string str);

	// 将任何枚举类转换为其基础整数类型
	template<typename EnumClass>
	constexpr auto ToIntegral(EnumClass e) {
		using T = std::underlying_type_t<EnumClass>;
		return static_cast<T>(e);
	}

	// 获取基本标量类型的const void*数据指针
	template<typename T>
	const T* GetValPtr(const T& val) {
		return &val;
	}

	template <typename... Args>
	auto GetValPtr(Args&&... args) -> decltype(glm::value_ptr(std::forward<Args>(args)...)) {
		return glm::value_ptr(std::forward<Args>(args)...);
	}
}
