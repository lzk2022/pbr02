#ifndef _RENDERER_H
#define _RENDERER_H

// 渲染器输入统一块，用于存储渲染器输入的统一数据
layout(std140, binding = 10) uniform RendererInput {
    ivec2 resolution;     // 视口大小，以像素为单位
    ivec2 cursor_pos;     // 光标位置相对于视口左上角的位置
    float near_clip;      // 视锥体近裁剪面距离
    float far_clip;       // 视锥体远裁剪面距离
    float time;           // 自窗口创建以来的秒数
    float delta_time;     // 自上一帧以来的秒数
    bool  depth_prepass;  // 提前深度测试
    uint  shadow_index;   // 阴影贴图渲染通道的索引（每个光源一个索引）
} rdr_in;

// 默认块（宽松）统一位置 >= 1000 仅供内部使用
struct self_t {
    mat4 transform;    // 1000，当前实体的模型矩阵
    uint material_id;  // 1001，当前网格的材质ID
    uint ext_1002;
    uint ext_1003;
    uint ext_1004;
    uint ext_1005;
    uint ext_1006;
    uint ext_1007;
};

layout(location = 1000) uniform self_t self;

#endif
