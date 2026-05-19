// Примитивы сцены (сфера, бесконечная плоскость), материалы и контейнер Scene.
// scene_closest_hit() возвращает ближайшее пересечение луча в интервале (tmin,
// tmax).

#ifndef RT_SCENE_H
#define RT_SCENE_H

#include <stdbool.h>

#include "ray.h"
#include "vec3.h"

#define MAX_SPHERES 16
#define MAX_PLANES 4
#define MAX_LIGHTS 4

typedef struct {
    vec3 albedo;
    float reflectivity; // 0 — чистый диффуз, 1 — идеальное зеркало.
    bool checker;       // Процедурный шахматный узор (используется для пола).
} Material;

typedef struct {
    vec3 center;
    float radius;
    Material material;
} Sphere;

typedef struct {
    float y0;
    Material material;
} Plane;

typedef struct {
    vec3 position;
    vec3 color;
} Light;

typedef struct {
    Sphere spheres[MAX_SPHERES];
    int n_spheres;
    Plane planes[MAX_PLANES];
    int n_planes;
    Light lights[MAX_LIGHTS];
    int n_lights;
    vec3 ambient;
} Scene;

typedef struct {
    float t; // Параметр луча в точке пересечения (валиден только если `valid`).
    vec3 point;        // Положение точки попадания в мировых координатах.
    vec3 normal;       // Единичная внешняя нормаль в точке пересечения.
    Material material; // Копия материала примитива, в который попали.
    bool valid;        // true, если в (tmin, tmax) было найдено пересечение.
} Hit;

Scene scene_build(void);

// Ближайшее пересечение луча `r` в интервале (tmin, tmax). При промахе
// hit.valid == false.
Hit scene_closest_hit(const Scene *scene, Ray r, float tmin, float tmax);

bool scene_any_hit(const Scene *scene, Ray r, float tmin, float tmax);

#endif /* RT_SCENE_H */
