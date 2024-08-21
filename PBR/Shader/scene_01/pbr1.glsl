#version 460 core
// �����Ż�ָ��
#pragma optimize(off)

layout(std140, binding = 0) uniform Camera {
    vec4 position;      // ���λ��
    vec4 direction;     // �������
    mat4 view;          // ��ͼ����
    mat4 projection;    // ͶӰ����
} camera;
// ������Ⱦ�������ļ�
#include "../core/renderer_input.glsl"

////////////////////////////////////////////////////////////////////////////////

#ifdef vertex_shader

// ������ɫ��
layout(location = 0) in vec3 position;    // ����λ������
layout(location = 1) in vec3 normal;      // ���㷨������
layout(location = 2) in vec2 uv;          // ������������
layout(location = 3) in vec2 uv2;         // �ڶ���������������
layout(location = 4) in vec3 tangent;     // ��������
layout(location = 5) in vec3 binormal;    // ˫��������

layout(location = 0) out _vtx {
    out vec3 _position;     // ���ݵ�Ƭ����ɫ���Ķ���λ��
    out vec3 _normal;       // ���ݵ�Ƭ����ɫ���Ķ��㷨��
    out vec2 _uv;           // ���ݵ�Ƭ����ɫ������������
    out vec2 _uv2;          // ���ݵ�Ƭ����ɫ���ĵڶ�����������
    out vec3 _tangent;      // ���ݵ�Ƭ����ɫ��������
    out vec3 _binormal;     // ���ݵ�Ƭ����ɫ����˫����
};

void main() {
    // ���㶥��Ĳü��ռ�����
    gl_Position = camera.projection * camera.view * self.transform * vec4(position, 1.0);

    // ������λ�á����ߡ����ߡ�˫���ߡ��������괫�ݸ���һ�׶�
    _position = vec3(self.transform * vec4(position, 1.0));
    _normal   = normalize(vec3(self.transform * vec4(normal, 0.0)));
    _tangent  = normalize(vec3(self.transform * vec4(tangent, 0.0)));
    _binormal = normalize(vec3(self.transform * vec4(binormal, 0.0)));
    _uv = uv;
}

#endif

////////////////////////////////////////////////////////////////////////////////


#ifdef fragment_shader

// Ƭ����ɫ���� ����PBRͳһ�������塢 ����PBR��ɫ����
#include "../core/pbr_uniform.glsl"
#include "../core/pbr_shading.glsl"

layout(location = 0) in _vtx {
    in vec3 _position;   // ����λ������
    in vec3 _normal;     // ���㷨������
    in vec2 _uv;         // ������������
    in vec2 _uv2;        // �ڶ���������������
    in vec3 _tangent;    // ��������
    in vec3 _binormal;   // ˫��������
};

layout(location = 0) out vec4 color;   // �����ɫ
layout(location = 1) out vec4 bloom;   // �������

layout(std430, binding = 0) readonly buffer Color    { vec4  pl_color[];    };   // ��Դ��ɫ����
layout(std430, binding = 1) readonly buffer Position { vec4  pl_position[]; };   // ��Դλ�û���
layout(std430, binding = 2) readonly buffer Range    { float pl_range[];    };   // ��Դ��Χ����
layout(std430, binding = 3) readonly buffer Index    { int   pl_index[];    };   // ��Դ��������

layout(std140, binding = 1) uniform DL {
    vec4  color;         // �������ɫ��ǿ��
    vec4  direction;     // ����ⷽ��
    float intensity;     // �����ǿ��
} dl;

layout(std140, binding = 2) uniform SL {
    vec4  color;         // �۹����ɫ��ǿ��
    vec4  position;      // �۹��λ��
    vec4  direction;     // �۹�Ʒ���
    float intensity;     // �۹��ǿ��
    float inner_cos;     // ��Բ׶������ֵ
    float outer_cos;     // ��Բ׶������ֵ
    float range;         // �۹�Ʒ�Χ
} sl;

layout(std140, binding = 3) uniform OL {
    vec4  color;         // �������ɫ
    vec4  position;      // �����λ��
    float intensity;     // �����ǿ��
    float linear;        // ���������˥��ϵ��
    float quadratic;     // ��������˥��ϵ��
    float range;         // ����ⷶΧ
} ol;

layout(std140, binding = 4) uniform PL {
    float intensity;     // ���Դǿ��
    float linear;        // ���Դ����˥��ϵ��
    float quadratic;     // ���Դ����˥��ϵ��
} pl;

const uint n_pls = 28;      // ��Դ����
const uint tile_size = 16;  // ��Ƭ��С

// ��ȡ��ǰ����������Ƭ��ƫ����������ʼ����
uint GetTileOffset() {
    ivec2 tile_id = ivec2(gl_FragCoord.xy) / ivec2(tile_size);   // ������ƬID
    uint n_cols = rdr_in.resolution.x / tile_size;               // ����ÿ�е���Ƭ��
    uint tile_index = tile_id.y * n_cols + tile_id.x;            // ������Ƭ����
    return tile_index * n_pls;                                   // ������Ƭ�����������е���ʼƫ����
}

void main() {
    // �����Ԥͨ���У���ִ���κλ��Ʋ���
    if (rdr_in.depth_prepass) {
        return;
    }

    Pixel px;
    px._position = _position;   // ��������λ��
    px._normal   = _normal;     // �������ط���
    px._uv       = _uv;         // ����������������
    px._has_tbn  = true;        // �����Ƿ�������߿ռ�

    InitPixel(px, camera.position.xyz);   // ��ʼ������

    vec3 Lo = vec3(0.0);    // ������⹱��
    vec3 Le = vec3(0.0);    // �Է��⹱��

    // ���㷽���Ĺ���
    Lo += EvaluateADL(px, dl.direction.xyz, 1.0) * dl.color.rgb * dl.intensity;

    // ����������ֵ�Ͳ�Ĺ���
    vec3 sc = EvaluateASL(px, sl.position.xyz, sl.direction.xyz, sl.range, sl.inner_cos, sl.outer_cos);
    Lo += sc * sl.color.rgb * sl.intensity;

    // ��������Ĺ���
    vec3 oc = EvaluateAPL(px, ol.position.xyz, ol.range, ol.linear, ol.quadratic, 1.0);
    Lo += oc * ol.color.rgb * ol.intensity;

    // ������Դ x 28 �Ĺ��ף����δ���޳��ҿɼ���
    uint offset = GetTileOffset();
    for (uint i = 0; i < n_pls && pl_index[offset + i] != -1; ++i) {
        int index = pl_index[offset + i];
        vec3 pc = EvaluateAPL(px, pl_position[index].xyz, pl_range[index], pl.linear, pl.quadratic, 1.0);
        Lo += pc * pl_color[index].rgb * pl.intensity;
    }

    // �������IDΪ6������̨ƽ̨���⣩�������Է��⹱��
    if (self.material_id == 6) {
        Le = CircularEaseInOut(abs(sin(rdr_in.time * 2.0))) * px.emission.rgb;
    }

    color = vec4(Lo + Le, px.albedo.a);  // ���������ɫ
    bloom = vec4(Le, 1.0);               // ���÷������
}

#endif
