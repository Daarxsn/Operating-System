#ifndef VFS_H
#define VFS_H

#include <stdint.h>
#include <stddef.h>

#define VFS_NAME_MAX 64
#define VFS_MAX_MOUNTS 16

typedef enum
{
    VFS_NODE_FILE = 0,
    VFS_NODE_DIRECTORY
} vfs_node_type_t;

struct vnode;
struct filesystem;

typedef struct vnode
{
    char name[VFS_NAME_MAX];
    vfs_node_type_t type;
    size_t size;
    struct vnode* parent;
    struct vnode* children;
    struct vnode* next;
    void* private_data;
} vnode_t;

typedef struct filesystem
{
    const char* name;
    int (*mount)(void);
    int (*unmount)(void);
    vnode_t* (*lookup)(const char* path);
    vnode_t* root;
} filesystem_t;

typedef struct mount_point
{
    char path[VFS_NAME_MAX];
    filesystem_t* fs;
} mount_point_t;

void vfs_init(void);
int vfs_register_filesystem(filesystem_t* fs);
int vfs_mount(const char* path, filesystem_t* fs);
int vfs_unmount(const char* path);
vnode_t* vfs_lookup(const char* path);

#endif
