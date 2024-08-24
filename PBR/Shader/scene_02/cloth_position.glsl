#version 460 core

// 在二维粒子弹簧网格上进行简单的布料模拟
// David 2018, "OpenGL 4 Shading Language Cookbook Third Edition, Chapter 11.4"

#ifdef compute_shader

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;  // 本地工作组大小

layout(std430, binding = 0) readonly buffer InPosition { vec4 p_in[]; };    // 输入位置缓冲
layout(std430, binding = 2) readonly buffer InVelocity { vec4 v_in[]; };    // 输入速度缓冲
layout(std430, binding = 1) writeonly buffer OutPosition { vec4 p_out[]; }; // 输出位置缓冲
layout(std430, binding = 3) writeonly buffer OutVelocity { vec4 v_out[]; }; // 输出速度缓冲

layout(location = 0) uniform vec3 gravity = vec3(0.0, -10, 0.0);  // 重力加速度
layout(location = 1) uniform vec3 wind = vec3(0.0);                // 风力
layout(location = 2) uniform float rh;  // 水平弹簧静止长度
layout(location = 3) uniform float rv;  // 垂直弹簧静止长度
layout(location = 4) uniform float rd;  // 对角弹簧静止长度

const float dt  = 5e-6;    // 时间步长（需要足够小以应对噪声影响）
const float dt2 = 25e-12;  // 时间步长的平方
const float K   = 2000.0;  // 弹簧劲度系数
const float dp  = 0.1;     // 阻尼系数（模拟空气阻力）
const float m   = 0.1;     // 顶点粒子质量
const float im  = 10;      // 质量的倒数，1 / m

shared vec3 share_pos[1024];  // 共享内存，用于加速后续读取操作

void main() {
    uvec3 n_verts = gl_NumWorkGroups * gl_WorkGroupSize;  // 计算总顶点数
    uint n_cols = n_verts.x;  // 列数
    uint n_rows = n_verts.y;  // 行数

    uint col = gl_GlobalInvocationID.x;  // 当前全局索引中的列索引
    uint row = gl_GlobalInvocationID.y;  // 当前全局索引中的行索引

    // 计算顶点在一维数组中的索引，按照行优先顺序，从左下角到右上角
    uint i = n_cols * row + col;
    vec3 p = p_in[i].xyz;  // 当前顶点位置
    vec3 v = v_in[i].xyz;  // 当前顶点速度

    share_pos[i] = p;  // 缓存到共享内存，以加速后续读取
    barrier();  // 确保本地工作组内的所有线程都完成了共享内存的操作

    // 计算受力，包括重力和风力，以及来自相邻顶点的弹簧力
    vec3 force = (gravity + wind) * m;
    vec3 r = vec3(0.0);  // 顶点到相邻顶点的向量

    if (row < n_rows - 1) {  // 上方
        r = share_pos[i + n_cols].xyz - p;
        force += (K * (length(r) - rv)) * normalize(r);
    }
    if (row > 0) {  // 下方
        r = share_pos[i - n_cols].xyz - p;
        force += (K * (length(r) - rv)) * normalize(r);
    }
    if (col > 0) {  // 左方
        r = share_pos[i - 1].xyz - p;
        force += (K * (length(r) - rh)) * normalize(r);
    }
    if (col < n_cols - 1) {  // 右方
        r = share_pos[i + 1].xyz - p;
        force += (K * (length(r) - rh)) * normalize(r);
    }
    if (col > 0 && row < n_rows - 1) {  // 左上方
        r = share_pos[i + n_cols - 1].xyz - p;
        force += (K * (length(r) - rd)) * normalize(r);
    }
    if (col < n_cols - 1 && row < n_rows - 1) {  // 右上方
        r = share_pos[i + n_cols + 1].xyz - p;
        force += (K * (length(r) - rd)) * normalize(r);
    }
    if (col > 0 && row > 0) {  // 左下方
        r = share_pos[i - n_cols - 1].xyz - p;
        force += (K * (length(r) - rd)) * normalize(r);
    }
    if (col < n_cols - 1 && row > 0) {  // 右下方
        r = share_pos[i - n_cols + 1].xyz - p;
        force += (K * (length(r) - rd)) * normalize(r);
    }

    force -= dp * v;  // 考虑阻尼力

    // 应用牛顿运动定律（使用欧拉方法进行积分）
    vec3 a = force * im;  // f = m * a
    p_out[i] = vec4(p + v * dt + 0.5 * a * dt2, 1.0);  // 更新位置
    v_out[i] = vec4(v + a * dt, 0.0);  // 更新速度

    // 四个角的顶点被固定（锚定顶点）
    if ((row == 0 || row == n_rows - 1) && (col == 0 || col == n_cols - 1)) {
        p_out[i] = vec4(p, 1.0);
        v_out[i] = vec4(0.0);
    }
}

#endif
