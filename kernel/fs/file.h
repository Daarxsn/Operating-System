#ifndef FILE_H
#define FILE_H

#include "vfs.h"

#include <stddef.h>
#include <stdint.h>

#define MAX_OPEN_FILES 128

typedef struct file
{
    int fd;

    vnode_t* node;

    size_t offset;

    uint32_t flags;

    int used;

} file_t;

void file_init(void);

int open(const char* path);

int read(
    int fd,
    void* buffer,
    size_t size
);

int write(
    int fd,
    const void* buffer,
    size_t size
);

int close(int fd);

#endif