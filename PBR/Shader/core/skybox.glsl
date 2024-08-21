#version 460 core

layout(std140, binding = 0) uniform Camera {
    vec4 position;      // 相机位置
    vec4 direction;     // 相机方向
    mat4 view;          // 视图矩阵
    mat4 projection;    // 投影矩阵
} camera;
// 引入渲染器输入统一块
#include "../core/renderer_input.glsl"

////////////////////////////////////////////////////////////////////////////////

#ifdef vertex_shader

layout(location = 0) in vec3 position;    // 顶点位置输入
layout(location = 0) out vec3 _tex_coords; // 传递给片段着色器的纹理坐标

void main() {
    // 天空盒的纹理坐标有三个维度u、v、w，大致等于其位置（因为天空盒立方体以原点为中心）
    _tex_coords = position;

    // 天空盒是静止的，不随相机移动，因此需要使用修正后的视图矩阵，去除了其平移分量
    mat4 rectified_view = mat4(mat3(camera.view));
    vec4 pos = camera.projection * rectified_view * self.transform * vec4(position, 1.0);

    // 使用分量重新排列的技巧确保天空盒的深度值在/w除法后始终为1
    // 这样它在场景中具有最远的距离，并且将在所有其他对象的后面进行渲染
    gl_Position = pos.xyww;
}

#endif

////////////////////////////////////////////////////////////////////////////////

#ifdef fragment_shader

layout(location = 0) in vec3 _tex_coords; // 从顶点着色器传入的纹理坐标
layout(location = 0) out vec4 color;      // 输出的颜色
layout(location = 1) out vec4 bloom;      // 输出的发光

layout(binding = 0) uniform samplerCube skybox; // 天空盒纹理

// 注意，同一张HDRI图像在不同应用或甚至场景中看起来可能不同
// 这是因为某些色调映射算子依赖于场景的最大或平均亮度
// 这可能根据视口中像素的亮度略有变化。
// 如果调色后天空盒看起来太暗，可以使用uniform `exposure`调整其亮度
// 在应用色调映射之前。

layout(location = 0) uniform float exposure = 1.0;  // 曝光度
layout(location = 1) uniform float lod = 0.0;        // 天空盒LOD级别

void main() {
    if (rdr_in.depth_prepass) {
        return;  // 在深度预通道中，片段着色器不绘制任何内容
    }

    const float max_level = textureQueryLevels(skybox) - 1.0;
    vec3 irradiance = textureLod(skybox, _tex_coords, clamp(lod, 0.0, max_level)).rgb;
    color = vec4(irradiance * exposure, 1.0);

    // 如果第二个MRT没有启用，bloom将写入GL_NONE并被丢弃
    bloom = vec4(0.0, 0.0, 0.0, 1.0);  // 确保天空盒不会发光
}

#endif
