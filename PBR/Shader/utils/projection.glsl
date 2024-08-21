#ifndef _PROJECTION_H
#define _PROJECTION_H

#include "extension.glsl"

/* 将切线空间中由法线N定义的向量v转换为世界空间

   这是一个简单的坐标变换问题，其中不涉及纹理坐标。
   给定一个法线向量N，我们可以找到切线向量T和副切线向量B，这样
   T、B和N一起定义切线空间的正交基，然后将它们投影到世界基向量X、Y和Z上。

   我们可以选择无限多个T和B向量来构建与N正交的正交基，因此实现可能会有所不同，
   但只要TB平面与半球上的着色点相切，结果就是相同的。因此，我们应该谨慎选择
   一个初始的上向量U，它不与N重叠，否则由于精度误差，cross(U, N)可能无法计算出正确的T。
   此外，在使用之前确保归一化每个向量。
*/
vec3 Tangent2World(vec3 N, vec3 v) {
    N = normalize(N);

    // 选择一个不与N重叠的上向量U
    vec3 U = mix(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), step(abs(N.y), 0.999));
    vec3 T = normalize(cross(U, N));
    vec3 B = normalize(cross(N, T));
    return T * v.x + B * v.y + N * v.z;  // mat3(T, B, N) * v
}

// 将笛卡尔坐标系中的向量v转换为球面坐标
vec2 Cartesian2Spherical(vec3 v) {
    float phi = atan(v.z, v.x);          // ~ [-PI, PI] (假设v已归一化)
    float theta = acos(v.y);             // ~ [0, PI]
    return vec2(phi / PI2, theta / PI);  // ~ [-0.5, 0.5], [0, 1]
}

// 将球面坐标v重新映射为等矩形纹理坐标
vec2 Spherical2Equirect(vec2 v) {
    return vec2(v.x + 0.5, v.y);  // ~ [0, 1]
}

// 将球面极坐标角度v转换为笛卡尔坐标系
vec3 Spherical2Cartesian(vec2 v) {
    float z = sin(v.y);
    return vec3(z * cos(v.x), z * sin(v.x), cos(v.y));
}

// 将立方体贴图面上的2D纹理坐标st转换为等效的3D纹理查找向量v
// 使得 `texture(cubemap, v) == texture(face, st)`
vec3 UV2Cartesian(vec2 st, uint face) {
    vec3 v = vec3(0.0);  // 世界空间中的纹理查找向量
    vec2 uv = 2.0 * vec2(st.x, 1.0 - st.y) - 1.0;  // 将 [0, 1] 转换为 [-1, 1] 并反转y轴

    // https://en.wikipedia.org/wiki/Cube_mapping#Memory_addressing
    switch (face) {
        case 0: v = vec3(+1.0, uv.y, -uv.x); break;  // posx
        case 1: v = vec3(-1.0, uv.y, uv.x); break;   // negx
        case 2: v = vec3(uv.x, +1.0, -uv.y); break;  // posy
        case 3: v = vec3(uv.x, -1.0, uv.y); break;   // negy
        case 4: v = vec3(uv.x, uv.y, +1.0); break;   // posz
        case 5: v = vec3(-uv.x, uv.y, -1.0); break;  // negz
    }

    return normalize(v);
}

// 将ILS图像坐标w转换为其等效的3D纹理查找向量v
// 使得 `texture(samplerCube, v) == imageLoad(imageCube, w)`
vec3 ILS2Cartesian(ivec3 w, vec2 resolution) {
    // w通常来自于计算着色器，形式为 `gl_GlobalInvocationID`
    vec2 st = w.xy / resolution;  // [0, 1] 范围内的纹理坐标
    return UV2Cartesian(st, w.z);
}

// 将立方体贴图纹理查找向量v转换为其ILS等效查找向量w
// 使得 `texture(samplerCube, v) == imageLoad(imageCube, w)`
ivec3 Cartesian2ILS(vec3 v, vec2 resolution) {
    vec3 size = abs(v);
    v /= max3(size);

    // http://alinloghin.com/articles/compute_ibl.html
    // https://en.wikipedia.org/wiki/Cube_mapping#Memory_addressing

    int face;  // posx = 0, negx = 1, posy = 2, negy = 3, posz = 4, negz = 5
    vec2 uv;   // ~ [-1, 1]

    // x主导
    if (size.x > size.y && size.x > size.z) {
        float negx = step(v.x, 0.0);
        face = int(negx);
        uv = mix(vec2(-v.z, v.y), v.zy, negx);  // ~ [-1, 1]
    }
    // y主导
    else if (size.y > size.z) {
        float negy = step(v.y, 0.0);
        face = int(negy) + 2;
        uv = mix(vec2(v.x, -v.z), v.xz, negy);  // ~ [-1, 1]
    }
    // z主导
    else {
        float negz = step(v.z, 0.0);
        face = int(negz) + 4;
        uv = mix(v.xy, vec2(-v.x, v.y), negz);  // ~ [-1, 1]
    }

    // 将 [-1, 1] 转换为 [0, 1] 并反转y轴
    vec2 st = (uv + 1.0) * 0.5;
    st = vec2(st.x, 1.0 - st.y);

    // 按照立方体贴图的分辨率进行缩放，并限制在 [0, resolution - 1] 范围内
    st = st * resolution;
    st = clamp(st, vec2(0.0), resolution - vec2(1.0));

    return ivec3(ivec2(st), face);
}

#endif
