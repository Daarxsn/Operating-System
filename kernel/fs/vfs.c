#include "vfs.h"
#include <string.h>
#include <stdbool.h>

static filesystem_t* registered_filesystems[VFS_MAX_MOUNTS];
static mount_point_t mounts[VFS_MAX_MOUNTS];
static size_t filesystem_count;
static size_t mount_count;

void vfs_init(void)
{
    filesystem_count = 0;
    mount_count = 0;
    memset(registered_filesystems, 0, sizeof(registered_filesystems));
    memset(mounts, 0, sizeof(mounts));
}

int vfs_register_filesystem(filesystem_t* fs)
{
    if (!fs || !fs->name || filesystem_count >= VFS_MAX_MOUNTS)
        return -1;

    for (size_t i = 0; i < filesystem_count; ++i)
        if (registered_filesystems[i] == fs ||
            strcmp(registered_filesystems[i]->name, fs->name) == 0)
            return -1;

    registered_filesystems[filesystem_count++] = fs;
    return 0;
}

static int mount_path_valid(const char* path)
{
    return path && path[0] == '/' && strlen(path) < VFS_NAME_MAX;
}

int vfs_mount(const char* path, filesystem_t* fs)
{
    if (!mount_path_valid(path) ||
        !fs ||
        mount_count >= VFS_MAX_MOUNTS)
    {
        return -1;
    }

    /*
     * A filesystem must be registered before it can be mounted.
     * This prevents an arbitrary filesystem object from bypassing
     * the VFS registry.
     */
    bool registered = false;

    for (size_t i = 0; i < filesystem_count; ++i)
    {
        if (registered_filesystems[i] == fs)
        {
            registered = true;
            break;
        }
    }

    if (!registered)
        return -1;

    for (size_t i = 0; i < mount_count; ++i)
    {
        if (strcmp(mounts[i].path, path) == 0)
            return -1;
    }

    if (fs->mount && fs->mount() != 0)
        return -1;

    strncpy(mounts[mount_count].path, path, VFS_NAME_MAX - 1);
    mounts[mount_count].path[VFS_NAME_MAX - 1] = '\0';
    mounts[mount_count].fs = fs;
    mount_count++;
    return 0;
}

int vfs_unmount(const char* path)
{
    if (!path)
        return -1;

    for (size_t i = 0; i < mount_count; ++i)
    {
        if (strcmp(mounts[i].path, path) != 0)
            continue;

        filesystem_t* fs = mounts[i].fs;
        if (fs && fs->unmount && fs->unmount() != 0)
            return -1;

        mounts[i] = mounts[mount_count - 1];
        memset(&mounts[mount_count - 1], 0, sizeof(mounts[0]));
        mount_count--;
        return 0;
    }
    return -1;
}

static int path_matches_mount(const char* path, const char* mount)
{
    if (strcmp(mount, "/") == 0)
        return path[0] == '/';

    size_t len = strlen(mount);
    return strncmp(path, mount, len) == 0 &&
           (path[len] == '\0' || path[len] == '/');
}

vnode_t* vfs_lookup(const char* path)
{
    if (!mount_path_valid(path))
        return NULL;

    size_t best = (size_t)-1;
    size_t best_len = 0;

    for (size_t i = 0; i < mount_count; ++i)
    {
        if (path_matches_mount(path, mounts[i].path))
        {
            size_t len = strlen(mounts[i].path);
            if (len >= best_len)
            {
                best = i;
                best_len = len;
            }
        }
    }

    if (best == (size_t)-1 || !mounts[best].fs)
        return NULL;

    filesystem_t* fs = mounts[best].fs;
    if (!fs->lookup)
        return (best_len == strlen(path)) ? fs->root : NULL;

    if (strcmp(mounts[best].path, "/") == 0)
        return fs->lookup(path);

    const char* relative = path + best_len;
    if (*relative == '\0')
        relative = "/";
    else if (*relative != '/')
        return NULL;

    return fs->lookup(relative);
}
