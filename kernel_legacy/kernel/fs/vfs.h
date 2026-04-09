#ifndef VFS_H
#define VFS_H

#include "../include/kernel.h"

#define VFS_NAME_MAX 128
#define VFS_MAX_CHILDREN 64

enum vfs_node_type {
    VFS_FILE = 1,
    VFS_DIRECTORY,
    VFS_DEVICE,
    VFS_SYMLINK
};

struct vfs_node;

struct vfs_node {
    char name[VFS_NAME_MAX];
    enum vfs_node_type type;
    uint32_t size;
    uint32_t inode;
    
    // File operations
    int (*read)(struct vfs_node *node, uint32_t offset, uint32_t size, uint8_t *buffer);
    int (*write)(struct vfs_node *node, uint32_t offset, uint32_t size, uint8_t *buffer);
    
    // Directory structure (simple in-memory representation)
    struct vfs_node *parent;
    struct vfs_node *children[VFS_MAX_CHILDREN];
    int num_children;
    
    // File content (if simple ramfs)
    uint8_t *data;
};

void vfs_init(void);
struct vfs_node* vfs_get_root(void);
struct vfs_node* vfs_find(const char *path);
struct vfs_node* vfs_create_file(const char *path, uint32_t size, uint8_t *data);
struct vfs_node* vfs_create_dir(const char *path);

#endif