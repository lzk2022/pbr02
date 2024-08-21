#include <glm/glm.hpp>
#include "Material.h"
#include "../utils/Log.h"
#include "../utils/Global.h"

namespace component {

    Material::Material(const std::shared_ptr<Shader>& shader) : Component() {
        LOG_TRACK;
        SetShader(shader);

        // 初始化内置PBR uniform的默认值，参见 "pbr_metallic.glsl"
        // 如果着色器中未激活的uniform位置将会被静默忽略
        if (mShader != nullptr) {
            // 设置采样布尔值
            SetUniform(900U, false);                   // sample_albedo
            SetUniform(901U, false);                   // sample_normal
            SetUniform(902U, false);                   // sample_metallic
            SetUniform(903U, false);                   // sample_roughness
            SetUniform(904U, false);                   // sample_ao
            SetUniform(905U, false);                   // sample_emission
            SetUniform(906U, false);                   // sample_displace
            SetUniform(907U, false);                   // sample_opacity
            SetUniform(908U, false);                   // sample_lightmap
            SetUniform(909U, false);                   // sample_anisotan

            // 共享属性
            SetUniform(912U, vec4(1.0f));              // albedo with alpha (not pre-multiplied)
            SetUniform(913U, 1.0f);                    // roughness
            SetUniform(914U, 1.0f);                    // ambient occlussion
            SetUniform(915U, vec4(vec3(0.0f), 1.0f));  // emission
            SetUniform(916U, vec2(1.0f));              // texture coordinates tiling factor
            SetUniform(928U, 0.0f);                    // alpha threshold

            // 标准模型
            SetUniform(917U, 0.0f);                    // metalness
            SetUniform(918U, 0.5f);                    // specular reflectance ~ [0.35, 1]
            SetUniform(919U, 0.0f);                    // anisotropy ~ [-1, 1]
            SetUniform(920U, vec3(1.0f, 0.0f, 0.0f));   // anisotropy direction

            // 折射模型
            SetUniform(921U, 0.0f);                    // transmission
            SetUniform(922U, 2.0f);                    // thickness
            SetUniform(923U, 1.5f);                    // index of refraction (IOR)
            SetUniform(924U, vec3(1.0f));              // transmittance color
            SetUniform(925U, 4.0f);                    // transmission distance
            SetUniform(931U, 0U);                      // volume type, 0 = uniform sphere, 1 = cube/box/glass

            // 布料模型
            SetUniform(926U, vec3(1.0f));              // sheen color
            SetUniform(927U, vec3(0.0f));              // subsurface color

            // 附加清晰的涂层
            SetUniform(929U, 0.0f);                    // clearcoat
            SetUniform(930U, 0.0f);                    // clearcoat roughness

            // 着色模型开关
            SetUniform(999U, uvec2(1, 0));             // uvec2 model
        }
    }

    Material::Material(const std::shared_ptr<Material>& material) : Material(*material) {}  // 调用复制构造函数

    void Material::Bind() const {
        LOG_TRACK;
        // 上传lambda函数
        static auto upload = [](auto& unif) { unif.Upload(); };
        LOG_ASSERT(mShader, "无法绑定材质，请先设置有效的着色器");
        mShader->Bind();  // 智能绑定已附加的着色器

        // 上传uniform值到着色器
        for (const auto& [location, unifVariant] : mUniforms) {
            std::visit(upload, unifVariant);
        }

        // 智能绑定纹理到纹理单元
        for (const auto& [unit, texture] : mTextures) {
            if (texture != nullptr) {
                texture->Bind(unit);
            }
        }
    }

    void Material::Unbind() const {
        LOG_TRACK;
        // 由于智能着色器和纹理绑定的存在，无需解绑或清理操作
        // 保持当前的渲染状态，让下一个材质的绑定工作完成即可
        if constexpr (false) {
            for (const auto& [unit, texture] : mTextures) {
                if (texture != nullptr) {
                    texture->UnBind(unit);
                }
            }
            mShader->UnBind();
        }
    }

    void Material::SetShader(std::shared_ptr<Shader> shader) {
        LOG_TRACK;
        glUseProgram(0);
        mUniforms.clear();
        mTextures.clear();
        mShader = shader;  // 共享所有权

        // 如果传入nullptr，意图是将Material重置为干净的空状态
        if (mShader == nullptr) {
            return;
        }

        // 从着色器中加载激活的uniform并缓存到 `uniforms` std::map 中
        GLuint id = mShader->getId();
        LOG_INFO("解析着色器中的激活uniform (id = {0})", id);

        GLint n_uniforms;
        glGetProgramInterfaceiv(id, GL_UNIFORM, GL_ACTIVE_RESOURCES, &n_uniforms);

        GLenum meta_data[] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_BLOCK_INDEX };

        for (int i = 0; i < n_uniforms; ++i) {
            GLint unifInfo[4]{};
            glGetProgramResourceiv(id, GL_UNIFORM, i, 4, meta_data, 4, NULL, unifInfo);

            if (unifInfo[3] != -1) continue;  // 跳过在块中的uniform (将由UBO处理)

            GLint nameLength = unifInfo[0];
            char* name = new char[nameLength];
            glGetProgramResourceName(id, GL_UNIFORM, i, nameLength, NULL, name);

            GLenum type = unifInfo[1];
            GLuint loc = unifInfo[2];

            // 注意，OpenGL会返回所有具有关键字 "uniform" 的资源，这不仅包括块中的uniform，
            // 还包括所有类型的采样器、图像甚至原子计数器，这些都不是我们想要的基本非透明类型uniform
            static const std::vector<GLenum> validTypes{
                GL_INT, GL_INT_VEC2, GL_INT_VEC3, GL_INT_VEC4, GL_BOOL, GL_BOOL_VEC2, GL_BOOL_VEC3, GL_BOOL_VEC4,
                GL_UNSIGNED_INT, GL_UNSIGNED_INT_VEC2, GL_UNSIGNED_INT_VEC3, GL_UNSIGNED_INT_VEC4,
                GL_FLOAT, GL_FLOAT_VEC2, GL_FLOAT_VEC3, GL_FLOAT_VEC4, GL_FLOAT_MAT2, GL_FLOAT_MAT3, GL_FLOAT_MAT4,
                GL_DOUBLE, GL_DOUBLE_VEC2, GL_DOUBLE_VEC3, GL_DOUBLE_VEC4, GL_DOUBLE_MAT2, GL_DOUBLE_MAT3, GL_DOUBLE_MAT4,
                GL_FLOAT_MAT2x3, GL_FLOAT_MAT2x4, GL_FLOAT_MAT3x2, GL_FLOAT_MAT3x4, GL_FLOAT_MAT4x2, GL_FLOAT_MAT4x3,
                GL_DOUBLE_MAT2x3, GL_DOUBLE_MAT2x4, GL_DOUBLE_MAT3x2, GL_DOUBLE_MAT3x4, GL_DOUBLE_MAT4x2, GL_DOUBLE_MAT4x3
            };

            if (utils::ranges::find(validTypes, type) == validTypes.end()) {
                delete[] name;  // 如果提前退出，不要忘记释放内存
                continue;       // 跳过非基本类型uniform (不透明类型采样器和图像)
            }

            #define IN_PLACE_CONSTRUCT_UNI_VARIANT(i) \
                mUniforms.try_emplace(loc, std::in_place_index<i>, id, loc, name);
            switch (type) {
            case GL_INT:                IN_PLACE_CONSTRUCT_UNI_VARIANT(0);   break;
            case GL_UNSIGNED_INT:       IN_PLACE_CONSTRUCT_UNI_VARIANT(1);   break;
            case GL_BOOL:               IN_PLACE_CONSTRUCT_UNI_VARIANT(2);   break;
            case GL_FLOAT:              IN_PLACE_CONSTRUCT_UNI_VARIANT(3);   break;
            case GL_FLOAT_VEC2:         IN_PLACE_CONSTRUCT_UNI_VARIANT(4);   break;
            case GL_FLOAT_VEC3:         IN_PLACE_CONSTRUCT_UNI_VARIANT(5);   break;
            case GL_FLOAT_VEC4:         IN_PLACE_CONSTRUCT_UNI_VARIANT(6);   break;
            case GL_UNSIGNED_INT_VEC2:  IN_PLACE_CONSTRUCT_UNI_VARIANT(7);   break;
            case GL_UNSIGNED_INT_VEC3:  IN_PLACE_CONSTRUCT_UNI_VARIANT(8);   break;
            case GL_UNSIGNED_INT_VEC4:  IN_PLACE_CONSTRUCT_UNI_VARIANT(9);   break;
            case GL_FLOAT_MAT2:         IN_PLACE_CONSTRUCT_UNI_VARIANT(10);  break;
            case GL_FLOAT_MAT3:         IN_PLACE_CONSTRUCT_UNI_VARIANT(11);  break;
            case GL_FLOAT_MAT4:         IN_PLACE_CONSTRUCT_UNI_VARIANT(12);  break;
            case GL_INT_VEC2:           IN_PLACE_CONSTRUCT_UNI_VARIANT(13);  break;
            case GL_INT_VEC3:           IN_PLACE_CONSTRUCT_UNI_VARIANT(14);  break;
            case GL_INT_VEC4:           IN_PLACE_CONSTRUCT_UNI_VARIANT(15);  break;
            default: { LOG_EXCEPTION(false,"Uniform {0} 使用了不支持的类型！", name); }
            }
            #undef IN_PLACE_CONSTRUCT_UNI_VARIANT       // 取消宏编程指令

            delete[] name;
        }
    }

    void Material::SetTexture(GLuint unit, std::shared_ptr<Texture> texture) {
        LOG_TRACK;
        // 计算当前已经使用的纹理数量
        size_t nTextures = 0;
        for (const auto& [_, texture] : mTextures) {
            if (texture != nullptr) {
                nTextures++;
            }
        }

        // 获取系统支持的最大纹理单元数
        static size_t maxSamplers = gHardware.glMaxTextureUnits;
        // 如果纹理不为空且已使用的纹理数量已达到最大限制，则打印错误信息并返回
        if (texture != nullptr && nTextures >= maxSamplers) {
            LOG_ERROR("{0} 采样器限制已达到，添加纹理失败", maxSamplers);
            return;
        }

        // 将纹理关联到指定的纹理单元，允许nullptr，空槽位会在Bind()时被跳过
        mTextures[unit] = texture;
    }

    // 设置材质的纹理，根据属性关联纹理到相应的纹理单元
    void Material::SetTexture(pbr_t attribute, std::shared_ptr<Texture> texture) {
        LOG_TRACK;
        GLuint textureUnit = static_cast<GLuint>(attribute);
        SetTexture(textureUnit, texture);

        // 对于高索引的纹理单元，设置相应的uniform来指示是否有效
        if (textureUnit >= 20) {
            GLuint sampleLoc = textureUnit + 880U;  // "sample_xxx" uniform位置
            SetUniform(sampleLoc, texture ? true : false);
        }
    }

}
