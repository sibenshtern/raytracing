// MPI-драйвер для трассировщика лучей.
//
// Изображение разрезается на непрерывные горизонтальные полосы строк. Каждый
// MPI-ранг рендерит свою полосу в локальный буфер RGB8, затем MPI_Gatherv
// собирает все полосы на ранге 0, который записывает out.png через libpng

#include <mpi.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "image_io.h"
#include "render.h"
#include "scene.h"
#include "vec3.h"

typedef struct {
    int width;
    int height;
    const char *out;
} Args;

static Args parse_args(int argc, char **argv) {
    Args a = {.width = 1200, .height = 800, .out = "out.png"};
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--w") == 0 && i + 1 < argc) {
            a.width = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--h") == 0 && i + 1 < argc) {
            a.height = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--o") == 0 && i + 1 < argc) {
            a.out = argv[++i];
        }
    }
    return a;
}

static inline uint8_t to_u8(float c) {
    if (c < 0.0f)
        c = 0.0f;
    if (c > 1.0f)
        c = 1.0f;
    return (uint8_t)(c * 255.0f + 0.5f);
}

int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    int rank = 0, nproc = 1;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    const Args args = parse_args(argc, argv);
    const int W = args.width;
    const int H = args.height;

    // Разбиение по полосам строк: ранг r обрабатывает строки [r*H/nproc,
    // (r+1)*H/nproc)
    const int row_start = (rank * H) / nproc;
    const int row_end = ((rank + 1) * H) / nproc;
    const int local_rows = row_end - row_start;

    // Сцена и камера строятся локально на каждом ранге
    const Scene scene = scene_build();
    const Camera camera = make_camera(W, H);

    const size_t local_bytes = (size_t)local_rows * (size_t)W * 3u;
    uint8_t *local = (uint8_t *)malloc(local_bytes > 0 ? local_bytes : 1u);

    int *recvcounts = (int *)malloc((size_t)nproc * sizeof(int));
    int *displs = (int *)malloc((size_t)nproc * sizeof(int));
    for (int r = 0; r < nproc; ++r) {
        const int rs = (r * H) / nproc;
        const int re = ((r + 1) * H) / nproc;
        recvcounts[r] = (re - rs) * W * 3;
        displs[r] = rs * W * 3;
    }

    uint8_t *full = NULL;
    if (rank == 0) {
        full = (uint8_t *)malloc((size_t)H * (size_t)W * 3u);
    }

    MPI_Barrier(MPI_COMM_WORLD);
    const double t0 = MPI_Wtime();

    for (int y = row_start; y < row_end; ++y) {
        for (int x = 0; x < W; ++x) {
            const vec3 color = trace_pixel(&scene, &camera, x, y, W, H);
            const size_t off =
                (size_t)(y - row_start) * (size_t)W * 3u + (size_t)x * 3u;
            local[off + 0] = to_u8(color.x);
            local[off + 1] = to_u8(color.y);
            local[off + 2] = to_u8(color.z);
        }
    }

    // Собираем полосы со всех рангов в буфер `full` на ранге 0.
    MPI_Gatherv(local, local_rows * W * 3, MPI_UNSIGNED_CHAR,
                rank == 0 ? full : NULL, recvcounts, displs, MPI_UNSIGNED_CHAR,
                0, MPI_COMM_WORLD);

    MPI_Barrier(MPI_COMM_WORLD);
    const double t1 = MPI_Wtime();

    if (rank == 0) {
        printf("render_time=%.6f\n", t1 - t0);
        printf("info: nproc=%d width=%d height=%d\n", nproc, W, H);

        if (!save_png(args.out, W, H, full)) {
            fprintf(stderr, "error: failed to write %s\n", args.out);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        printf("wrote %s\n", args.out);
    }

    free(local);
    free(recvcounts);
    free(displs);
    free(full);

    MPI_Finalize();
    return 0;
}
