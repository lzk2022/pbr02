#version 460 core

// 从HDR环境立方体贴图计算漫反射辐照度图

#ifdef compute_shader

#include "../utils/projection.glsl"
#include "../utils/sampling.glsl"

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;
layout(binding = 0) uniform samplerCube environment_map;  // 环境立方体贴图采样器
layout(binding = 0, rgba16f) restrict writeonly uniform imageCube irradiance_map;  // 辐照度立方体图写入目标

// 简单但有偏差和慢的半球采样方法。
// N: 表面法线向量
// h_step: 方位角（经度）的步长
// v_step: 极角（纬度）的步长
vec3 NaiveConvolution(vec3 N, float h_step, float v_step) {
    vec3 irradiance = vec3(0.0);  // 最终辐照度结果
    uint n_samples = 0;  // 采样点数计数器

    // 根据环境贴图的分辨率调整步长，例如1K贴图大约需要0.025的步长，2K/4K可能需要更小的0.01甚至更小的步长。
    // 过大的步长会导致欠采样产生明显的伪影。
    // h_step: 方位角（经度）的增量
    // v_step: 极角（纬度）的增量

    for (float phi = 0.0; phi < PI2; phi += h_step) {
        for (float theta = 0.0; theta < 0.5 * PI; theta += v_step) {
            // 构造采样方向向量L
            vec3 L = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            L = Tangent2World(N, L);  // 将局部空间转换为世界空间
            irradiance += texture(environment_map, L).rgb * cos(theta) * sin(theta);  // 根据cosine权重计算辐照度
            n_samples++;  // 增加采样点数
        }
    }

    /* 这个表达式已经考虑了Lambert漫反射因子`INV_PI`，
       否则我们应该乘以`PI的平方`而不是`PI`，可以参考这个公式：
       http://www.codinglabs.net/article_physically_based_rendering.aspx

       如果你还在疑惑为什么要乘以`PI`，这里是另一个角度的解释：
       每次纹理采样都被`sin(theta) * cos(theta) = sin(2 * theta) / 2`权重化，
       其中极角`theta`在半球上从0到`PI / 2`变化。因此，
       所有权重的期望值是`1 / PI`，如果解决积分，PDF是1除以PI，因此我们乘以`PI`来补偿这一点。
    */

    return PI * irradiance / float(n_samples);  // 返回最终辐照度结果，乘以PI来校正权重
}

// 这种半球采样方法是均匀、无偏且更快的。
// 对于2K分辨率，我们需要大约16000个样本来避免欠采样伪影。
vec3 UniformConvolution(vec3 N, uint n_samples) {
    vec3 irradiance = vec3(0.0);  // 最终辐照度结果

    for (uint i = 0; i < n_samples; i++) {
        vec2 u = Hammersley2D(i, n_samples);  // 获取Hammersley序列中的二维样本点
        vec3 L = UniformSampleHemisphere(u.x, u.y);  // 均匀采样半球
        L = Tangent2World(N, L);  // 将局部空间向量L转换为世界空间
        float NoL = max(dot(N, L), 0.0);  // 计算法线和光照方向的点乘
        irradiance += texture(environment_map, L).rgb * NoL;  // 根据法线和光照方向的点乘计算辐照度
    }

    // 每次纹理采样都由`NoL`加权，其范围从0到1，
    // 采样在半球上是均匀的，因此加权平均权重是0.5。
    // 为了补偿这一点，我们需要将结果乘以2。

    return 2.0 * irradiance / float(n_samples);  // 返回乘以2后的最终辐照度结果
}

// 这种半球采样方法是余弦加权、更精确且更快的。
// 对于2K分辨率，我们只需要大约8000个样本就能得到良好的结果。
vec3 CosineConvolution(vec3 N, uint n_samples) {
    vec3 irradiance = vec3(0.0);  // 最终辐照度结果

    for (uint i = 0; i < n_samples; i++) {
        vec2 u = Hammersley2D(i, n_samples);  // 获取Hammersley序列中的二维样本点
        vec3 L = CosineSampleHemisphere(u.x, u.y);  // 余弦加权采样半球
        L = Tangent2World(N, L);  // 将局部空间向量L转换为世界空间
        irradiance += texture(environment_map, L).rgb;  // 根据采样方向L获取环境贴图的颜色
    }

    /* 因为采样已经是余弦加权的，我们可以直接求和检索到的纹素值
       并除以总的样本数，没有必要再包含一个权重，然后用一个乘法器来平衡结果。
       如果我们像在均匀采样中那样将每个纹素乘以`NoL`，然后将结果加倍，
       我们实际上是对辐射进行了两次加权，这种情况下辐射度图将在明亮像素出现更亮
       和黑暗区域更暗，事实上很多人做错了这个事情。
    */

    return irradiance / float(n_samples);  // 返回最终辐照度结果
}

void main() {
    ivec3 ils_coordinate = ivec3(gl_GlobalInvocationID);  // 计算当前像素在辐照度图中的坐标
    vec2 resolution = vec2(imageSize(irradiance_map));  // 获取辐照度图的分辨率
    vec3 N = ILS2Cartesian(ils_coordinate, resolution);  // 将像素坐标转换为世界空间中的法线向量N

    // 这里我们展示了3种不同的方式从HDR环境贴图计算漫反射辐照度图，
    // 所有3种方法都考虑了积分中的余弦项，并且产生的结果几乎看不出区别。
    // 最后一种方法使用了余弦加权采样，性能更好且需要更少的样本即可收敛。

    // vec3 irradiance = NaiveConvolution(N, 0.01, 0.01);
    // vec3 irradiance = UniformConvolution(N, 16384);
    vec3 irradiance = CosineConvolution(N, 16384);  // 使用余弦加权采样计算辐照度

    imageStore(irradiance_map, ils_coordinate, vec4(irradiance, 1.0));  // 将计算得到的辐照度存储到辐照度图中
}

#endif


