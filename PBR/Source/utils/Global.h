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
    std::string glVendor;        // OpenGL�ĳ���
    std::string glVersion;       // OpenGL�İ汾��Ϣ
    std::string glRenderer;      // OpenGL����Ⱦ��
    std::string glslVersion;     // GLSL�İ汾��Ϣ

    GLint glTexsize;             // �������ߴ�
    GLint glTexsize3d;           // ���3D����ߴ�
    GLint glTexsizeCubemap;      // �����������ͼ����ߴ�

    GLint glMaxTextureUnits;     // �������Ԫ��
    GLint glMaxImageUnits;       // ���ͼ��Ԫ��
    GLint glMaxColorBuffs;       // �ɻ�����ɫ���������������
    GLint glMaxvAtcs;            // ������ɫ���׶ε�ԭ�Ӽ��������������
    GLint glMaxfAtcs;            // Ƭ����ɫ���׶ε�ԭ�Ӽ��������������
    GLint glMaxcAtcs;            // ������ɫ���׶ε�ԭ�Ӽ��������������

    GLint glMaxvUbos;            // ������ɫ���׶ε�Uniform���������������
    GLint glMaxgUbos;            // ������ɫ���׶ε�Uniform���������������
    GLint glMaxfUbos;            // Ƭ����ɫ���׶ε�Uniform���������������
    GLint glMaxcUbos;            // ������ɫ���׶ε�Uniform���������������

    GLint glMaxfSsbos;           // Ƭ����ɫ������ɫ���洢����������
    GLint glMaxcSsbos;           // ������ɫ������ɫ���洢����������

    GLint maxInvocations;        // Ӧ�ó����������������
    GLint localSizeX;            // �ֲ�������Xά�ȴ�С����
    GLint localSizeY;            // �ֲ�������Yά�ȴ�С����
    GLint localSizeZ;            // �ֲ�������Zά�ȴ�С����
    GLint workNumX;              // ������Xά����������
    GLint workNumY;              // ������Yά����������
    GLint workNumZ;              // ������Zά����������
};

inline Hardware gHardware;
