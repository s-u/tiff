#ifndef PKG_TIFF_COMMON_H__
#define PKG_TIFF_COMMON_H__

#include <stdio.h>
#include <tiff.h>
#include <tiffio.h>

typedef struct tiff_job {
    FILE *f;
    long ptr, len, alloc;
    char *data;
} tiff_job_t;

TIFF *TIFF_Open(const char *mode, tiff_job_t *rj);

#endif
