#ifndef RAMFS_H
#define RAMFS_H

#include "./vfs.h"
#include <stdint.h>
#include <stddef.h>

#define RAMFS_NAME_MAX 64

typedef struct ramfs_node
{
    vnode_t vnode;
    uint8_t* data;
} ramfs_node_t;

typedef struct
{
    ramfs_node_t* root;
} ramfs_t;

extern filesystem_t ramfs_filesystem;

int ramfs_mount(void);
int ramfs_unmount(void);

ramfs_node_t* ramfs_create_node(const char* name, vfs_node_type_t type);
int ramfs_add_child(ramfs_node_t* parent, ramfs_node_t* child);
ramfs_node_t* ramfs_find_child(ramfs_node_t* parent, const char* name);
ramfs_node_t* ramfs_lookup(const char* path);
ramfs_node_t* ramfs_create_directory(const char* path);
ramfs_node_t* ramfs_create_file(const char* path);

#endif
