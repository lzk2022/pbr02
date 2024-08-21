#include "Sampler.h"
#include "../utils/Log.h"
namespace asset {

	Sampler::Sampler(FilterMode mode):Asset(){
        LOG_TRACK;
        glCreateSamplers(1, &mId);

        // 设置默认的纹理包裹模式为GL_CLAMP_TO_BORDER，并设置边界颜色为{0.0f, 0.0f, 0.0f, 1.0f}
        static const float border[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        SetParam(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        SetParam(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        SetParam(GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
        SetParam(GL_TEXTURE_BORDER_COLOR, border);

        // 根据给定的采样模式初始化采样器的过滤模式
        switch (mode) {
        case FilterMode::Bilinear: {    // 设置双线性过滤
            SetParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            SetParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        }
        case FilterMode::Trilinear: {   // 设置三线性过滤
            SetParam(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            SetParam(GL_TEXTURE_MAG_FILTER, GL_LINEAR);  // 放大时不使用 mipmaps
            break;
        }
        case FilterMode::Point:         // 默认情况下创建点采样器
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