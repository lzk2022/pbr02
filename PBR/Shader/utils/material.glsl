#ifndef _DISNEY_BSDF_H
#define _DISNEY_BSDF_H

/* 这是从Google Filament改编的简化Disney BSDF材质模型，用于物理渲染理论
   和细节，请查阅Filament核心文档以及从2012年起的SIGGRAPH物理渲染课程系列。

   参考：
   - https://google.github.io/filament/Filament.html
   - https://google.github.io/filament/Materials.html
   - https://blog.selfshadow.com/publications/
   - http://www.advances.realtimerendering.com/
   - https://pbr-book.org/3ed-2018/contents
*/

#include "extension.glsl"

/* ------------------------------ 漫反射BRDF模型 ------------------------------ */

// Disney的漫反射BRDF，考虑粗糙度（尽管不保能量）
// Brent Burley 2012, "Physically Based Shading at Disney"
float Fd_Burley(float alpha, float NoV, float NoL, float HoL) {
    float F90 = 0.5 + 2.0 * HoL * HoL * alpha;
    float a = 1.0 + (F90 - 1.0) * pow5(1.0 - NoL);
    float b = 1.0 + (F90 - 1.0) * pow5(1.0 - NoV);
    return a * b * INV_PI;
}

// Lambertian漫反射BRDF，假设在半球H2上的响应均匀
// 这里1/PI来自于能量守恒约束（对H2上的BRDF积分 = 1）
float Fd_Lambert() {
    return INV_PI;
}

// 使用包装漫反射术语近似的布料漫反射BRDF（能量保守）
// 来源：Filament文档中的Physically Based Rendering，第4.12.2节
float Fd_Wrap(float NoL, float w) {
    float x = pow2(1.0 + w);
    return clamp((NoL + w) / x, 0.0, 1.0);
}

/* ------------------------- 镜面D - 法线分布函数 ------------------------- */

// Trowbridge-Reitz GGX法线分布函数（长尾分布）
// Bruce Walter等人 2007, "Microfacet Models for Refraction through Rough Surfaces"
float D_TRGGX(float alpha, float NoH) {
    float a = NoH * alpha;
    float k = alpha / (1.0 - NoH * NoH + a * a);
    return k * k * INV_PI;
}

// 当gamma = 1时的广义Trowbridge-Reitz NDF（尾部更长）
// Brent Burley 2012, "Physically Based Shading at Disney"
float D_GTR1(float alpha, float NoH) {
    if (alpha >= 1.0) return INV_PI;  // 当gamma = alpha = 1时的奇点情况
    float a2 = alpha * alpha;
    float t = 1.0 + (a2 - 1.0) * NoH * NoH;
    return (a2 - 1.0) / (PI * log(a2) * t);
}

// GTR2（等同于GGX）各向异性法线分布函数
// Brent Burley 2012, "Physically Based Shading at Disney"
float D_AnisoGTR2(float at, float ab, float ToH, float BoH, float NoH) {
    float a2 = at * ab;
    vec3 d = vec3(ab * ToH, at * BoH, a2 * NoH);
    float d2 = dot(d, d);
    float b2 = a2 / d2;
    return a2 * b2 * b2 * INV_PI;
}

// Ashikhmin基于反转高斯的天鹅绒分布函数，由Neubelt归一化
// Ashikhmin和Premoze 2007, "Distribution-based BRDFs"
// Neubelt 2013, "Crafting a Next-Gen Material Pipeline for The Order: 1886"
float D_Ashikhmin(float alpha, float NoH) {
    float a2 = alpha * alpha;
    float cos2 = NoH * NoH;
    float sin2 = 1.0 - cos2;
    float sin4 = sin2 * sin2;
    float cot2 = -cos2 / (a2 * sin2);
    return 1.0 / (PI * (4.0 * a2 + 1.0) * sin4) * (4.0 * exp(cot2) + sin4);
}

// Charlie分布函数，基于指数正弦
// Estevez和Kulla 2017, "Production Friendly Microfacet Sheen BRDF"
float D_Charlie(float alpha, float NoH) {
    float inv_alpha = 1.0 / alpha;
    float cos2 = NoH * NoH;
    float sin2 = 1.0 - cos2;
    return (2.0 + inv_alpha) * pow(sin2, inv_alpha * 0.5) / PI2;
}

/* ------------------------- 镜面G - 几何函数（阴影-遮罩） ------------------------- */

// Smith的几何函数（用于间接图像为基础的照明）
// Schlick 1994, "An Inexpensive BRDF Model for Physically-based Rendering"
float G_SmithGGX_IBL(float NoV, float NoL, float alpha) {
    float k = alpha / 2.0;
    float GGXV = NoV / (NoV * (1.0 - k) + k);  // 从视角V的Schlick-GGX
    float GGXL = NoL / (NoL * (1.0 - k) + k);  // 从光线方向L的Schlick-GGX
    return GGXV * GGXL;
}

// 用于解析光源修改过的Smith几何函数
// Burley 2012, "Physically Based Shading at Disney", Karis 2013, "Real Shading in Unreal Engine 4"
float G_SmithGGX(float NoV, float NoL, float roughness) {
    float r = (roughness + 1.0) * 0.5;  // 在平方之前重新映射粗糙度以减少热度
    return G_SmithGGX_IBL(NoV, NoL, r * r);
}

// Smith的高度相关能见度函数（V = G / 标准化项）
// Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
float V_SmithGGX(float alpha, float NoV, float NoL) {
    float a2 = alpha * alpha;
    float GGXV = NoL * sqrt(NoV * NoV * (1.0 - a2) + a2);
    float GGXL = NoV * sqrt(NoL * NoL * (1.0 - a2) + a2);
    return 0.5 / (GGXV + GGXL);
}

// 高度相关GGX分布的各向异性能见度函数
// Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"
float V_AnisoSmithGGX(float at, float ab, float ToV, float BoV, float ToL, float BoL, float NoV, float NoL) {
    float GGXV = NoL * length(vec3(at * ToV, ab * BoV, NoV));
    float GGXL = NoV * length(vec3(at * ToL, ab * BoL, NoL));
    return 0.5 / (GGXV + GGXL);
}

// Kelemen的清漆镜面BRDF的能见度函数
// Kelemen 2001, "A Microfacet Based Coupled Specular-Matte BRDF Model with Importance Sampling"
float V_Kelemen(float HoL) {
    return 0.25 / (HoL * HoL);
}

// Neubelt的用于布料和天鹅绒分布的平滑能见度函数
// Neubelt和Pettineo 2013, "Crafting a Next-gen Material Pipeline for The Order: 1886"
float V_Neubelt(float NoV, float NoL) {
    return 1.0 / (4.0 * (NoL + NoV - NoL * NoV));
}

/* ------------------------- 镜面F - 菲涅尔反射率函数 ------------------------- */

// Schlick的镜面反射率的近似（菲涅尔因子）
// Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"
vec3 F_Schlick(const vec3 F0, float F90, float HoV) {
    return F0 + (F90 - F0) * pow5(1.0 - HoV);  // HoV = HoL
}

// 当假设切线角反射率F90接近于1时的Schlick近似
vec3 F_Schlick(const vec3 F0, float HoV) {
    return F0 + (1.0 - F0) * pow5(1.0 - HoV);  // HoV = HoL
}

/* ------------------------- 环境遮蔽，其他 ------------------------- */

// 基于地面真实的彩色环境遮挡（彩色GTAO）
// Jimenez等人 2016, "Practical Realtime Strategies for Accurate Indirect Occlusion"
vec3 MultiBounceGTAO(float visibility, const vec3 albedo) {
    vec3 a =  2.0404 * albedo - 0.3324;
    vec3 b = -4.7951 * albedo + 0.6417;
    vec3 c =  2.7552 * albedo + 0.6903;
    float v = visibility;
    return max(vec3(v), ((v * a + b) * v + c) * v);
}

#endif
