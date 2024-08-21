// 将(2:1)的等矩形贴图转换为(1:1 x 6)的立方体贴图贴图

#ifdef compute_shader

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;  // 设置本地工作组大小
layout(binding = 0) uniform sampler2D equirectangle;  // 绑定输入的等矩形贴图
layout(binding = 0, rgba16f) restrict writeonly uniform imageCube cubemap;  // 绑定输出的立方体贴图

// 引入投影相关的GLSL函数
#include "../utils/projection.glsl"

// 主函数，将等矩形贴图转换为立方体贴图的一个面
void main() {
    vec2 resolution = vec2(imageSize(cubemap));  // 获取立方体贴图的分辨率
    ivec3 ils_coordinate = ivec3(gl_GlobalInvocationID);  // 获取当前线程的全局ID作为ILS坐标

    vec3 v = ILS2Cartesian(ils_coordinate, resolution);  // 将ILS坐标转换为笛卡尔坐标系中的向量

    vec2 sample_vec = Cartesian2Spherical(v);  // 将笛卡尔坐标转换为球坐标
    sample_vec = Spherical2Equirect(sample_vec);  // 将球坐标转换为等矩形贴图的采样坐标
    vec4 color = texture(equirectangle, sample_vec);  // 从等矩形贴图中采样颜色值

    imageStore(cubemap, ils_coordinate, color);  // 将采样到的颜色存储到立方体贴图对应的ILS坐标处
}

#endif
