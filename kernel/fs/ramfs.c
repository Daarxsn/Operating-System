#include "ramfs.h"

#include <string.h>
#include "../memory/heap.h"

static ramfs_t ramfs;

static vnode_t* ramfs_vfs_lookup(const char* path)
{
    ramfs_node_t* node = ramfs_lookup(path);
    return node ? &node->vnode : NULL;
}

filesystem_t ramfs_filesystem = {
    .name = "ramfs",
    .mount = ramfs_mount,
    .unmount = ramfs_unmount,
    .lookup = ramfs_vfs_lookup,
    .root = NULL
};

static void ramfs_destroy_tree(ramfs_node_t* node)
{
    if (!node)
        return;

    vnode_t* child = node->vnode.children;
    while (child)
    {
        vnode_t* next = child->next;
        ramfs_destroy_tree((ramfs_node_t*)child);
        child = next;
    }

    if (node->data)
        kfree(node->data);

    kfree(node);
}

int ramfs_mount(void)
{
    if (ramfs.root)
        return 0;

    ramfs.root = ramfs_create_node("/", VFS_NODE_DIRECTORY);
    if (!ramfs.root)
        return -1;

    ramfs_filesystem.root = &ramfs.root->vnode;
    return 0;
}

int ramfs_unmount(void)
{
    if (ramfs.root)
        ramfs_destroy_tree(ramfs.root);

    ramfs.root = NULL;
    ramfs_filesystem.root = NULL;
    return 0;
}

ramfs_node_t* ramfs_create_node(const char* name, vfs_node_type_t type)
{
    if (!name || !*name)
        return NULL;

    ramfs_node_t* node = kmalloc(sizeof(*node));
    if (!node)
        return NULL;

    memset(node, 0, sizeof(*node));
    strncpy(node->vnode.name, name, VFS_NAME_MAX - 1);
    node->vnode.name[VFS_NAME_MAX - 1] = '\0';
    node->vnode.type = type;
    node->vnode.private_data = node;
    return node;
}

int ramfs_add_child(ramfs_node_t* parent, ramfs_node_t* child)
{
    if (!parent || !child || parent->vnode.type != VFS_NODE_DIRECTORY)
        return -1;

    if (ramfs_find_child(parent, child->vnode.name))
        return -1;

    child->vnode.parent = &parent->vnode;
    child->vnode.next = NULL;

    if (!parent->vnode.children)
    {
        parent->vnode.children = &child->vnode;
        return 0;
    }

    vnode_t* current = parent->vnode.children;
    while (current->next)
        current = current->next;
    current->next = &child->vnode;
    return 0;
}

ramfs_node_t* ramfs_find_child(ramfs_node_t* parent, const char* name)
{
    if (!parent || !name)
        return NULL;

    vnode_t* current = parent->vnode.children;
    while (current)
    {
        if (strcmp(current->name, name) == 0)
            return (ramfs_node_t*)current;
        current = current->next;
    }
    return NULL;
}

static ramfs_node_t* ramfs_lookup_internal(const char* path)
{
    if (!path || !ramfs.root)
        return NULL;

    if (*path == '\0' || strcmp(path, "/") == 0)
        return ramfs.root;

    ramfs_node_t* current = ramfs.root;
    const char* p = path;

    while (*p == '/')
        p++;

    while (*p)
    {
        char component[RAMFS_NAME_MAX];
        size_t len = 0;

        while (*p && *p != '/')
        {
            if (len + 1 >= sizeof(component))
                return NULL;
            component[len++] = *p++;
        }
        component[len] = '\0';

        if (len > 0 && strcmp(component, ".") != 0)
        {
            if (strcmp(component, "..") == 0)
            {
                if (current->vnode.parent)
                    current = (ramfs_node_t*)current->vnode.parent;
            }
            else
            {
                current = ramfs_find_child(current, component);
                if (!current)
                    return NULL;
            }
        }

        while (*p == '/')
            p++;
    }

    return current;
}

ramfs_node_t* ramfs_lookup(const char* path)
{
    return ramfs_lookup_internal(path);
}

static int split_parent(const char* path, char* parent, size_t parent_size,
                        char* name, size_t name_size)
{
    if (!path || !parent || !name || parent_size == 0 || name_size == 0)
        return -1;

    size_t len = strlen(path);
    while (len > 1 && path[len - 1] == '/')
        len--;

    if (len <= 1 || len >= 256)
        return -1;

    char temp[256];
    memcpy(temp, path, len);
    temp[len] = '\0';

    char* slash = NULL;
    for (char* p = temp; *p; ++p)
        if (*p == '/') slash = p;

    if (!slash || slash == temp)
    {
        strncpy(parent, "/", parent_size - 1);
        parent[parent_size - 1] = '\0';
        strncpy(name, slash ? slash + 1 : temp, name_size - 1);
    }
    else
    {
        *slash = '\0';
        strncpy(parent, temp, parent_size - 1);
        parent[parent_size - 1] = '\0';
        strncpy(name, slash + 1, name_size - 1);
    }

    name[name_size - 1] = '\0';
    return name[0] ? 0 : -1;
}

static ramfs_node_t* ramfs_create_at(const char* path, vfs_node_type_t type)
{
    char parent_path[256];
    char name[RAMFS_NAME_MAX];

    if (split_parent(path, parent_path, sizeof(parent_path),
                     name, sizeof(name)) != 0)
        return NULL;

    ramfs_node_t* parent = ramfs_lookup(parent_path);
    if (!parent || parent->vnode.type != VFS_NODE_DIRECTORY)
        return NULL;

    if (ramfs_find_child(parent, name))
        return NULL;

    ramfs_node_t* node = ramfs_create_node(name, type);
    if (!node)
        return NULL;

    if (ramfs_add_child(parent, node) != 0)
    {
        kfree(node);
        return NULL;
    }

    return node;
}

ramfs_node_t* ramfs_create_directory(const char* path)
{
    if (!ramfs.root && ramfs_mount() != 0)
        return NULL;
    return ramfs_create_at(path, VFS_NODE_DIRECTORY);
}

ramfs_node_t* ramfs_create_file(const char* path)
{
    if (!ramfs.root && ramfs_mount() != 0)
        return NULL;
    return ramfs_create_at(path, VFS_NODE_FILE);
}
