#version 460 core
#pragma optimize(off)

// 计算镜面环境BRDF（带能量补偿的多散射LUT）

#ifdef compute_shader

// 扩展函数库 、材质定义 、 投影函数、采样函数
#include "../utils/extension.glsl"
#include "../utils/material.glsl"
#include "../utils/projection.glsl"
#include "../utils/sampling.glsl"

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(binding = 2, rgba16f) restrict writeonly uniform image2D BRDF_LUT;

/* 近似的2D LUT只依赖于两个参数：粗糙度和余弦项。
   也就是说，对于给定的粗糙度，我们只关心余弦值，或者N和L之间的角度，
   而不关心确切的向量。因此，为了方便起见，我们可以假设N是一个常向量`vec3(0, 0, 1)`，
   并将V限制在X-Z平面（V.y = 0），因此对于每个输入的余弦值，我们可以在半圆`x^2 + z^2 = 1`上找到一个唯一的V，
   这样就满足了`dot(N, V) = NoV`，一旦有了半向量H，这也将给我们L。这种假设的思想是保持所有向量在切线空间内，
   由于没有涉及到纹理查找操作，因此无需将它们转换为世界空间，只要所有角度都在同一个参考框架中测量，
   结果就不会改变。

   注意，我们需要正确处理余弦或粗糙度接近零的边缘情况，以减少边界上的伪影：
   如果余弦=0，则N和V将互相正交，可能会引发问题，因此我们将NoV夹紧到0.0001。
   另一方面，当粗糙度为0时，GGX重要性采样将返回一个与N重叠的半向量H，这可能会导致精度错误，
   要解决这个问题，我们必须夹紧粗糙度，或使用`precise`修饰符。

   还请注意，我们在积分中使用的是`NoV`而不是`NoL`作为余弦项，因为L需要从V和H导出。
   实际上，`NoV, NoL, HoV`和`HoL`都是非常接近的角度，可以互换使用而不会增加明显的错误。
   参见：https://knarkowicz.wordpress.com/2014/12/27/analytical-dfg-term-for-ibl/
*/

// 计算BRDF积分的函数
// NoV: 视角向量与法线的余弦值
// roughness: 粗糙度参数
// n_samples: 积分采样次数
vec3 IntegrateBRDF(float NoV, float roughness, uint n_samples) {
    float alpha = roughness * roughness;  // 计算粗糙度的平方作为alpha参数
    float inv_ns = 1.0 / float(n_samples);  // 计算积分采样次数的倒数

    NoV = max(NoV, 1e-4);  // 减少边界伪影，确保NoV不过小
    vec3 N = vec3(0.0, 0.0, 1.0);  // 法线N假设为(0, 0, 1)
    vec3 V = vec3(sqrt(1.0 - NoV * NoV), 0.0, NoV);  // 计算视角向量V，位于X-Z平面上

    float scale = 0.0;  // 初始化scale为0
    float bias = 0.0;   // 初始化bias为0

    // 使用GGX重要性采样来计算积分
    for (uint i = 0; i < n_samples; i++) {
        vec2 u = Hammersley2D(i, n_samples);  // 获取Hammersley序列的采样点
        vec3 H = ImportanceSampleGGX(u.x, u.y, alpha);  // 重要性采样GGX分布得到半角向量H
        precise vec3 L = 2 * dot(V, H) * H - V;  // 计算光照方向L，需要precise修饰符

        // 隐式假设所有向量位于X-Z平面，计算必要的几何项
        float NoL = max(L.z, 0.0);  // 光照向量L与法线N的余弦值
        float NoH = max(H.z, 0.0);  // 半角向量H与法线N的余弦值
        float HoV = max(dot(H, V), 0.0);  // 半角向量H与视角向量V的点积

        if (NoL > 0.0) {
            float V = V_SmithGGX(alpha, NoV, NoL) * NoL * HoV / max(NoH, 1e-5);  // 计算几何遮挡因子
            float Fc = pow5(1.0 - HoV);  // 计算菲涅尔反射项Fc，并分解出积分

            // 考虑多次散射能量补偿
            scale += V * Fc;
            bias += V;
        }
    }

    scale *= (4.0 * inv_ns);  // 缩放得到最终的scale
    bias  *= (4.0 * inv_ns);   // 缩放得到最终的bias

    // 对于布料材质，进行另外一次均匀采样，计算单一的DG项
    float cloth = 0.0;
    for (uint i = 0; i < n_samples; i++) {
        vec2 u = Hammersley2D(i, n_samples);  // 获取Hammersley序列的采样点
        vec3 H = UniformSampleHemisphere(u.x, u.y);  // 均匀采样半球，得到半角向量H
        precise vec3 L = 2 * dot(V, H) * H - V;  // 计算光照方向L，需要precise修饰符

        // 隐式假设所有向量位于X-Z平面，计算必要的几何项
        float NoL = max(L.z, 0.0);  // 光照向量L与法线N的余弦值
        float NoH = max(H.z, 0.0);  // 半角向量H与法线N的余弦值
        float HoV = max(dot(H, V), 0.0);  // 半角向量H与视角向量V的点积

        if (NoL > 0.0) {
            float V = V_Neubelt(NoV, NoL);  // 计算Neubelt方法的几何遮挡因子
            float D = D_Charlie(alpha, NoH);  // 计算Charlie法线分布函数D
            cloth += V * D * NoL * HoV;  // 计算布料材质的BRDF积分
        }
    }

    // 在半球上积分为2PI，因此均匀采样的PDF为1 over 2PI，现在我们将2PI补偿回来（4来自Jacobian项）
    cloth *= (4.0 * PI2 * inv_ns);  // 缩放得到最终的布料项

    return vec3(scale, bias, cloth);  // 返回最终的BRDF LUT像素值，包括scale、bias和cloth
}

// 主函数，根据像素ID计算BRDF LUT的值
void main() {
    vec2 resolution = vec2(imageSize(BRDF_LUT));  // 获取BRDF LUT的分辨率
    float cosine    = float(gl_GlobalInvocationID.x) / resolution.x;  // 计算余弦值
    float roughness = float(gl_GlobalInvocationID.y) / resolution.y;  // 计算粗糙度

    vec3 texel = IntegrateBRDF(cosine, roughness, 2048);  // 调用函数计算BRDF LUT的像素值
    imageStore(BRDF_LUT, ivec2(gl_GlobalInvocationID.xy), vec4(texel, 0.0));  // 将计算结果存储到BRDF LUT中
}

#endif
