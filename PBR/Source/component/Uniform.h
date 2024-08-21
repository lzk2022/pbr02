#pragma once
#include <map>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>
#include <memory>
#include <glad/glad.h>
#include <glm/glm.hpp>
namespace component {
    using namespace glm;
    // ��ʾһ��Uniform���������ڹ���һ��GLSL uniform
    template<typename T>
    class Uniform {
    public:
        std::string mName;      // uniform������
        GLuint mLocation;       // uniform��shader�е�λ��
        GLuint mOwnerId;        // ӵ�и�uniform��shader��ID
        GLuint mSize;           // uniform�Ĵ�С���������������Ϊ1

    private:
        T mValue;               // uniform��ֵ
        const T* mpValue;     // uniform��ֵ��ָ��
        const std::vector<T>* mpArray;                // uniform�����ָ��
        void Upload(T val, GLuint index = 0) const;     // ��ֵ�ϴ���shader��

    public:
        Uniform() = default;
        Uniform(GLuint ownerId, GLuint location, const char* name);

        void operator<<(const T& value);                    // ����uniform��ֵ
        void operator<<=(const T* pValue);                  // ��һ��ָ����ָ���ֵ��uniform
        void operator<<=(const std::vector<T>* pArray);     // ����һ��uniform�����ֵ
        void Upload() const;                                // ��uniform��ֵ�ϴ���shader��
    };

    // ��ʾGLSL uniform��ö������
    enum class pbr_u : uint16_t {
        albedo = 912U,          // ��������ɫ
        roughness = 913U,       // �ֲڶ�
        ao = 914U,              // �������ڱ�
        emission = 915U,        // �Է���
        uv_scale = 916U,        // UV����
        alpha_mask = 928U,      // Alpha����
        metalness = 917U,       // ������
        specular = 918U,        // �߹��
        anisotropy = 919U,      // ��������
        aniso_dir = 920U,       // �������Է���
        transmission = 921U,    // ͸����
        thickness = 922U,       // ���
        ior = 923U,             // ������
        transmittance = 924U,   // ͸����ɫ
        tr_distance = 925U,     // ͸�����
        volume_type = 931U,     // �������
        sheen_color = 926U,     // ������ɫ
        subsurf_color = 927U,   // �α�����ɫ
        clearcoat = 929U,       // ����
        cc_roughness = 930U,    // ����ֲڶ�
        shading_model = 999U    // ��ɫģ��
    };

    // ��ʾ�����������Ե�ö������
    enum class pbr_t : uint8_t {
        irradiance_map = 17U,   // ���ն���ͼ
        prefiltered_map = 18U,  // Ԥ������ͼ
        BRDF_LUT = 19U,         // BRDF���ұ�
        albedo = 20U,           // ��������ɫ��ͼ
        normal = 21U,           // ������ͼ
        metallic = 22U,         // ��������ͼ
        roughness = 23U,        // �ֲڶ���ͼ
        ao = 24U,               // �������ڱ���ͼ
        emission = 25U,         // �Է�����ͼ
        displace = 26U,         // λ����ͼ
        opacity = 27U,          // ��͸������ͼ
        lightmap = 28U,         // ������ͼ
        anisotropic = 29U       // ����������ͼ
    };

}
#include "Uniform.hpp"

