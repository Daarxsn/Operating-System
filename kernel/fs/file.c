#include "file.h"
#include "ramfs.h"
#include <string.h>
#include <stdint.h>
#include "../memory/heap.h"

static file_t file_table[MAX_OPEN_FILES];

void file_init(void)
{
    memset(file_table, 0, sizeof(file_table));
}

int open(const char* path)
{
    vnode_t* node = vfs_lookup(path);
    if (!node || node->type != VFS_NODE_FILE)
        return -1;

    for (int i = 0; i < MAX_OPEN_FILES; ++i)
    {
        if (!file_table[i].used)
        {
            file_table[i].used = 1;
            file_table[i].fd = i;
            file_table[i].node = node;
            file_table[i].offset = 0;
            file_table[i].flags = 0;
            return i;
        }
    }
    return -1;
}

int close(int fd)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || !file_table[fd].used)
        return -1;

    memset(&file_table[fd], 0, sizeof(file_table[fd]));
    return 0;
}

static ramfs_node_t* file_ramfs_node(int fd)
{
    if (fd < 0 || fd >= MAX_OPEN_FILES || !file_table[fd].used)
        return NULL;

    vnode_t* vnode = file_table[fd].node;
    if (!vnode || vnode->type != VFS_NODE_FILE)
        return NULL;

    return (ramfs_node_t*)vnode;
}

int read(int fd, void* buffer, size_t size)
{
    ramfs_node_t* node = file_ramfs_node(fd);

    /*
     * A zero-length read is a successful no-op and does not
     * require a valid buffer.
     */
    if (size == 0)
        return 0;

    if (!node || !buffer)
        return -1;

    if (file_table[fd].offset >= node->vnode.size)
        return 0;

    size_t remaining =
        node->vnode.size - file_table[fd].offset;

    size_t bytes =
        size < remaining ? size : remaining;

    /*
     * The public API returns int. Do not silently truncate a
     * successful size_t result into a negative/incorrect int.
     */
    if (bytes > (size_t)INT32_MAX)
        return -1;

    memcpy(
        buffer,
        node->data + file_table[fd].offset,
        bytes
    );

    file_table[fd].offset += bytes;

    return (int)bytes;
}

int write(int fd, const void* buffer, size_t size)
{
    ramfs_node_t* node = file_ramfs_node(fd);

    if (!node || (!buffer && size != 0))
        return -1;

    /*
     * The API returns int, so reject sizes that cannot be
     * represented without truncation.
     */
    if (size > (size_t)INT32_MAX)
        return -1;

    size_t offset = file_table[fd].offset;

    if (offset > (size_t)-1 - size)
        return -1;

    size_t required = offset + size;
    uint8_t* new_data = node->data;

    if (required > node->vnode.size)
    {
        new_data = kmalloc(required);
        if (!new_data)
            return -1;

        memset(new_data, 0, required);
        if (node->data && node->vnode.size)
            memcpy(new_data, node->data, node->vnode.size);
    }

    if (size)
        memcpy(new_data + offset, buffer, size);

    if (new_data != node->data)
    {
        if (node->data)
            kfree(node->data);
        node->data = new_data;
    }

    if (required > node->vnode.size)
        node->vnode.size = required;

    file_table[fd].offset = required;
    return (int)size;
}
