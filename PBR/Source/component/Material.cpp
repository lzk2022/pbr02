#include <glm/glm.hpp>
#include "Material.h"
#include "../utils/Log.h"
#include "../utils/Global.h"

namespace component {

    Material::Material(const std::shared_ptr<Shader>& shader) : Component() {
        LOG_TRACK;
        SetShader(shader);

        // ��ʼ������PBR uniform��Ĭ��ֵ���μ� "pbr_metallic.glsl"
        // �����ɫ����δ�����uniformλ�ý��ᱻ��Ĭ����
        if (mShader != nullptr) {
            // ���ò�������ֵ
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

            // ��������
            SetUniform(912U, vec4(1.0f));              // albedo with alpha (not pre-multiplied)
            SetUniform(913U, 1.0f);                    // roughness
            SetUniform(914U, 1.0f);                    // ambient occlussion
            SetUniform(915U, vec4(vec3(0.0f), 1.0f));  // emission
            SetUniform(916U, vec2(1.0f));              // texture coordinates tiling factor
            SetUniform(928U, 0.0f);                    // alpha threshold

            // ��׼ģ��
            SetUniform(917U, 0.0f);                    // metalness
            SetUniform(918U, 0.5f);                    // specular reflectance ~ [0.35, 1]
            SetUniform(919U, 0.0f);                    // anisotropy ~ [-1, 1]
            SetUniform(920U, vec3(1.0f, 0.0f, 0.0f));   // anisotropy direction

            // ����ģ��
            SetUniform(921U, 0.0f);                    // transmission
            SetUniform(922U, 2.0f);                    // thickness
            SetUniform(923U, 1.5f);                    // index of refraction (IOR)
            SetUniform(924U, vec3(1.0f));              // transmittance color
            SetUniform(925U, 4.0f);                    // transmission distance
            SetUniform(931U, 0U);                      // volume type, 0 = uniform sphere, 1 = cube/box/glass

            // ����ģ��
            SetUniform(926U, vec3(1.0f));              // sheen color
            SetUniform(927U, vec3(0.0f));              // subsurface color

            // ����������Ϳ��
            SetUniform(929U, 0.0f);                    // clearcoat
            SetUniform(930U, 0.0f);                    // clearcoat roughness

            // ��ɫģ�Ϳ���
            SetUniform(999U, uvec2(1, 0));             // uvec2 model
        }
    }

    Material::Material(const std::shared_ptr<Material>& material) : Material(*material) {}  // ���ø��ƹ��캯��

    void Material::Bind() const {
        LOG_TRACK;
        // �ϴ�lambda����
        static auto upload = [](auto& unif) { unif.Upload(); };
        LOG_ASSERT(mShader, "�޷��󶨲��ʣ�����������Ч����ɫ��");
        mShader->Bind();  // ���ܰ��Ѹ��ӵ���ɫ��

        // �ϴ�uniformֵ����ɫ��
        for (const auto& [location, unifVariant] : mUniforms) {
            std::visit(upload, unifVariant);
        }

        // ���ܰ���������Ԫ
        for (const auto& [unit, texture] : mTextures) {
            if (texture != nullptr) {
                texture->Bind(unit);
            }
        }
    }

    void Material::Unbind() const {
        LOG_TRACK;
        // ����������ɫ��������󶨵Ĵ��ڣ���������������
        // ���ֵ�ǰ����Ⱦ״̬������һ�����ʵİ󶨹�����ɼ���
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
        mShader = shader;  // ��������Ȩ

        // �������nullptr����ͼ�ǽ�Material����Ϊ�ɾ��Ŀ�״̬
        if (mShader == nullptr) {
            return;
        }

        // ����ɫ���м��ؼ����uniform�����浽 `uniforms` std::map ��
        GLuint id = mShader->getId();
        LOG_INFO("������ɫ���еļ���uniform (id = {0})", id);

        GLint n_uniforms;
        glGetProgramInterfaceiv(id, GL_UNIFORM, GL_ACTIVE_RESOURCES, &n_uniforms);

        GLenum meta_data[] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION, GL_BLOCK_INDEX };

        for (int i = 0; i < n_uniforms; ++i) {
            GLint unifInfo[4]{};
            glGetProgramResourceiv(id, GL_UNIFORM, i, 4, meta_data, 4, NULL, unifInfo);

            if (unifInfo[3] != -1) continue;  // �����ڿ��е�uniform (����UBO����)

            GLint nameLength = unifInfo[0];
            char* name = new char[nameLength];
            glGetProgramResourceName(id, GL_UNIFORM, i, nameLength, NULL, name);

            GLenum type = unifInfo[1];
            GLuint loc = unifInfo[2];

            // ע�⣬OpenGL�᷵�����о��йؼ��� "uniform" ����Դ���ⲻ���������е�uniform��
            // �������������͵Ĳ�������ͼ������ԭ�Ӽ���������Щ������������Ҫ�Ļ�����͸������uniform
            static const std::vector<GLenum> validTypes{
                GL_INT, GL_INT_VEC2, GL_INT_VEC3, GL_INT_VEC4, GL_BOOL, GL_BOOL_VEC2, GL_BOOL_VEC3, GL_BOOL_VEC4,
                GL_UNSIGNED_INT, GL_UNSIGNED_INT_VEC2, GL_UNSIGNED_INT_VEC3, GL_UNSIGNED_INT_VEC4,
                GL_FLOAT, GL_FLOAT_VEC2, GL_FLOAT_VEC3, GL_FLOAT_VEC4, GL_FLOAT_MAT2, GL_FLOAT_MAT3, GL_FLOAT_MAT4,
                GL_DOUBLE, GL_DOUBLE_VEC2, GL_DOUBLE_VEC3, GL_DOUBLE_VEC4, GL_DOUBLE_MAT2, GL_DOUBLE_MAT3, GL_DOUBLE_MAT4,
                GL_FLOAT_MAT2x3, GL_FLOAT_MAT2x4, GL_FLOAT_MAT3x2, GL_FLOAT_MAT3x4, GL_FLOAT_MAT4x2, GL_FLOAT_MAT4x3,
                GL_DOUBLE_MAT2x3, GL_DOUBLE_MAT2x4, GL_DOUBLE_MAT3x2, GL_DOUBLE_MAT3x4, GL_DOUBLE_MAT4x2, GL_DOUBLE_MAT4x3
            };

            if (utils::ranges::find(validTypes, type) == validTypes.end()) {
                delete[] name;  // �����ǰ�˳�����Ҫ�����ͷ��ڴ�
                continue;       // �����ǻ�������uniform (��͸�����Ͳ�������ͼ��)
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
            default: { LOG_EXCEPTION(false,"Uniform {0} ʹ���˲�֧�ֵ����ͣ�", name); }
            }
            #undef IN_PLACE_CONSTRUCT_UNI_VARIANT       // ȡ������ָ��

            delete[] name;
        }
    }

    void Material::SetTexture(GLuint unit, std::shared_ptr<Texture> texture) {
        LOG_TRACK;
        // ���㵱ǰ�Ѿ�ʹ�õ���������
        size_t nTextures = 0;
        for (const auto& [_, texture] : mTextures) {
            if (texture != nullptr) {
                nTextures++;
            }
        }

        // ��ȡϵͳ֧�ֵ��������Ԫ��
        static size_t maxSamplers = gHardware.glMaxTextureUnits;
        // �������Ϊ������ʹ�õ����������Ѵﵽ������ƣ����ӡ������Ϣ������
        if (texture != nullptr && nTextures >= maxSamplers) {
            LOG_ERROR("{0} �����������Ѵﵽ���������ʧ��", maxSamplers);
            return;
        }

        // �����������ָ��������Ԫ������nullptr���ղ�λ����Bind()ʱ������
        mTextures[unit] = texture;
    }

    // ���ò��ʵ������������Թ���������Ӧ������Ԫ
    void Material::SetTexture(pbr_t attribute, std::shared_ptr<Texture> texture) {
        LOG_TRACK;
        GLuint textureUnit = static_cast<GLuint>(attribute);
        SetTexture(textureUnit, texture);

        // ���ڸ�����������Ԫ��������Ӧ��uniform��ָʾ�Ƿ���Ч
        if (textureUnit >= 20) {
            GLuint sampleLoc = textureUnit + 880U;  // "sample_xxx" uniformλ��
            SetUniform(sampleLoc, texture ? true : false);
        }
    }

}
