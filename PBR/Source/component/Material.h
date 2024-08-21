/*
   a material component glues together a shader and its associated textures and uniforms,
   it's designed to ease the preparation setup for rendering, which internally automates
   the tasks of uniform uploads, smart shader binding and smart texture bindings.

   the usage of this class is very similar to Unity's material system, despite that our
   implementation is much simplified. In our physically-based rendering pipeline, a PBR
   shader is often shared by multiple entities in the scene, so that we don't create 100
   shader programs with the exact same code for 100 different meshes. It's then the duty
   of the material component to identify a particular entity's shading inputs, it will
   remember all the uniform values and textures of every individual entity.
*/

/*
   材质组件将着色器及其关联的纹理和统一变量粘合在一起，
   旨在简化渲染准备设置，内部自动化统一变量上传、智能着色器绑定和智能纹理绑定任务。

   这个类的使用方式与Unity的材质系统非常相似，尽管我们的实现要简化得多。
   在我们的物理渲染管线中，PBR着色器通常由场景中多个实体共享，这样我们就不会为100个不同的网格创建100个完全相同的着色器程序。
   因此，材质组件的职责是识别特定实体的着色输入，它将记住每个单独实体的所有统一变量值和纹理。
*/


#pragma once

#include <map>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>
#include <memory>
#include <glad/glad.h>

#include "../asset/Shader.h"
#include "../asset/Texture.h"
#include "../component/Component.h"
#include "../component/Uniform.h"

namespace component {
    using namespace asset;
    // 表示材质的类，继承自Component
    class Material : public Component {
    private:
        using uniform_variant = std::variant<
            Uniform<int>, Uniform<uint>, Uniform<bool>, Uniform<float>,
            Uniform<vec2>, Uniform<vec3>, Uniform<vec4>, Uniform<uvec2>, Uniform<uvec3>, Uniform<uvec4>,
            Uniform<mat2>, Uniform<mat3>, Uniform<mat4>, Uniform<ivec2>, Uniform<ivec3>, Uniform<ivec4>
        >;
        static_assert(std::variant_size_v<uniform_variant> == 16);  // 断言确保variant包含16种类型

    private:
        std::shared_ptr<Shader> mShader;                        // 材质使用的Shader引用
        std::map<GLuint, uniform_variant> mUniforms;            // 纹理uniform的映射表
        std::map<GLuint, std::shared_ptr<Texture>> mTextures;   // 纹理单元与纹理引用的映射表

    public:
        Material(const std::shared_ptr<Shader>& shader);    
        Material(const std::shared_ptr<Material>& material);  

        void Bind() const;    
        void Unbind() const;   

        void SetShader(std::shared_ptr<Shader> shader);                         
        void SetTexture(GLuint unit, std::shared_ptr<Texture> texture);         
        void SetTexture(pbr_t attribute, std::shared_ptr<Texture> texture);     

        // 设置单个uniform的值
        template<typename T, typename = is_glsl_t<T>>
        void SetUniform(GLuint location, const T& value);       

        // 根据属性设置单个uniform的值
        template<typename T, typename = is_glsl_t<T>>
        void SetUniform(pbr_u attribute, const T& value);       

        // 绑定一个指针所指向的值到一个uniform
        template<typename T, typename = is_glsl_t<T>>
        void BindUniform(GLuint location, const T* pValue);     

        // 根据属性绑定一个指针所指向的值到一个uniform
        template<typename T, typename = is_glsl_t<T>>
        void BindUniform(pbr_u attribute, const T* pValue);     

        // 设置一个uniform数组的值
        template<typename T, typename = is_glsl_t<T>>
        void SetUniformArray(GLuint location, GLuint size, const std::vector<T>* pArray);   
    };

}  // namespace component

#include "Material.hpp"

