// Построение сцены и процедуры пересечения луча с примитивами.

#include "scene.h"

#include <math.h>

// Пересечение луча и сферы: решаем |o + t*d - c|^2 = r^2 — квадратное уравнение относительно t.
// Возвращает ближайший положительный корень в интервале (tmin, tmax) либо отрицательное число при промахе.
static float intersect_sphere(const Sphere *s, Ray r, float tmin, float tmax)
{
    const vec3 oc = v3_sub(r.origin, s->center);
    const float a = v3_dot(r.dir, r.dir);
    const float half_b = v3_dot(oc, r.dir);
    const float c = v3_dot(oc, oc) - s->radius * s->radius;
    const float disc = half_b * half_b - a * c;
    if (disc < 0.0f)
        return -1.0f;
    const float sd = sqrtf(disc);
    float t = (-half_b - sd) / a; // Сначала пробуем ближний корень, если не подходит, то дальний.
    if (t < tmin || t > tmax)
        t = (-half_b + sd) / a;
    if (t < tmin || t > tmax)
        return -1.0f;
    return t;
}

// Пересечение луча с плоскостью (y = y0). Возвращает t в (tmin, tmax) или отрицательное число при промахе.
static float intersect_plane(const Plane *p, Ray r, float tmin, float tmax)
{
    if (fabsf(r.dir.y) < 1e-6f)
        return -1.0f; // Луч параллелен плоскости.
    const float t = (p->y0 - r.origin.y) / r.dir.y;
    if (t < tmin || t > tmax)
        return -1.0f;
    return t;
}

Scene scene_build(void)
{
    Scene s = {0};
    s.ambient = v3(0.06f, 0.07f, 0.10f);

    // Центральная зеркальная сфера — главный объект. Отражает соседние сферы
    // s.spheres[s.n_spheres++] = (Sphere){
    //     .center = v3(0.0f, 1.0f, -1.0f), .radius = 1.0f, .material = {.albedo = v3(0.92f, 0.92f, 0.95f), .reflectivity = 0.90f, .checker = false}};

    // Тёплый «солнечный» спутник слева, диффузный.
    s.spheres[s.n_spheres++] = (Sphere){
        .center = v3(-1.9f, 0.6f, 0.3f), .radius = 0.6f, .material = {.albedo = v3(0.95f, 0.55f, 0.20f), .reflectivity = 0.0f, .checker = false}};

    // Глянцевая золотая сфера, частично отражающая.
    s.spheres[s.n_spheres++] = (Sphere){
        .center = v3(-0.9f, 0.35f, 0.8f), .radius = 0.35f, .material = {.albedo = v3(0.95f, 0.80f, 0.35f), .reflectivity = 0.55f, .checker = false}};

    // Холодная бирюзовая диффузная сфера перед зеркалом.
    s.spheres[s.n_spheres++] = (Sphere){
        .center = v3(0.1f, 0.30f, 1.2f), .radius = 0.30f, .material = {.albedo = v3(0.15f, 0.75f, 0.65f), .reflectivity = 0.0f, .checker = false}};

    // Тёмно-красная сфера, слегка отражающая.
    s.spheres[s.n_spheres++] = (Sphere){
        .center = v3(1.2f, 0.45f, 0.6f), .radius = 0.45f, .material = {.albedo = v3(0.85f, 0.18f, 0.20f), .reflectivity = 0.15f, .checker = false}};

    // Высокая синяя сфера справа.
    s.spheres[s.n_spheres++] = (Sphere){
        .center = v3(2.3f, 0.75f, -0.2f), .radius = 0.75f, .material = {.albedo = v3(0.20f, 0.40f, 0.95f), .reflectivity = 0.0f, .checker = false}};

    // Маленький хромовый шарик впереди, у самой земли.
    s.spheres[s.n_spheres++] = (Sphere){
        .center = v3(-0.3f, 0.20f, 1.7f), .radius = 0.20f, .material = {.albedo = v3(0.90f, 0.90f, 0.90f), .reflectivity = 0.95f, .checker = false}};

    // Пурпурная сфера за зеркалом, чуть выглядывает.
    s.spheres[s.n_spheres++] = (Sphere){
        .center = v3(1.8f, 0.40f, -1.8f), .radius = 0.40f, .material = {.albedo = v3(0.85f, 0.25f, 0.75f), .reflectivity = 0.0f, .checker = false}};

    // Дальняя хромовая сфера слева — даёт зеркалу что отражать.
    s.spheres[s.n_spheres++] = (Sphere){
        .center = v3(-3.0f, 0.50f, -1.5f), .radius = 0.50f, .material = {.albedo = v3(0.85f, 0.90f, 0.85f), .reflectivity = 0.80f, .checker = false}};

    // Пол — плоскость с шахматным узором.
    s.planes[s.n_planes++] = (Plane){
        .y0 = 0.0f,
        .material = {.albedo = v3(0.85f, 0.85f, 0.88f), .reflectivity = 0.0f, .checker = true}};

    // Источник света
    s.lights[s.n_lights++] = (Light){
        .position = v3(4.0f, 6.0f, 3.0f), .color = v3(1.00f, 0.92f, 0.78f)};
    return s;
}

Hit scene_closest_hit(const Scene *scene, Ray r, float tmin, float tmax)
{
    Hit best = {0};
    float best_t = tmax;

    for (int i = 0; i < scene->n_spheres; ++i)
    {
        const Sphere *sp = &scene->spheres[i];
        const float t = intersect_sphere(sp, r, tmin, best_t);
        if (t > 0.0f)
        {
            best_t = t;
            best.valid = true;
            best.t = t;
            best.point = ray_at(r, t);
            best.normal = v3_normalize(v3_sub(best.point, sp->center));
            best.material = sp->material;
        }
    }

    for (int i = 0; i < scene->n_planes; ++i)
    {
        const Plane *pl = &scene->planes[i];
        const float t = intersect_plane(pl, r, tmin, best_t);
        if (t > 0.0f)
        {
            best_t = t;
            best.valid = true;
            best.t = t;
            best.point = ray_at(r, t);
            best.normal = v3(0.0f, 1.0f, 0.0f);
            best.material = pl->material;
            if (pl->material.checker)
            {
                const int cx = (int)floorf(best.point.x);
                const int cz = (int)floorf(best.point.z);
                if (((cx + cz) & 1) == 0)
                {
                    best.material.albedo = v3(0.20f, 0.20f, 0.22f);
                }
            }
        }
    }

    return best;
}

bool scene_any_hit(const Scene *scene, Ray r, float tmin, float tmax)
{
    for (int i = 0; i < scene->n_spheres; ++i)
    {
        if (intersect_sphere(&scene->spheres[i], r, tmin, tmax) > 0.0f)
            return true;
    }
    for (int i = 0; i < scene->n_planes; ++i)
    {
        if (intersect_plane(&scene->planes[i], r, tmin, tmax) > 0.0f)
            return true;
    }
    return false;
}
