#include <iostream>
#include <unordered_map>
#include <variant>
#include <cassert>
#include <glad/glad.h>

// �������־���Ժ�
#define LOG_ASSERT_TRUE(condition, message) assert((condition) && (message))

// Uniform ��ģ��
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

// Material ��
class Material {
public:
    // ʹ�� std::variant �洢��ͬ���͵� Uniform
    using UniformVariant = std::variant<Uniform<int>, Uniform<float>>;

    void SetUniform(GLuint location, const int& value) {
        if (mUniforms.count(location) > 0) {
            auto& unifVariant = mUniforms[location];
            LOG_ASSERT_TRUE(std::holds_alternative<Uniform<int>>(unifVariant), "��ƥ��� uniform ���ͣ�");
            std::get<Uniform<int>>(unifVariant) << value;
        }
    }

    void SetUniform(GLuint location, const float& value) {
        if (mUniforms.count(location) > 0) {
            auto& unifVariant = mUniforms[location];
            LOG_ASSERT_TRUE(std::holds_alternative<Uniform<float>>(unifVariant), "��ƥ��� uniform ���ͣ�");
            std::get<Uniform<float>>(unifVariant) << value;
        }
    }

    // ���� uniform ֵ�ĸ�������
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

    // ��� uniform
    mat.AddUniform(1, Uniform<int>(42));
    mat.AddUniform(2, Uniform<float>(3.14f));

    // ���� SetUniform ����
    mat.SetUniform(2, 84);
    mat.SetUniform(1, 6.28f);

    // ��ӡ uniform ֵ
    mat.PrintUniform(1); // Ӧ�����: Uniform int value: 84
    mat.PrintUniform(2); // Ӧ�����: Uniform float value: 6.28

    return 0;
}
/* -------------------------------------------------------------------------------------------
 * ���߳ƺ���LZK
 * �������䣺liuzhikun2022@163.com
 * ����ʱ�䣺2024-07-02
 * ����������
 * 
 *
 * ------------------------------------------------------------------------------------------- */
