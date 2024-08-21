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
   �����������ɫ����������������ͳһ����ճ����һ��
   ּ�ڼ���Ⱦ׼�����ã��ڲ��Զ���ͳһ�����ϴ���������ɫ���󶨺��������������

   ������ʹ�÷�ʽ��Unity�Ĳ���ϵͳ�ǳ����ƣ��������ǵ�ʵ��Ҫ�򻯵öࡣ
   �����ǵ�������Ⱦ�����У�PBR��ɫ��ͨ���ɳ����ж��ʵ�干���������ǾͲ���Ϊ100����ͬ�����񴴽�100����ȫ��ͬ����ɫ������
   ��ˣ����������ְ����ʶ���ض�ʵ�����ɫ���룬������סÿ������ʵ�������ͳһ����ֵ������
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
    // ��ʾ���ʵ��࣬�̳���Component
    class Material : public Component {
    private:
        using uniform_variant = std::variant<
            Uniform<int>, Uniform<uint>, Uniform<bool>, Uniform<float>,
            Uniform<vec2>, Uniform<vec3>, Uniform<vec4>, Uniform<uvec2>, Uniform<uvec3>, Uniform<uvec4>,
            Uniform<mat2>, Uniform<mat3>, Uniform<mat4>, Uniform<ivec2>, Uniform<ivec3>, Uniform<ivec4>
        >;
        static_assert(std::variant_size_v<uniform_variant> == 16);  // ����ȷ��variant����16������

    private:
        std::shared_ptr<Shader> mShader;                        // ����ʹ�õ�Shader����
        std::map<GLuint, uniform_variant> mUniforms;            // ����uniform��ӳ���
        std::map<GLuint, std::shared_ptr<Texture>> mTextures;   // ����Ԫ���������õ�ӳ���

    public:
        Material(const std::shared_ptr<Shader>& shader);    
        Material(const std::shared_ptr<Material>& material);  

        void Bind() const;    
        void Unbind() const;   

        void SetShader(std::shared_ptr<Shader> shader);                         
        void SetTexture(GLuint unit, std::shared_ptr<Texture> texture);         
        void SetTexture(pbr_t attribute, std::shared_ptr<Texture> texture);     

        // ���õ���uniform��ֵ
        template<typename T, typename = is_glsl_t<T>>
        void SetUniform(GLuint location, const T& value);       

        // �����������õ���uniform��ֵ
        template<typename T, typename = is_glsl_t<T>>
        void SetUniform(pbr_u attribute, const T& value);       

        // ��һ��ָ����ָ���ֵ��һ��uniform
        template<typename T, typename = is_glsl_t<T>>
        void BindUniform(GLuint location, const T* pValue);     

        // �������԰�һ��ָ����ָ���ֵ��һ��uniform
        template<typename T, typename = is_glsl_t<T>>
        void BindUniform(pbr_u attribute, const T* pValue);     

        // ����һ��uniform�����ֵ
        template<typename T, typename = is_glsl_t<T>>
        void SetUniformArray(GLuint location, GLuint size, const std::vector<T>* pArray);   
    };

}  // namespace component

#include "Material.hpp"

