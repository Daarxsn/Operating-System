#ifndef XYRIS_ABI_VERSION_H
#define XYRIS_ABI_VERSION_H

/* Xyris System ABI version 0.1. */
#define XYRIS_ABI_MAJOR 0u
#define XYRIS_ABI_MINOR 1u
#define XYRIS_ABI_VERSION ((XYRIS_ABI_MAJOR << 16) | XYRIS_ABI_MINOR)
#define XYRIS_ABI_STRUCT_VERSION_1 1u

/* Compatibility helpers. Compare ABI major versions first. */
#define XYRIS_ABI_VERSION_MAJOR(v) (((v) >> 16) & 0xffffu)
#define XYRIS_ABI_VERSION_MINOR(v) ((v) & 0xffffu)
#define XYRIS_ABI_MAJOR_COMPATIBLE(provider, consumer) \
    (XYRIS_ABI_VERSION_MAJOR(provider) == XYRIS_ABI_VERSION_MAJOR(consumer))
#define XYRIS_ABI_MINOR_SATISFIES(provider, required) \
    (XYRIS_ABI_VERSION_MINOR(provider) >= XYRIS_ABI_VERSION_MINOR(required))

#endif /* XYRIS_ABI_VERSION_H */
