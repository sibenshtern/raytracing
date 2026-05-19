// Попиксельный трассировщик лучей:
//   первичный луч -> ближайшее пересечение -> Ламберт + ambient + теневой луч
//   если материал отражающий — итеративно прослеживаем до MAX_REFLECTION_DEPTH
//   отскоков при промахе — возвращаем вертикальный градиент неба

#include "render.h"

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Небольшое смещение вдоль нормали поверхности, чтобы избежать самопересечения
// при выпуске теневых и отражённых лучей из точки попадания.
static const float kEps = 1e-4f;

// Максимальная глубина зеркальных отскоков.
#define MAX_REFLECTION_DEPTH 3

static vec3 sky_color(vec3 dir) {
    // Вертикальный градиент: от горизонта (тёплый) к зениту (холодный голубой).
    const float t = 0.5f * (dir.y + 1.0f);
    return v3_lerp(v3(1.0f, 0.95f, 0.85f), v3(0.35f, 0.55f, 0.95f), t);
}

// Диффузный шейдинг в точке попадания: по одному теневому лучу на каждый
// источник.
static vec3 shade_local(const Scene *scene, Hit hit) {
    vec3 color = v3_mul(scene->ambient, hit.material.albedo);

    for (int i = 0; i < scene->n_lights; ++i) {
        const Light *light = &scene->lights[i];
        const vec3 to_light = v3_sub(light->position, hit.point);
        const float dist = v3_length(to_light);
        const vec3 ldir = v3_scale(to_light, 1.0f / dist);

        const Ray shadow_ray = {v3_add(hit.point, v3_scale(hit.normal, kEps)),
                                ldir};
        if (scene_any_hit(scene, shadow_ray, kEps, dist - kEps))
            continue;

        const float lambert = fmaxf(0.0f, v3_dot(hit.normal, ldir));
        const vec3 diff =
            v3_scale(v3_mul(hit.material.albedo, light->color), lambert);
        color = v3_add(color, diff);
    }
    return color;
}

Camera make_camera(int width, int height) {
    return (Camera){
        .origin = v3(0.0f, 1.2f, 4.0f),
        .look_at = v3(0.0f, 0.6f, 0.0f),
        .up = v3(0.0f, 1.0f, 0.0f),
        .vfov_deg = 50.0f,
        .aspect = (float)width / (float)height,
    };
}

vec3 trace_pixel(const Scene *scene, const Camera *cam, int x, int y, int width,
                 int height) {
    // Строится правосторонний базис камеры: forward, right, up.
    const vec3 forward = v3_normalize(v3_sub(cam->look_at, cam->origin));
    const vec3 right = v3_normalize(v3_cross(forward, cam->up));
    const vec3 up = v3_cross(right, forward);

    const float vfov = cam->vfov_deg * (float)M_PI / 180.0f;
    const float half_h = tanf(0.5f * vfov);
    const float half_w = half_h * cam->aspect;

    // Переводится центр пикселя в нормализованные координаты устройства [-1,
    // 1].
    const float u = (2.0f * ((float)x + 0.5f) / (float)width - 1.0f) * half_w;
    const float v = (1.0f - 2.0f * ((float)y + 0.5f) / (float)height) * half_h;

    const vec3 rdir = v3_normalize(
        v3_add(forward, v3_add(v3_scale(right, u), v3_scale(up, v))));
    Ray ray = {cam->origin, rdir};

    // Итеративная трассировка отражений. Поддерживается накопительный цвет и
    // «пропускную способность» текущего луча: на каждом отскоке доля (1-k)
    // оседает локальным шейдингом, доля k уходит дальше по отражённому лучу.
    vec3 color = v3(0.0f, 0.0f, 0.0f);
    float throughput = 1.0f;

    for (int depth = 0; depth < MAX_REFLECTION_DEPTH; ++depth) {
        Hit hit = scene_closest_hit(scene, ray, kEps, 1e4f);
        if (!hit.valid) {
            color = v3_add(color, v3_scale(sky_color(ray.dir), throughput));
            break;
        }

        const vec3 local = shade_local(scene, hit);
        const float k = hit.material.reflectivity;

        color = v3_add(color, v3_scale(local, throughput * (1.0f - k)));

        if (k <= 0.0f)
            break; // Чисто диффузная поверхность — дальше не отражаем.

        throughput *= k;
        if (throughput < 1e-3f)
            break; // Энергия упала ниже видимого порога.

        // Следующий отражённый луч.
        const vec3 r_dir = v3_reflect(ray.dir, hit.normal);
        ray = (Ray){v3_add(hit.point, v3_scale(hit.normal, kEps)), r_dir};
    }

    return v3(sqrtf(fmaxf(0.0f, color.x)), sqrtf(fmaxf(0.0f, color.y)),
              sqrtf(fmaxf(0.0f, color.z)));
}
