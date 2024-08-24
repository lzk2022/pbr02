#version 460 core

// 计算布料格点顶点的法向量
// David 2018, "OpenGL 4 Shading Language Cookbook Third Edition, Chapter 11.4"

#ifdef compute_shader

layout(local_size_x = 32, local_size_y = 32, local_size_z = 1) in;

layout(std430, binding = 0) readonly buffer InPosition { vec4 position[]; }; // 输入位置缓冲
layout(std430, binding = 4) writeonly buffer OutNormal { vec4 normal[]; };   // 输出法向量缓冲

void main() {
    uvec3 n_verts = gl_NumWorkGroups * gl_WorkGroupSize; // 计算总顶点数
    uint n_cols = n_verts.x;  // 列数
    uint n_rows = n_verts.y;  // 行数

    uint col = gl_GlobalInvocationID.x; // 当前工作组中的列索引
    uint row = gl_GlobalInvocationID.y; // 当前工作组中的行索引

    // 计算顶点在一维数组中的索引，按照行优先顺序，从左下角到右上角
    uint i = n_cols * row + col;
    vec3 p = vec3(position[i]); // 当前顶点位置
    vec3 n = vec3(0.0); // 初始化法向量为零向量
    vec3 a, b, c;

    // 每个顶点的法向量通过其相邻顶点之间的向量来计算
    // 对于每个顶点，有三个相邻向量a、b和c，计算(a, b)和(b, c)的叉积可以得到两个局部法线方向。
    // 将所有的局部法线方向进行累加（平均），然后进行归一化，得到全局顶点法线向量。

    if (row < n_rows - 1) {
        c = position[i + n_cols].xyz - p;

        if (col < n_cols - 1) {
            a = position[i + 1].xyz - p;
            b = position[i + n_cols + 1].xyz - p;
            n += cross(a, b);
            n += cross(b, c);
        }

        if (col > 0) {
            a = c;
            b = position[i + n_cols - 1].xyz - p;
            c = position[i - 1].xyz - p;
            n += cross(a, b);
            n += cross(b, c);
        }
    }

    if (row > 0) {
        c = position[i - n_cols].xyz - p;

        if (col > 0) {
            a = position[i - 1].xyz - p;
            b = position[i - n_cols - 1].xyz - p;
            n += cross(a, b);
            n += cross(b, c);
        }

        if (col < n_cols - 1) {
            a = c;
            b = position[i - n_cols + 1].xyz - p;
            c = position[i + 1].xyz - p;
            n += cross(a, b);
            n += cross(b, c);
        }
    }

    normal[i] = vec4(normalize(n), 0.0); // 存储归一化的法向量到输出缓冲
}

#endif
