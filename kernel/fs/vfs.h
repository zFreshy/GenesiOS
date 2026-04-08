/*
 * kernel/fs/vfs.h
 * Virtual File System interface (Fase 5).
 */
#ifndef VFS_H
#define VFS_H

#include "../include/kernel.h"

typedef struct vnode vnode_t;
typedef struct vfs   vfs_t;

typedef struct vfs_ops {
    int (*open) (vnode_t *vn, int flags);
    int (*close)(vnode_t *vn);
    int64_t (*read) (vnode_t *vn, void *buf, size_t len, size_t off);
    int64_t (*write)(vnode_t *vn, const void *buf, size_t len, size_t off);
} vfs_ops_t;

struct vnode {
    uint64_t   inode;
    uint64_t   size;
    vfs_ops_t *ops;
    vfs_t     *fs;
    void      *priv;
};

struct vfs {
    char       name[32];
    vnode_t   *root;
    vfs_t     *next;
};

/* TODO: Phase 5 implementation */
void vfs_init(void);
int  vfs_mount(const char *path, vfs_t *fs);
vnode_t *vfs_open(const char *path, int flags);

#endif /* VFS_H */
