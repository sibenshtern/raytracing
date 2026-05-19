#ifndef RT_IMAGE_IO_H
#define RT_IMAGE_IO_H

#include <stdbool.h>
#include <stdint.h>

// Записывает RGB8-буфер размера (width * height * 3) байт в файл `path` как
// PNG.
bool save_png(const char *path, int width, int height, const uint8_t *rgb);

#endif /* RT_IMAGE_IO_H */
