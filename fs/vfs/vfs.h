/* ============================================
 * fs/vfs/vfs.h - Virtual File System Header
 * ============================================ */
#ifndef VFS_H
#define VFS_H

#include "../../include/types.h"

#define MAX_PATH 256
#define MAX_FILENAME 64
#define MAX_MOUNTPOINTS 16

typedef enum {
    FILE_TYPE_REGULAR,
    FILE_TYPE_DIRECTORY,
    FILE_TYPE_DEVICE
} file_type_t;

typedef struct vfs_node vfs_node_t;
typedef struct vfs_operations vfs_operations_t;

struct vfs_node {
    char name[MAX_FILENAME];
    file_type_t type;
    uint32_t inode;
    uint32_t size;
    uint32_t permissions;
    vfs_node_t *parent;
    vfs_operations_t *ops;
    void *fs_data;
};

struct vfs_operations {
    int (*read)(vfs_node_t *node, void *buffer, size_t size, size_t offset);
    int (*write)(vfs_node_t *node, const void *buffer, size_t size, size_t offset);
    vfs_node_t* (*readdir)(vfs_node_t *node, uint32_t index);
    vfs_node_t* (*finddir)(vfs_node_t *node, const char *name);
    int (*mkdir)(vfs_node_t *parent, const char *name, uint32_t permissions);
    int (*unlink)(vfs_node_t *parent, const char *name);
};

void vfs_init(void);
int vfs_mount(const char *path, const char *type, uint32_t flags);
vfs_node_t *vfs_open(const char *path);
int vfs_close(vfs_node_t *node);
int vfs_read(vfs_node_t *node, void *buffer, size_t size);
int vfs_write(vfs_node_t *node, const void *buffer, size_t size);
int vfs_mkdir(const char *path, uint32_t permissions);
int vfs_unlink(const char *path);
vfs_node_t *vfs_readdir(vfs_node_t *dir, uint32_t index);

#endif
