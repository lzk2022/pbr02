#if !defined(_PBR_SHADING_H) && defined(_PBR_UNIFORM_H)
#define _PBR_SHADING_H

#include "../utils/extension.glsl"
#include "../utils/easing.glsl"
#include "../utils/sampling.glsl"
#include "../utils/material.glsl"
#include "../utils/postprocess.glsl"

// 该着色器实现了我们的物理基础渲染API，参考：
// https://google.github.io/filament/Filament.html
// https://blog.selfshadow.com/publications/
// https://pbr-book.org/3ed-2018/contents

/* 计算分裂求和方程中的LD项（从预过滤环境贴图中）

   这里我们使用了一个非线性映射来映射粗糙度和mipmap级别，
   以突出镜面反射。这个四次缓入函数将大多数粗糙度小于0.5的表面映射到
   预过滤环境贴图的基础级别。根据情况，人们可能更喜欢粗糙外观而不是
   强烈的光滑表面，这通常看起来更好、更真实。在这种情况下，
   您可以使用一个立方体缓出函数来调整粗糙度，使得大多数表面从
   更高的mipmap级别采样，镜面IBL将被限制在高度光滑的表面上。
*/

// 计算LD项（从预过滤环境贴图中），在分裂求和方程中使用

// R: 反射向量
// roughness: 表面粗糙度，范围[0, 1]
vec3 ComputeLD(const vec3 R, float roughness) {
    const float max_level = textureQueryLevels(prefilter_map) - 1.0;  // 获取预过滤环境贴图的最大mipmap级别
    float miplevel = max_level * QuarticEaseIn(roughness);  // 使用四次缓入函数映射粗糙度到mipmap级别
    return textureLod(prefilter_map, R, miplevel).rgb;  // 根据映射后的mipmap级别采样预过滤环境贴图
}

/* 计算当前像素处的切线-副切线-法线（TBN）矩阵

   TBN矩阵用于将向量从切线空间转换到世界空间，
   如果VBO中未提供切线和副切线作为顶点属性输入，
   则此函数将使用局部偏导数来近似计算矩阵。
   如果已经提供了切线和副切线，除非插值的T、B和N向量
   可能失去正交性，否则我们不需要这个函数。
*/

// 计算TBN矩阵（切线-副切线-法线），用于将向量从切线空间转换到世界空间

// position: 当前像素的位置
// normal: 当前像素的法线向量
// uv: 当前像素的纹理坐标
mat3 ComputeTBN(const vec3 position, const vec3 normal, const vec2 uv) {
    vec3 dpx = dFdxFine(position);  // 计算位置向量关于x的精细偏导数
    vec3 dpy = dFdyFine(position);  // 计算位置向量关于y的精细偏导数
    vec2 duu = dFdxFine(uv);  // 计算纹理坐标关于x的精细偏导数
    vec2 dvv = dFdyFine(uv);  // 计算纹理坐标关于y的精细偏导数

    vec3 N = normalize(normal);  // 归一化法线向量
    vec3 T = normalize(dpx * dvv.t - dpy * duu.t);  // 计算切线向量
    vec3 B = -normalize(cross(N, T));  // 计算副切线向量

    return mat3(T, B, N);  // 返回TBN矩阵
}

/* 计算粗糙度-各向异性-修正的反射向量RAR

   对于完美的镜面表面，我们使用反射向量R来获取IBL。
   对于100%漫反射，使用法线向量N，因为漫反射与视角无关。
   给定粗糙度范围为[0, 1]，现在我们有漫反射和镜面的混合，
   因此我们应该根据表面的粗糙度插值两者之间，这给出了向量RAR。
   RAR还考虑了各向异性，因此使用了字母A。

   RAR向量仅用于计算镜面LD项。至于漫反射部分，我们仍然应该使用
   各向同性法线向量N来获取预计算的辐照度图或评估SH9，而不考虑各向异性。
   各向异性仅适用于刷磨金属，因此主要涉及镜面组分。
*/


// 计算RAR向量（粗糙度-各向异性-修正的反射向量）

// px: 像素数据结构，包含反射向量R、法线向量N、视线方向V、粗糙度、各向异性参数、透明度等
vec3 ComputeRAR(const Pixel px) {
    vec3 R = px.R;  // 初始化反射向量为输入像素的反射向量R

    // 如果各向异性参数绝对值大于1e-5，表示存在各向异性
    if (abs(px.anisotropy) > 1e-5) {
        // 根据各向异性参数选择使用主刷磨方向还是副刷磨方向
        vec3 aniso_B = px.anisotropy >= 0.0 ? px.aniso_B : px.aniso_T;
        vec3 aniso_T = cross(aniso_B, px.V);  // 计算各向异性切线向量
        vec3 aniso_N = cross(aniso_T, aniso_B);  // 计算各向异性法线向量

        // 根据粗糙度和各向异性参数插值计算修正后的法线向量N
        vec3 N = mix(px.N, aniso_N, abs(px.anisotropy) * clamp(px.roughness * 4, 0.0, 1.0));

        // 根据修正后的法线向量计算反射向量R
        R = reflect(-px.V, normalize(N));
    }

    // 根据透明度的平方根插值反射向量R和法线向量N，得到最终的RAR向量
    return mix(R, px.N, px.alpha * px.alpha);
}


// 计算漫反射颜色，根据基础颜色（反照率）和金属度
// 对于非金属材质，漫反射颜色为持久的基础颜色，F0是无色的
// 对于导电体，没有漫反射颜色，diffuse = vec3(0)，F0是有色的
vec3 ComputeAlbedo(const vec3 albedo, float metalness) {
    return albedo * (1.0 - metalness);  // 计算并返回漫反射颜色
}

// 计算F0值，根据基础颜色（反照率）、金属度和高光度
// 对于导电体是有色的，对于非金属材质是无色的
vec3 ComputeF0(const vec3 albedo, float metalness, float specular) {
    vec3 dielectric_F0 = vec3(0.16 * specular * specular);  // 计算非导电体的F0值
    return mix(dielectric_F0, albedo, metalness);  // 根据金属度插值计算并返回F0值
}

// 计算F0值，根据入射折射率和透射折射率（折射率）
// 假设空气-介质界面，空气的入射折射率ni = 1

// ni: 入射折射率
// nt: 透射折射率
vec3 ComputeF0(float ni, float nt) {
    float x = (nt - ni) / (nt + ni);  // x = (eta - 1) / (eta + 1)
    return vec3(x * x);  // 计算并返回F0值
}

// 计算折射介质的折射率（IOR），根据F0值，假设空气-介质界面
// 返回的值实际上是 eta = nt / ni，但由于 ni = 1，因此本质上是折射率（IOR）

// F0: F0值，用于计算折射率
float ComputeIOR(vec3 F0) {
    float x = sqrt(F0.x);  // 计算F0.x的平方根
    return (1.0 + x) / (1.0 - x);  // 计算并返回折射率（IOR）
}

// 计算用于偏移阴影图的偏置项，以消除阴影痤疮
// 当NoL较小时需要更多的偏置，NoL较大时需要较少的偏置（垂直表面）
// 如果阴影图的分辨率较高，可以减小最小/最大偏置值
// L和N必须归一化，N必须是几何法线，而非来自法线贴图

// L: 光线方向向量
// N: 表面法线向量
float ComputeDepthBias(const vec3 L, const vec3 N) {
    const float max_bias = 0.001;  // 最大偏置值
    const float min_bias = 0.0001;  // 最小偏置值
    return max(max_bias * (1.0 - dot(N, L)), min_bias);  // 计算并返回深度偏置值
}

/*
    评估像素处的折射路径，假设空气-介质界面
    折射的确切行为取决于介质内部的几何结构
    简化起见，我们只考虑形式为球体或立方体的均匀固体体积

    如果光线进入一个体积为均匀球体、圆柱体或胶囊体的介质，它会
    被球形扭曲，并且表面上每一点的本地厚度都不同。
    本地厚度从直径d降至0，当我们从球心到边缘时

    如果光线进入一个均匀平面体积，例如立方体、塑料条或玻璃板，它会
    由于体积的厚度而不是由于体积的厚度而移动。入/出接口
    对称于对称，这意味着光的出射方向等于
    入射方向，这就是视图向量V。因此，在采样远处的无限远时
    环境地图，我们将无法观察到像在现实生活中那样变化的位移。为此，我们采用了一个廉价的解决方案通过添加一个硬编码的偏移量。

    如果使用本地光探头而不是远距离的IBL，我们将需要另一个
    函数，因为采样本地IBL取决于位置
*/

// px: 像素信息结构体，包含折射需要的所有参数
// transmittance: 输出参数，传递率
// direction: 输出参数，折射方向
void EvalRefraction(const Pixel px, out vec3 transmittance, out vec3 direction) {
    // 球形折射
    if (px.volume == 0) {
        vec3 r_in = refract(-px.V, px.N, px.eta);  // 计算入射光线的折射向量
        float NoR = dot(-px.N, r_in);  // 计算入射角的余弦值

        float m_thickness = px.thickness * px.NoV;  // 本地厚度变化
        float r_distance = m_thickness * NoR;  // 计算折射距离

        vec3 T = clamp(exp(-px.absorption * r_distance), 0.0, 1.0);  // 贝尔-兰伯特定律
        vec3 n_out = -normalize(NoR * r_in + px.N * 0.5);  // 从出口到球心的向量
        vec3 r_out = refract(r_in, n_out, 1.0 / px.eta);  // 计算出射光线的折射向量

        transmittance = T;  // 输出传递率
        direction = r_out;  // 输出折射方向
    }

    // 立方体或平面折射
    else if (px.volume == 1) {
        vec3 r_in = refract(-px.V, px.N, px.eta);  // 计算入射光线的折射向量
        float NoR = dot(-px.N, r_in);  // 计算入射角的余弦值

        float m_thickness = px.thickness;  // 平面表面上的厚度是恒定的
        float r_distance = m_thickness / max(NoR, 0.001);  // 计算折射距离

        vec3 T = clamp(exp(-px.absorption * r_distance), 0.0, 1.0);  // 贝尔-兰伯特定律
        vec3 r_out = normalize(r_in * r_distance - px.V * 10.0);  // 一个固定的偏移量为10.0

        transmittance = T;  // 输出传递率
        direction = r_out;  // 输出折射方向
    }
}

// 评估基础材质的漫反射BRDF分布
// px: 像素信息结构体，包含用于计算的所有参数
// NoV: 视角与表面法线的夹角余弦值
// NoL: 光线方向与表面法线的夹角余弦值
// HoL: 半角向量与光线方向的夹角余弦值
vec3 EvalDiffuseLobe(const Pixel px, float NoV, float NoL, float HoL) {
    return px.diffuse_color * Fd_Burley(px.alpha, NoV, NoL, HoL);
}

// 评估基础材质的高光BRDF分布
// px: 像素信息结构体，包含用于计算的所有参数
// L: 入射光线方向向量
// H: 半角向量
// NoV: 视角与表面法线的夹角余弦值
// NoL: 光线方向与表面法线的夹角余弦值
// NoH: 半角向量与表面法线的夹角余弦值
// HoL: 半角向量与光线方向的夹角余弦值
vec3 EvalSpecularLobe(const Pixel px, const vec3 L, const vec3 H, float NoV, float NoL, float NoH, float HoL) {
    float D = 0.0;  // 分布项
    float V = 0.0;  // 几何遮挡项
    vec3  F = vec3(0.0);  // 菲涅尔项

    if (model.x == 3) {  // 布料高光BRDF
        D = D_Charlie(px.alpha, NoH);  // Charlie分布函数
        V = V_Neubelt(NoV, NoL);  // Neubelt几何遮挡函数
        F = px.F0;  // 用绒面颜色替代菲涅尔项，以模拟柔和的光泽
    }
    else if (abs(px.anisotropy) <= 1e-5) {  // 非布料，各向同性高光BRDF
        D = D_TRGGX(px.alpha, NoH);  // TRGGX分布函数
        V = V_SmithGGX(px.alpha, NoV, NoL);  // Smith-GGX几何遮挡函数
        F = F_Schlick(px.F0, HoL);  // Schlick近似菲涅尔函数
    }
    else {  // 非布料，各向异性高光BRDF
        float HoT = dot(px.aniso_T, H);
        float HoB = dot(px.aniso_B, H);
        float LoT = dot(px.aniso_T, L);
        float LoB = dot(px.aniso_B, L);
        float VoT = dot(px.aniso_T, px.V);
        float VoB = dot(px.aniso_B, px.V);

        // Brent Burley 2012, "Physically Based Shading at Disney"
        // float aspect = sqrt(1.0 - 0.9 * px.anisotropy);
        // float at = max(px.alpha / aspect, 0.002025);  // 沿切线方向的alpha值
        // float ab = max(px.alpha * aspect, 0.002025);  // 沿副法线方向的alpha值

        // Kulla 2017, "Revisiting Physically Based Shading at Imageworks"
        float at = max(px.alpha * (1.0 + px.anisotropy), 0.002025);  // 限制为0.045 ^ 2 = 0.002025
        float ab = max(px.alpha * (1.0 - px.anisotropy), 0.002025);

        D = D_AnisoGTR2(at, ab, HoT, HoB, NoH);  // 各向异性GTR2分布函数
        V = V_AnisoSmithGGX(at, ab, VoT, VoB, LoT, LoB, NoV, NoL);  // 各向异性Smith-GGX几何遮挡函数
        F = F_Schlick(px.F0, HoL);  // Schlick近似菲涅尔函数
    }

    return (D * V) * F;  // 返回最终高光颜色值
}

// 评估添加型透明涂层的高光BRDF分布
// px: 像素信息结构体，包含用于计算的所有参数
// NoH: 半角向量与表面法线的夹角余弦值
// HoL: 半角向量与光线方向的夹角余弦值
// Fcc: 输出的透明涂层的Fresnel反射率
vec3 EvalClearcoatLobe(const Pixel px, float NoH, float HoL, out float Fcc) {
    float D = D_TRGGX(px.clearcoat_alpha, NoH);  // TRGGX分布函数
    float V = V_Kelemen(HoL);  // Kelemen几何遮挡函数
    vec3  F = F_Schlick(vec3(0.04), HoL) * px.clearcoat;  // 假设折射率为1.5（4%的F0）
    Fcc = F.x;  // 输出透明涂层的Fresnel反射率
    return (D * V) * F;  // 返回透明涂层的高光颜色值
}

// 评估单位强度的白色解析光源的贡献
// px: 像素信息结构体，包含用于计算的所有参数
// L: 入射光线方向向量
vec3 EvaluateAL(const Pixel px, const vec3 L) {
    float NoL = dot(px.N, L);  // 视角与表面法线的夹角余弦值
    if (NoL <= 0.0) return vec3(0.0);  // 光线方向与表面法线背向，返回黑色

    vec3 H = normalize(px.V + L);  // 半角向量
    vec3 Fr = vec3(0.0);  // 高光项
    vec3 Fd = vec3(0.0);  // 漫反射项
    vec3 Lo = vec3(0.0);  // 输出颜色

    float NoV = px.NoV;  // 视角与表面法线的夹角余弦值
    float NoH = max(dot(px.N, H), 0.0);  // 半角向量与表面法线的夹角余弦值
    float HoL = max(dot(H, L), 0.0);  // 半角向量与光线方向的夹角余弦值

    if (model.x == 1) {  // 标准模型
        Fr = EvalSpecularLobe(px, L, H, NoV, NoL, NoH, HoL) * px.Ec;  // 补偿能量的高光项
        Fd = EvalDiffuseLobe(px, NoV, NoL, HoL);  // 漫反射项
        Lo = (Fd + Fr) * NoL;  // 计算颜色输出
    }
    else if (model.x == 2) {  // 折射模型
        Fr = EvalSpecularLobe(px, L, H, NoV, NoL, NoH, HoL) * px.Ec;  // 补偿能量的高光项
        Fd = EvalDiffuseLobe(px, NoV, NoL, HoL) * (1.0 - px.transmission);  // 考虑透射的漫反射项
        Lo = (Fd + Fr) * NoL;  // 计算颜色输出
    }
    else if (model.x == 3) {  // 布料模型
        Fr = EvalSpecularLobe(px, L, H, NoV, NoL, NoH, HoL);  // 布料特有的高光项，无需能量补偿
        Fd = EvalDiffuseLobe(px, NoV, NoL, HoL) * clamp01(px.subsurface_color + NoL);  // 伪造次表面颜色
        float cloth_NoL = Fd_Wrap(NoL, 0.5);  // 模拟次表面散射
        Lo = Fd * cloth_NoL + Fr * NoL;  // 计算颜色输出
    }

    if (model.y == 1) {  // 添加型透明涂层
        float NoLcc = max(dot(px.GN, L), 0.0);  // 使用几何法线
        float NoHcc = max(dot(px.GN, H), 0.0);  // 使用几何法线
        float Fcc = 0.0;  // 透明涂层的Fresnel反射率
        // 透明涂层仅具有高光项，漫反射项通过覆盖基本粗糙度进行伪造
        vec3 Fr_cc = EvalClearcoatLobe(px, NoHcc, HoL, Fcc);  // 透明涂层的高光项
        Lo *= (1.0 - Fcc);  // 乘以透明涂层的反射率
        Lo += (Fr_cc * NoLcc);  // 添加透明涂层的高光项
    }

    return Lo;  // 返回计算得到的颜色输出
}

/*********************************** MAIN API ***********************************/

// 初始化当前像素（片段），从材质输入计算得到的值
// px: 输入输出的像素信息结构体
// camera_pos: 相机位置
void InitPixel(inout Pixel px, const vec3 camera_pos) {
    px.position = px._position;  // 像素位置
    px.uv = px._uv * uv_scale;  // 纹理坐标经过缩放后的值

    // 计算切线空间(TBN)矩阵，如果没有提供切线空间则通过局部导数近似
    px.TBN = px._has_tbn ? mat3(px._tangent, px._binormal, px._normal) :
        ComputeTBN(px._position, px._normal, px.uv);

    px.V = normalize(camera_pos - px.position);  // 视角向量
    px.N = sample_normal ? normalize(px.TBN * (texture(normal_map, px.uv).rgb * 2.0 - 1.0)) : px._normal;  // 表面法线
    px.R = reflect(-px.V, px.N);  // 反射向量
    px.NoV = max(dot(px.N, px.V), 1e-4);  // 视角与表面法线的夹角余弦值，确保不为零

    px.GN = px._normal;  // 几何法线向量，不受法线贴图影响
    px.GR = reflect(-px.V, px._normal);  // 几何反射向量

    px.albedo = sample_albedo ? vec4(Gamma2Linear(texture(albedo_map, px.uv).rgb), 1.0) : albedo;  // 漫反射颜色
    px.albedo.a = sample_opacity ? texture(opacity_map, px.uv).r : px.albedo.a;  // 不透明度
    // px.albedo.rgb *= px.albedo.a;  // 预乘透明度通道（已被注释掉）

    if (px.albedo.a < alpha_mask) {  // 根据透明度掩码丢弃片段
        discard;
    }

    px.roughness = sample_roughness ? texture(roughness_map, px.uv).r : roughness;  // 粗糙度
    px.roughness = clamp(px.roughness, 0.045, 1.0);  // 将粗糙度限制在0.045到1.0之间
    px.alpha = pow2(px.roughness);  // 从粗糙度计算alpha值

    px.ao = sample_ao ? texture(ao_map, px.uv).rrr : vec3(ao);  // 环境光遮蔽
    px.emission = sample_emission ? vec4(Gamma2Linear(texture(emission_map, px.uv).rgb), 1.0) : emission;  // 自发光
    px.DFG = texture(BRDF_LUT, vec2(px.NoV, px.roughness)).rgb;  // BRDF LUT纹理采样结果

    // 标准模型，绝缘体或金属，可选各向异性
    if (model.x == 1) {
        px.metalness = sample_metallic ? texture(metallic_map, px.uv).r : metalness;  // 金属度
        px.specular = clamp(specular, 0.35, 1.0);  // 高光强度
        px.diffuse_color = ComputeAlbedo(px.albedo.rgb, px.metalness);  // 漫反射颜色
        px.F0 = ComputeF0(px.albedo.rgb, px.metalness, px.specular);  // F0值
        px.anisotropy = anisotropy;  // 各向异性度
        px.aniso_T = sample_anisotan ? texture(anisotan_map, px.uv).rgb : aniso_dir;  // 各向异性方向
        px.aniso_T = normalize(px.TBN * px.aniso_T);
        px.aniso_B = normalize(cross(px._normal, px.aniso_T));  // 使用几何法线计算，而非法线贴图
        px.Ec = 1.0 + px.F0 * (1.0 / px.DFG.y - 1.0);  // 能量补偿因子 >= 1.0
    }

    // 折射模型，仅适用于各向同性介电体
    else if (model.x == 2) {
        px.anisotropy = 0.0;  // 无各向异性
        px.metalness = 0.0;  // 非金属
        px.diffuse_color = px.albedo.rgb;  // 漫反射颜色同基本颜色相同
        px.F0 = ComputeF0(1.0, clamp(ior, 1.0, 2.33));  // 计算折射介质的F0值
        px.Ec = 1.0 + px.F0 * (1.0 / px.DFG.y - 1.0);  // 能量补偿因子

        px.eta = 1.0 / ior;  // 空气到介质的折射率
        px.transmission = clamp01(transmission);  // 透射率

        // 注意，透射距离定义了光在介质中传播的距离
        // 对于高折射率的密集介质，光线会弯曲得更多，并且随着传播会显著衰减
        // 因此，`tr_distance`应该设置小一些，否则可以设置得很大，不要将其限制为最大厚度，因为它实际上可以到达无穷大（例如在真空中）

        px.absorption = -log(clamp(transmittance, 1e-5, 1.0)) / max(1e-5, tr_distance);  // 吸收率
        px.thickness = max(thickness, 1e-5);  // 体积的最大厚度，而非每个像素
        px.volume = clamp(volume_type, uint(0), uint(1));  // 体积类型
    }

    // 布料模型，单层各向同性介电体，无折射
    else if (model.x == 3) {
        px.anisotropy = 0.0;  // 无各向异性
        px.metalness = 0.0;

        if (!sample_roughness) {
            px.roughness = px.roughness * 0.2 + 0.8;  // 布料粗糙度小于0.8不现实
            px.alpha = pow2(px.roughness);  // 从粗糙度计算alpha值
        }

        px.diffuse_color = px.albedo.rgb;  // 使用基本颜色作为漫反射颜色
        px.F0 = sheen_color;  // 使用光泽色作为高光F0值
        px.subsurface_color = subsurface_color;  // 次表面颜色
        px.Ec = vec3(1.0);  // 次表面散射损失能量，因此不需要能量补偿
    }

    // 添加型透明涂层
    if (model.y == 1) {
        px.clearcoat = clearcoat;  // 透明涂层强度
        px.clearcoat_roughness = clamp(clearcoat_roughness, 0.045, 1.0);  // 透明涂层粗糙度
        px.clearcoat_alpha = pow2(px.clearcoat_roughness);  // 从粗糙度计算alpha值

        // 如果涂层比基本表面更粗糙，应覆盖基本表面的粗糙度
        float max_roughness = max(px.roughness, px.clearcoat_roughness);
        float mix_roughness = mix(px.roughness, max_roughness, px.clearcoat);
        px.roughness = clamp(mix_roughness, 0.045, 1.0);  // 更新粗糙度
        px.alpha = pow2(px.roughness);  // 从粗糙度计算alpha值
    }
}


// 评估像素处环境IBL的贡献
// px: 输入的像素信息结构体
vec3 EvaluateIBL(const Pixel px) {
    vec3 Fr = vec3(0.0);  // 镜面反射（菲涅尔效应），乘以E加权
    vec3 Fd = vec3(0.0);  // 漫反射，乘以(1 - E) * (1 - transmission)
    vec3 Ft = vec3(0.0);  // 漫折射，乘以(1 - E) * transmission

    vec3 E = vec3(0.0);  // 镜面BRDF的总能量贡献（LD项之后的积分）
    vec3 AO = px.ao;     // 漫反射环境遮蔽

    if (model.x == 3) {  // 布料模型
        E = px.F0 * px.DFG.z;
        AO *= MultiBounceGTAO(AO.r, px.diffuse_color);
        AO *= Fd_Wrap(px.NoV, 0.5);  // 模拟带有包裹漫反射项的次表面散射
        AO *= clamp01(px.subsurface_color + px.NoV);  // 模拟次表面颜色（简易方法）
    }
    else {
        E = mix(px.DFG.xxx, px.DFG.yyy, px.F0);
        AO *= MultiBounceGTAO(AO.r, px.diffuse_color);
    }

    Fr = ComputeLD(ComputeRAR(px), px.roughness) * E;
    Fr *= px.Ec;  // 应用多次散射能量补偿（Kulla-Conty 17和Lagarde 18）

    // 辐照度贴图已经包含朗伯BRDF，因此直接乘以漫反射颜色
    // 这里不需要除以PI，因为这将导致球谐函数重复计数
    // 对于球谐函数，INV_PI应该在C++预计算期间合并到SH9中

    Fd = texture(irradiance_map, px.N).rgb * px.diffuse_color * (1.0 - E);
    Fd *= AO;  // 应用环境遮蔽和多次散射彩色GTAO

    if (model.x == 2) {  // 折射模型
        vec3 transmittance;
        vec3 r_out;
        EvalRefraction(px, transmittance, r_out);

        Ft = ComputeLD(r_out, px.roughness) * px.diffuse_color;
        Ft *= transmittance;  // 应用吸收（透射颜色可能与基础反射率不同）

        // 注意，反射和折射是互斥的，从表面反射的光子不会进入物体内部，
        // 因此，折射的存在只会消耗部分漫反射贡献，但不会影响镜面部分

        Fd *= (1.0 - px.transmission);  // 已经乘以(1.0 - E)
        Ft *= (1.0 - E) * px.transmission;
    }

    if (model.y == 1) {  // 添加型透明涂层
        float Fcc = F_Schlick(vec3(0.04), px.NoV).x * px.clearcoat;  // 聚氨酯F0 = 4%
        Fd *= (1.0 - Fcc);
        Fr *= (1.0 - Fcc);
        Fr += ComputeLD(px.GR, px.clearcoat_roughness) * Fcc;
    }

    return Fr + Fd + Ft;
}

// 评估白色定向光的单位强度贡献
// px: 输入的像素信息结构体
// L: 光源方向向量
// visibility: 可见性因子
vec3 EvaluateADL(const Pixel px, const vec3 L, float visibility) {
    return visibility <= 0.0 ? vec3(0.0) : (EvaluateAL(px, L) * visibility);
}

// 评估白色点光源的单位强度贡献
// px: 输入的像素信息结构体
// position: 光源位置
// range: 光源范围
// linear: 线性衰减因子
// quadratic: 平方衰减因子
// visibility: 可见性因子
vec3 EvaluateAPL(const Pixel px, const vec3 position, float range, float linear, float quadratic, float visibility) {
    vec3 L = normalize(position - px.position);

    // 距离衰减：反比平方衰减
    float d = distance(position, px.position);
    float attenuation = (d >= range) ? 0.0 : (1.0 / (1.0 + linear * d + quadratic * d * d));

    return (attenuation <= 0.0 || visibility <= 0.0) ? vec3(0.0) : (attenuation * visibility * EvaluateAL(px, L));
}

// 评估单位强度白光聚光灯的贡献
// px: 输入的像素信息结构体
// pos: 聚光灯位置
// dir: 聚光灯方向
// range: 聚光灯范围
// inner_cos: 内锥角的余弦值
// outer_cos: 外锥角的余弦值
vec3 EvaluateASL(const Pixel px, const vec3 pos, const vec3 dir, float range, float inner_cos, float outer_cos) {
    vec3 l = pos - px.position;
    vec3 L = normalize(l);

    // 距离衰减使用简易线性衰减（不遵循反比平方定律）
    float ds = dot(dir, l);  // 投影到聚光灯方向上的距离
    float da = 1.0 - clamp01(ds / range);

    // 角度衰减从内锥角到外锥角逐渐减弱
    float cosine = dot(dir, L);
    float aa = clamp01((cosine - outer_cos) / (inner_cos - outer_cos));
    float attenuation = da * aa;

    return attenuation <= 0.0 ? vec3(0.0) : (EvaluateAL(px, L) * attenuation);
}

/* 评估使用PCSS算法的单个光源的遮挡量

   该函数在线性空间中使用全方位SM，阴影贴图必须是存储线性值的立方体贴图，注意这仅仅是
   一个来自点光源、聚光灯或方向光的软阴影的黑科技，但在实际生活中它们确实应该是硬阴影，
   因为只有面光源才能投射软阴影。

   对于PCF，使用泊松盘采样选择纹素，有利于附近的样本，即使`n_samples`或搜索半径很大，
   它也可以很好地保留阴影形状，这对于均匀盘采样来说并不正确，后者通常会使阴影过度模糊。
   要增加阴影的柔和度，可以使用更大的光半径。

   参考链接：
   https://sites.cs.ucsb.edu/~lingqi/teaching/resources/GAMES202_Lecture_03.pdf
   https://developer.download.nvidia.cn/shaderlibrary/docs/shadow_PCSS.pdf
   https://developer.download.nvidia.cn/whitepapers/2008/PCSS_Integration.pdf
   https://pbr-book.org/3ed-2018/Monte_Carlo_Integration/2D_Sampling_with_Multidimensional_Transformations
*/

// 评估使用PCSS算法的单个光源的遮挡量
// px: 输入的像素信息结构体
// shadow_map: 阴影立方体贴图采样器
// light_pos: 光源位置
// light_radius: 光源半径

float EvalOcclusion(const Pixel px, in samplerCube shadow_map, const vec3 light_pos, float light_radius) {
    const float near_clip = 0.1;
    const float far_clip = 100.0;
    vec3 l = light_pos - px.position;
    vec3 L = normalize(l);
    float depth = length(l) / far_clip;  // 当前像素的线性深度

    // 找到一个切线T和副法线B，使得T、B和L互相正交
    // 有无限多个可能的T和B，但我们可以任意选择它们，因为在单位圆盘上采样是对称的
    // 只需选择一个与L不共线的向量U，然后cross(U, L)将给出T

    vec3 U = mix(vec3(1.0, 0.0, 0.0), vec3(0.0, 1.0, 0.0), step(abs(L.y), 0.999));
    vec3 T = normalize(cross(U, -L));
    vec3 B = normalize(cross(-L, T));

    // 选项1：为每个像素生成16个随机泊松样本（运行时非常慢）
    // const int n_samples = 16;
    // vec2 samples[16];
    // vec3 scale = abs(l);
    // float seed = min3(scale) / max3(scale);
    // PoissonSampleDisk(seed, samples);

    // 选项2：使用预先计算的16个泊松盘样本（速度更快）
    const int n_samples = 16;
    const vec2 samples[16] = vec2[] (  // 参考来源：Nvidia 2008年，《PCSS Integration白皮书》
        vec2(-0.94201624, -0.39906216), vec2( 0.94558609, -0.76890725),
        vec2(-0.09418410, -0.92938870), vec2( 0.34495938,  0.29387760),
        vec2(-0.91588581,  0.45771432), vec2(-0.81544232, -0.87912464),
        vec2(-0.38277543,  0.27676845), vec2( 0.97484398,  0.75648379),
        vec2( 0.44323325, -0.97511554), vec2( 0.53742981, -0.47373420),
        vec2(-0.26496911, -0.41893023), vec2( 0.79197514,  0.19090188),
        vec2(-0.24188840,  0.99706507), vec2(-0.81409955,  0.91437590),
        vec2( 0.19984126,  0.78641367), vec2( 0.14383161, -0.14100790)
    );

    // 样本单位圆盘，计算阻挡者数量和总深度
    float search_radius = light_radius * depth;  // 如果像素很远，则在较大区域内搜索
    float total_depth = 0.0;
    int n_blocker = 0;

    for (int i = 0; i < n_samples; ++i) {
        vec2 offset = samples[i];
        vec3 v = -L + (offset.x * T + offset.y * B) * search_radius;
        float sm_depth = texture(shadow_map, v).r;

        if (depth > sm_depth) {  // 在这一步中我们不需要偏差
            total_depth += sm_depth;
            n_blocker++;
        }
    }

    // 如果未找到阻挡者则提前返回（完全可见）
    if (n_blocker == 0) {
        return 0.0;
    }

    // 计算平均阻挡者深度和半影大小
    float z_blocker = total_depth / float(n_blocker);
    float penumbra = (depth - z_blocker) / z_blocker;  // ~ [0.0, 1.0]

    // 使用PCF计算遮挡，这次使用半影确定核大小
    float PCF_radius = light_radius * penumbra;
    float bias = ComputeDepthBias(L, px.GN);
    float occlusion = 0.0;

    for (int i = 0; i < n_samples; ++i) {
        vec2 offset = samples[i];
        vec3 v = -L + (offset.x * T + offset.y * B) * PCF_radius;
        float sm_depth = texture(shadow_map, v).r;

        if (depth - bias > sm_depth) {
            occlusion += 1.0;
        }
    }

    return occlusion / float(n_samples);
}

#endif
