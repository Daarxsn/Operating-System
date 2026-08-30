#ifndef XYRIS_ABI_VERSION_H
#define XYRIS_ABI_VERSION_H

/*
 * Xyris System ABI version 0.1.
 *
 * The ABI version identifies the public binary contract. It is separate
 * from the SDK/API version and from an individual application's version.
 */
#define XYRIS_ABI_MAJOR 0u
#define XYRIS_ABI_MINOR 1u
#define XYRIS_ABI_VERSION ((XYRIS_ABI_MAJOR << 16) | XYRIS_ABI_MINOR)

/* Version of the common extensible-structure header. */
#define XYRIS_ABI_STRUCT_VERSION_1 1u

#endif /* XYRIS_ABI_VERSION_H */
