// Попиксельная трассировка лучей. Возвращает гамма-кодированный линейный
// RGB-цвет в диапазоне [0, 1]^3 для пикселя с целочисленными координатами (x,
// y) изображения (w, h).

#ifndef RT_RENDER_H
#define RT_RENDER_H

#include "scene.h"
#include "vec3.h"

typedef struct {
    vec3 origin;
    vec3 look_at;
    vec3 up;
    float vfov_deg;
    float aspect;
} Camera;

Camera make_camera(int width, int height);

// Выпускает один первичный луч через пиксель (x, y) и возвращает его затенённый
// цвет.
vec3 trace_pixel(const Scene *scene, const Camera *cam, int x, int y, int width,
                 int height);

#endif /* RT_RENDER_H */
