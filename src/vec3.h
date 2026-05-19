// Минимальный 3D-вектор и операции над ним, используемые во всём рендерере.
#ifndef RT_VEC3_H
#define RT_VEC3_H

#include <math.h>

typedef struct {
    float x, y, z;
} vec3;

static inline vec3 v3(float x, float y, float z) { return (vec3){x, y, z}; }
static inline vec3 v3_add(vec3 a, vec3 b) {
    return (vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}
static inline vec3 v3_sub(vec3 a, vec3 b) {
    return (vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}
static inline vec3 v3_scale(vec3 a, float s) {
    return (vec3){a.x * s, a.y * s, a.z * s};
}
static inline vec3 v3_mul(vec3 a, vec3 b) {
    return (vec3){a.x * b.x, a.y * b.y, a.z * b.z};
}
static inline float v3_dot(vec3 a, vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline vec3 v3_cross(vec3 a, vec3 b) {
    return (vec3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
                  a.x * b.y - a.y * b.x};
}

static inline float v3_length(vec3 a) { return sqrtf(v3_dot(a, a)); }
static inline vec3 v3_normalize(vec3 a) {
    return v3_scale(a, 1.0f / v3_length(a));
}

static inline vec3 v3_lerp(vec3 a, vec3 b, float t) {
    return v3_add(v3_scale(a, 1.0f - t), v3_scale(b, t));
}

// Отражение направления d от поверхности с единичной нормалью n: r = d -
// 2(d·n)n.
static inline vec3 v3_reflect(vec3 d, vec3 n) {
    return v3_sub(d, v3_scale(n, 2.0f * v3_dot(d, n)));
}

#endif /* RT_VEC3_H */
