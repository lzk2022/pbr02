#include "Sampler.h"
#include "../utils/Log.h"
namespace asset {

	Sampler::Sampler(FilterMode mode):Asset(){
        LOG_TRACK;
        glCreateSamplers(1, &mId);

        // ����Ĭ�ϵ��������ģʽΪGL_CLAMP_TO_BORDER�������ñ߽���ɫΪ{0.0f, 0.0f, 0.0f, 1.0f}
        static const float border[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        SetParam(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        SetParam(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        SetParam(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
        SetParam(GL_TEXTURE_BORDER_COLOR, border);

        // ���ݸ����Ĳ���ģʽ��ʼ���������Ĺ���ģʽ
        switch (mode) {
        case FilterMode::Bilinear: {    // ����˫���Թ���
            SetParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            SetParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        }
        case FilterMode::Trilinear: {   // ���������Թ���
            SetParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            SetParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);  // �Ŵ�ʱ��ʹ�� mipmaps
            break;
        }
        case FilterMode::Point:         // Ĭ������´����������
        default: {
            SetParam(GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            SetParam(GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        }
        }
	}
    Sampler::~Sampler(){
        LOG_TRACK;
        glDeleteSamplers(1, &mId);
    }
    void Sampler::Bind(GLuint index) const{
        LOG_TRACK;
        glBindSampler(index, mId);
    }
    void Sampler::UnBind(GLuint index) const{
        LOG_TRACK;
        glBindSampler(index, 0);
    }
}