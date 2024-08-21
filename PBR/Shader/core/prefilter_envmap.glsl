#version 460 core

// compute specular prefiltered environment map

#ifdef compute_shader

#include "../utils/projection.glsl"
#include "../utils/material.glsl"
#include "../utils/sampling.glsl"

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
layout(binding = 0) uniform samplerCube environment_map;
layout(binding = 1, rgba16f) restrict writeonly uniform imageCube prefilter_map;

layout(location = 0) uniform float roughness;

/* 
   这部分积分不包含余弦项，我们只是对半球内所有传入的辐射Li进行平均。利用GGX重要性采样，
   我们将半球缩小到由GGX法线分布函数（NDF）定义的较小区域（镜面波段），因此大多数采样将
   在这个波段内进行，这有效地减少了达到收敛所需的总采样数量。

   GGX NDF的形状还取决于视角，为了消除这一维度，"分裂求和"近似进一步假设视角为零，即`N = V = R`。
   请注意，这仅适用于各向同性反射模型。对于我们绘制的每个样本，检索到的像素颜色按`NoL`加权，
   这样可以减少从`N = V = R`假设中产生的误差（正如Epic Games建议的那样），其思想如下：

   当表面法线`N`投影到入射光方向`L`时，那个像素应该对预过滤镜面颜色贡献更多。
   如果`NoL = 0`，则`L`垂直于`N`，因此几乎平行于表面，就好像我们从接近边缘的角度观察一样。
   相反，如果`NoL = 1`，则`N`和`L`重合，我们是从法线入射观察，因此镜面成分必须最高。

   对于包含高频细节的环境贴图，这种卷积可能导致由于欠采样而产生锯齿效应，特别是在分辨率 >= 2K时。
   为了减少这种伪影，NVIDIA提出了一种称为"使用mipmap进行滤波"的技术，能够产生更平滑、视觉上更可接受的结果，
   而不需要增加采样数量。参考：

   https://developer.nvidia.com/gpugems/GPUGems3/gpugems3_ch20.html Section 20.4
   https://github.com/Nadrin/PBR/blob/master/data/shaders/glsl/spmap_cs.glsl

   使用mipmap进行滤波的思路如下：
   PDF越低，每个样本应该平均的环境像素就越多，这大致对应于使用较低的mipmap级别。
   另一方面，如果PDF非常高，很多样本可能朝相似的方向绘制，因此它们有助于平均出来自该区域的积分估计误差。
   在这种情况下，样本应该只平均环境贴图的一个小区域，因此应使用较高的mipmap级别。
*/


// 预过滤环境贴图函数，使用GGX重要性采样对环境贴图进行预过滤
// 参数:
// R: 视角方向向量
// n_samples: 采样数量

vec3 PrefilterEnvironmentMap(vec3 R, uint n_samples) {
    vec3 N = R;  // 法线与视角方向相同，假设为各向同性反射
    vec3 V = R;  // 视角方向

    vec2 env_size = vec2(textureSize(environment_map, 0));
    float w = 4.0 * PI / (6 * env_size.x * env_size.y);  // 每个纹素的立体角（方程12）

    // 由于基本级别直接复制，粗糙度保证大于0
    float alpha = roughness * roughness;

    float weight = 0.0;  // 权重
    vec3 color = vec3(0.0);  // 颜色累积

    for (uint i = 0; i < n_samples; i++) {
        vec2 u = Hammersley2D(i, n_samples);  // Hammersley序列
        vec3 H = Tangent2World(N, ImportanceSampleGGX(u.x, u.y, alpha));  // GGX重要性采样得到半角向量H
        vec3 L = 2 * dot(H, V) * H - V;  // 入射光方向L

        float NoH = max(dot(N, H), 0.0);  // 法线与半角向量的点积
        float NoL = max(dot(N, L), 0.0);  // 法线与入射光方向的点积
        float HoV = max(dot(H, V), 0.0);  // 半角向量与视角方向的点积

        if (NoL > 0.0) {
            float pdf = D_TRGGX(NoH, alpha) * NoH / (4.0 * HoV);  // 概率密度函数（方程11）
            float ws = 1.0 / (n_samples * pdf + 0.0001);  // 与此样本相关的立体角（方程11）
            float mip_level = max(0.5 * log2(ws / w) + 1.0, 0.0);  // Mipmap级别（方程13，偏移+1）
            color += textureLod(environment_map, L, mip_level).rgb * NoL;  // 累积颜色
            weight += NoL;  // 累积权重
        }
    }

    return color / weight;  // 返回预过滤后的环境贴图颜色
}

// 主函数，使用计算着色器计算镜面预过滤环境贴图
void main() {
    ivec3 ils_coordinate = ivec3(gl_GlobalInvocationID);  // 获取全局调度ID
    vec2 resolution = vec2(imageSize(prefilter_map));  // 获取预过滤贴图的分辨率

    // 确保在计算更高Mipmap级别时不会超出贴图范围
    if (ils_coordinate.x >= resolution.x || ils_coordinate.y >= resolution.y) {
        return;
    }

    vec3 R = ILS2Cartesian(ils_coordinate, resolution);  // 将索引坐标转换为球面坐标系下的方向向量R
    vec3 color = PrefilterEnvironmentMap(R, 2048);  // 对方向R进行2048次采样预过滤环境贴图

    imageStore(prefilter_map, ils_coordinate, vec4(color, 1.0));  // 存储预过滤后的颜色到预过滤贴图
}

#endif
