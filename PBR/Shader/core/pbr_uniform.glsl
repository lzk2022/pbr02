// 如果_PBR_UNIFORM_H未定义并且fragment_shader已定义，则定义_PBR_UNIFORM_H
#if !defined(_PBR_UNIFORM_H) && defined(fragment_shader)
#define _PBR_UNIFORM_H

// 立方体采样器绑定点（纹理单元）17-19保留用于PBR IBL
layout(binding = 17) uniform samplerCube irradiance_map;
layout(binding = 18) uniform samplerCube prefilter_map;
layout(binding = 19) uniform sampler2D BRDF_LUT;

// 纹理单元（纹理单元）>= 20保留用于PBR使用
layout(binding = 20) uniform sampler2D albedo_map;
layout(binding = 21) uniform sampler2D normal_map;
layout(binding = 22) uniform sampler2D metallic_map;
layout(binding = 23) uniform sampler2D roughness_map;
layout(binding = 24) uniform sampler2D ao_map;
layout(binding = 25) uniform sampler2D emission_map;
layout(binding = 26) uniform sampler2D displace_map;
layout(binding = 27) uniform sampler2D opacity_map;
layout(binding = 28) uniform sampler2D light_map;
layout(binding = 29) uniform sampler2D anisotan_map;  // 各向异性切线图（RGB）
layout(binding = 30) uniform sampler2D ext_unit_30;
layout(binding = 31) uniform sampler2D ext_unit_31;

// 默认块（宽松）统一位置>= 900保留用于PBR使用
layout(location = 900) uniform bool sample_albedo;       // 采样反照率
layout(location = 901) uniform bool sample_normal;       // 采样法线
layout(location = 902) uniform bool sample_metallic;     // 采样金属度
layout(location = 903) uniform bool sample_roughness;    // 采样粗糙度
layout(location = 904) uniform bool sample_ao;           // 采样环境光遮蔽
layout(location = 905) uniform bool sample_emission;     // 采样发光
layout(location = 906) uniform bool sample_displace;     // 采样位移
layout(location = 907) uniform bool sample_opacity;      // 采样不透明度
layout(location = 908) uniform bool sample_lightmap;     // 采样光照图
layout(location = 909) uniform bool sample_anisotan;     // 采样各向异性切线
layout(location = 910) uniform bool sample_ext_910;      // 采样扩展单元910
layout(location = 911) uniform bool sample_ext_911;      // 采样扩展单元911

// 物理材质输入属性


// 共享属性
layout(location = 912) uniform vec4  albedo;         // alpha未预乘
layout(location = 913) uniform float roughness;      // 范围在0.045以内，以便观察到高光
layout(location = 914) uniform float ao;             // 0.0 = 遮蔽，1.0 = 未遮蔽
layout(location = 915) uniform vec4  emission;       // 可选的发光颜色
layout(location = 916) uniform vec2  uv_scale;       // 纹理坐标平铺因子
layout(location = 928) uniform float alpha_mask;     // 丢弃片段的alpha阈值以下

// 标准模型，大部分不透明但可以有简单的alpha混合
layout(location = 917) uniform float metalness;      // 应为二进制值0或1
layout(location = 918) uniform float specular;       // 4% F0 = 0.5，2% F0 = 0.35（水），范围在[0.35, 1]之间
layout(location = 919) uniform float anisotropy;     // ～ [-1, 1]
layout(location = 920) uniform vec3  aniso_dir;      // 各向异性默认为切线方向

// 折射模型仅考虑各向同性介电体
layout(location = 921) uniform float transmission;   // 通过材料传递的漫反射光比例
layout(location = 922) uniform float thickness;      // 法线方向的最大体积厚度
layout(location = 923) uniform float ior;            // 空气1.0，塑料/玻璃1.5，水1.33，宝石1.6-2.33
layout(location = 924) uniform vec3  transmittance;  // 透射颜色作为线性RGB，可能与反照率不同
layout(location = 925) uniform float tr_distance;    // 透射距离，密度较大的IOR较小
layout(location = 931) uniform uint  volume_type;    // 透视根据体积的内部几何变化

// 布料模型仅考虑单层各向同性介电体且不考虑折射
layout(location = 926) uniform vec3  sheen_color;
layout(location = 927) uniform vec3  subsurface_color;

// （可选）附加的清晰涂层，不适用于布料
layout(location = 929) uniform float clearcoat;
layout(location = 930) uniform float clearcoat_roughness;

// 亚表面散射模型非常复杂，目前我们将跳过它，Disney BSDF（2015）和Filament处理实时SSS的方式非常tricky，
// 目前不值得学习。一个良好的SSS模型通常涉及路径追踪体积光学的近似，我会在未来项目中回到这个话题，特别是在上CMU 15-468课程时。

// 两位数数字，指示像素应该如何着色
// 组件x编码着色模型：标准=1，折射=2，布料=3
// 组件y编码附加层：无=0，清晰涂层=1，光泽=2
layout(location = 999) uniform uvec2 model;

// 像素数据定义
struct Pixel {
    vec3 _position;             // 像素位置（内部使用）
    vec3 _normal;               // 像素法线（内部使用）
    vec2 _uv;                   // 第一套纹理坐标（内部使用）
    vec2 _uv2;                  // 第二套纹理坐标（内部使用）
    vec3 _tangent;              // 切线（内部使用）
    vec3 _binormal;             // 双切线（内部使用）
    bool _has_tbn;              // 是否有切线空间基向量（内部使用）
    bool _has_uv2;              // 是否有第二套纹理坐标（内部使用）

    vec3  position;             // 像素位置
    vec2  uv;                   // 第一套纹理坐标
    mat3  TBN;                  // 切线空间基向量（切线、双切线、法线）
    vec3  V;                    // 视觉方向（视点到像素位置的方向）
    vec3  N;                    // 像素法线
    vec3  R;                    // 反射向量（入射光线的反向在平面上的镜像）
    vec3  GN;                   // 几何法线（着色器中使用的法线）
    vec3  GR;                   // 几何反射向量（着色器中使用的反射向量）
    float NoV;                  // 视角与法线的点积
    vec4  albedo;               // 反照率颜色（包括alpha通道）
    float roughness;            // 粗糙度
    float alpha;                // 透明度
    vec3  ao;                   // 环境光遮蔽
    vec4  emission;             // 发光颜色
    vec3  diffuse_color;        // 漫反射颜色
    vec3  F0;                   // 菲涅尔反射率
    vec3  DFG;                  // 漫反射、镜面反射和几何遮挡的乘积
    vec3  Ec;                   // 环境光照
    float metalness;            // 金属度
    float specular;             // 镜面反射强度
    float anisotropy;           // 各向异性程度
    vec3  aniso_T;              // 各向异性切线
    vec3  aniso_B;              // 各向异性双切线
    float clearcoat;            // 清晰涂层强度
    float clearcoat_roughness;  // 清晰涂层粗糙度
    float clearcoat_alpha;      // 清晰涂层透明度
    float eta;                  // 折射率
    float transmission;         // 透射率
    vec3  absorption;           // 吸收率
    float thickness;            // 厚度
    uint  volume;               // 体积类型（用于折射模型）
    vec3  subsurface_color;     // 亚表面散射颜色
};


#endif