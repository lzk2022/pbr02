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
    // 表示一个Uniform变量，用于管理一个GLSL uniform
    template<typename T>
    class Uniform {
    public:
        std::string mName;      // uniform的名称
        GLuint mLocation;       // uniform在shader中的位置
        GLuint mOwnerId;        // 拥有该uniform的shader的ID
        GLuint mSize;           // uniform的大小，如果不是数组则为1

    private:
        T mValue;               // uniform的值
        const T* mpValue;     // uniform的值的指针
        const std::vector<T>* mpArray;                // uniform数组的指针
        void Upload(T val, GLuint index = 0) const;     // 将值上传到shader中

    public:
        Uniform() = default;
        Uniform(GLuint ownerId, GLuint location, const char* name);

        void operator<<(const T& value);                    // 设置uniform的值
        void operator<<=(const T* pValue);                  // 绑定一个指针所指向的值到uniform
        void operator<<=(const std::vector<T>* pArray);     // 设置一个uniform数组的值
        void Upload() const;                                // 将uniform的值上传到shader中
    };

    // 表示GLSL uniform的枚举类型
    enum class pbr_u : uint16_t {
        albedo = 912U,          // 漫反射颜色
        roughness = 913U,       // 粗糙度
        ao = 914U,              // 环境光遮蔽
        emission = 915U,        // 自发光
        uv_scale = 916U,        // UV缩放
        alpha_mask = 928U,      // Alpha遮罩
        metalness = 917U,       // 金属度
        specular = 918U,        // 高光度
        anisotropy = 919U,      // 各向异性
        aniso_dir = 920U,       // 各向异性方向
        transmission = 921U,    // 透射率
        thickness = 922U,       // 厚度
        ior = 923U,             // 折射率
        transmittance = 924U,   // 透射颜色
        tr_distance = 925U,     // 透射距离
        volume_type = 931U,     // 体积类型
        sheen_color = 926U,     // 闪光颜色
        subsurf_color = 927U,   // 次表面颜色
        clearcoat = 929U,       // 清漆
        cc_roughness = 930U,    // 清漆粗糙度
        shading_model = 999U    // 着色模型
    };

    // 表示材质纹理属性的枚举类型
    enum class pbr_t : uint8_t {
        irradiance_map = 17U,   // 辐照度贴图
        prefiltered_map = 18U,  // 预过滤贴图
        BRDF_LUT = 19U,         // BRDF查找表
        albedo = 20U,           // 漫反射颜色贴图
        normal = 21U,           // 法线贴图
        metallic = 22U,         // 金属度贴图
        roughness = 23U,        // 粗糙度贴图
        ao = 24U,               // 环境光遮蔽贴图
        emission = 25U,         // 自发光贴图
        displace = 26U,         // 位移贴图
        opacity = 27U,          // 不透明度贴图
        lightmap = 28U,         // 光照贴图
        anisotropic = 29U       // 各向异性贴图
    };

}
#include "Uniform.hpp"

