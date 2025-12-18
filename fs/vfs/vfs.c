/* ============================================
 * fs/vfs/vfs.c - VFS Implementation (Basic)
 * ============================================ */
#include "vfs.h"
#include "../../lib/string/string.h"

// Use __attribute__((unused)) to suppress "defined but not used" warning
static vfs_node_t *root_node __attribute__((unused)) = NULL;

void vfs_init(void) {
    // Will be initialized by specific filesystem
}

int vfs_mount(const char *path, const char *type, uint32_t flags) {
    (void)path;
    (void)type;
    (void)flags;
    // Simplified mount - actual implementation in ramfs/devfs
    return 0;
}

vfs_node_t *vfs_open(const char *path) {
    (void)path;
    return NULL; // Simplified
}

int vfs_close(vfs_node_t *node) {
    (void)node;
    return 0;
}

int vfs_read(vfs_node_t *node, void *buffer, size_t size) {
    if (node && node->ops && node->ops->read) {
        return node->ops->read(node, buffer, size, 0);
    }
    return -1;
}

int vfs_write(vfs_node_t *node, const void *buffer, size_t size) {
    if (node && node->ops && node->ops->write) {
        return node->ops->write(node, buffer, size, 0);
    }
    return -1;
}

int vfs_mkdir(const char *path, uint32_t permissions) {
    (void)path;
    (void)permissions;
    return 0; // Simplified
}

int vfs_unlink(const char *path) {
    (void)path;
    return 0; // Simplified
}

vfs_node_t *vfs_readdir(vfs_node_t *dir, uint32_t index) {
    if (dir && dir->ops && dir->ops->readdir) {
        return dir->ops->readdir(dir, index);
    }
    return NULL;
}
