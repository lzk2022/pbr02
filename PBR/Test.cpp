#include <iostream>
#include <unordered_map>
#include <variant>
#include <cassert>
#include <glad/glad.h>

// 假设的日志断言宏
#define LOG_ASSERT_TRUE(condition, message) assert((condition) && (message))

// Uniform 类模板
template<typename T>
class Uniform {
public:
    Uniform() : mValue{} {}
    Uniform(const T& value) : mValue(value) {}

    Uniform& operator<<(const T& value) {
        mValue = value;
        return *this;
    }

    const T& getValue() const {
        return mValue;
    }

private:
    T mValue;
};

// Material 类
class Material {
public:
    // 使用 std::variant 存储不同类型的 Uniform
    using UniformVariant = std::variant<Uniform<int>, Uniform<float>>;

    void SetUniform(GLuint location, const int& value) {
        if (mUniforms.count(location) > 0) {
            auto& unifVariant = mUniforms[location];
            LOG_ASSERT_TRUE(std::holds_alternative<Uniform<int>>(unifVariant), "不匹配的 uniform 类型！");
            std::get<Uniform<int>>(unifVariant) << value;
        }
    }

    void SetUniform(GLuint location, const float& value) {
        if (mUniforms.count(location) > 0) {
            auto& unifVariant = mUniforms[location];
            LOG_ASSERT_TRUE(std::holds_alternative<Uniform<float>>(unifVariant), "不匹配的 uniform 类型！");
            std::get<Uniform<float>>(unifVariant) << value;
        }
    }

    // 设置 uniform 值的辅助函数
    void AddUniform(GLuint location, const UniformVariant& uniform) {
        mUniforms[location] = uniform;
    }

    void PrintUniform(GLuint location) {
        if (mUniforms.count(location) > 0) {
            auto& unifVariant = mUniforms[location];
            if (std::holds_alternative<Uniform<int>>(unifVariant)) {
                std::cout << "Uniform int value: " << std::get<Uniform<int>>(unifVariant).getValue() << std::endl;
            }
            else if (std::holds_alternative<Uniform<float>>(unifVariant)) {
                std::cout << "Uniform float value: " << std::get<Uniform<float>>(unifVariant).getValue() << std::endl;
            }
        }
        else {
            std::cout << "Uniform not found!" << std::endl;
        }
    }

private:
    std::unordered_map<GLuint, UniformVariant> mUniforms;
};

int main1() {
    Material mat;

    // 添加 uniform
    mat.AddUniform(1, Uniform<int>(42));
    mat.AddUniform(2, Uniform<float>(3.14f));

    // 测试 SetUniform 方法
    mat.SetUniform(2, 84);
    mat.SetUniform(1, 6.28f);

    // 打印 uniform 值
    mat.PrintUniform(1); // 应该输出: Uniform int value: 84
    mat.PrintUniform(2); // 应该输出: Uniform float value: 6.28

    return 0;
}
/* -------------------------------------------------------------------------------------------
 * 作者称呼：LZK
 * 电子邮箱：liuzhikun2022@163.com
 * 创建时间：2024-07-02
 * 功能描述：
 * 
 *
 * ------------------------------------------------------------------------------------------- */
