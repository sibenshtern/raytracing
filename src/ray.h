#ifndef RT_RAY_H
#define RT_RAY_H

#include "vec3.h"

typedef struct {
    vec3 origin;
    vec3 dir;
} Ray;

// Возвращает точку на луче при параметре t.
static inline vec3 ray_at(Ray r, float t) {
    return v3_add(r.origin, v3_scale(r.dir, t));
}

#endif /* RT_RAY_H */
