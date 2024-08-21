#ifndef _SPHERICAL_HARMONICS_H
#define _SPHERICAL_HARMONICS_H

#ifndef SH_UNIFORM_LOCATION
#define SH_UNIFORM_LOCATION 90
#endif

// SH系数在C++中预先计算并传递到GLSL（INV_PI因子已经内嵌）
// 如果sh9的uniform位置未指定，默认将使用位置90到98

layout(location = SH_UNIFORM_LOCATION) uniform vec3 sh9[9];

// 计算2个带（4个系数）
vec3 EvaluateSH2(const vec3 n) {
    vec3 irradiance
        = sh9[0]                         // l0m0
        + sh9[1] * (n.y)                 // l1m-1
        + sh9[2] * (n.z)                 // l1m0
        + sh9[3] * (n.x)                 // l1m1
    ;
    return irradiance;
}

// 计算3个带（9个系数）
vec3 EvaluateSH3(const vec3 n) {
    vec3 irradiance
        = sh9[0]                         // l0m0
        + sh9[1] * (n.y)                 // l1m-1
        + sh9[2] * (n.z)                 // l1m0
        + sh9[3] * (n.x)                 // l1m1
        + sh9[4] * (n.y * n.x)           // l2m-2
        + sh9[5] * (n.y * n.z)           // l2m-1
        + sh9[6] * (3.0 * n.z * n.z - 1.0)  // l2m0
        + sh9[7] * (n.z * n.x)           // l2m1
        + sh9[8] * (n.x * n.x - n.y * n.y)  // l2m2
    ;
    return irradiance;
}

#endif
