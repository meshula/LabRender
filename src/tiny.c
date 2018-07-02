
#define TINYALLOC_IMPLEMENTATION
#include "tinyheaders/tinyalloc.h"
#define TP_ALLOC(size) TINYALLOC_ALLOC(size, 0)
#define TP_FREE(ptr) TINYALLOC_FREE(ptr, 0)
#define TP_CALLOC(count, element_size) TINYALLOC_CALLOC(count, element_size, 0)

// TinyPng has a texture altas in it which is handy
#define TINYPNG_IMPLEMENTATION
#include "tinyheaders/tinypng.h"

#define SPRITEBATCH_MALLOC(size, ctx) TINYALLOC_ALLOC(size, ctx)
#define SPRITEBATCH_FREE(mem, ctx) TINYALLOC_FREE(mem, ctx)
#define SPRITEBATCH_IMPLEMENTATION
#include "tinyheaders/tinyspritebatch.h"
