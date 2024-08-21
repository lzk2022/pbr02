#pragma once
#include <string>
#include <glad/glad.h>
inline constexpr bool gIsDebug = true;
//static bool gIsExit = false;

inline static std::string gResourcePath = "..\\Resource\\";
inline static std::string gShaderPath = "..\\PBR\\Shader\\";
inline static std::string gTexturePath = gResourcePath+"texture\\";
inline static std::string gFontPath = gResourcePath + "font\\";
//inline static std::string gShaderPath = gResourcePath + "shader\\";

struct Hardware
{
    std::string glVendor;        // OpenGL的厂商
    std::string glVersion;       // OpenGL的版本信息
    std::string glRenderer;      // OpenGL的渲染器
    std::string glslVersion;     // GLSL的版本信息

    GLint glTexsize;             // 最大纹理尺寸
    GLint glTexsize3d;           // 最大3D纹理尺寸
    GLint glTexsizeCubemap;      // 最大立方体贴图纹理尺寸

    GLint glMaxTextureUnits;     // 最大纹理单元数
    GLint glMaxImageUnits;       // 最大图像单元数
    GLint glMaxColorBuffs;       // 可绘制颜色缓冲区的最大数量
    GLint glMaxvAtcs;            // 顶点着色器阶段的原子计数器的最大数量
    GLint glMaxfAtcs;            // 片段着色器阶段的原子计数器的最大数量
    GLint glMaxcAtcs;            // 计算着色器阶段的原子计数器的最大数量

    GLint glMaxvUbos;            // 顶点着色器阶段的Uniform缓冲区的最大数量
    GLint glMaxgUbos;            // 几何着色器阶段的Uniform缓冲区的最大数量
    GLint glMaxfUbos;            // 片段着色器阶段的Uniform缓冲区的最大数量
    GLint glMaxcUbos;            // 计算着色器阶段的Uniform缓冲区的最大数量

    GLint glMaxfSsbos;           // 片段着色器的着色器存储块的最大数量
    GLint glMaxcSsbos;           // 计算着色器的着色器存储块的最大数量

    GLint maxInvocations;        // 应用程序的最大调用数限制
    GLint localSizeX;            // 局部工作组X维度大小限制
    GLint localSizeY;            // 局部工作组Y维度大小限制
    GLint localSizeZ;            // 局部工作组Z维度大小限制
    GLint workNumX;              // 工作组X维度数量限制
    GLint workNumY;              // 工作组Y维度数量限制
    GLint workNumZ;              // 工作组Z维度数量限制
};

inline Hardware gHardware;
