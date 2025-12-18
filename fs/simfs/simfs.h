/* ============================================
 * fs/simfs/simfs.h - Simple In-Memory Filesystem
 * Separate filesystem module for LexOS
 * ============================================ */
#ifndef SIMFS_H
#define SIMFS_H

#include "../../include/types.h"

#define SIMFS_MAX_FILES 64
#define SIMFS_MAX_DIRS 32
#define SIMFS_MAX_NAME 64
#define SIMFS_MAX_PATH 256
#define SIMFS_MAX_CONTENT 2048

typedef enum {
    SIMFS_TYPE_FILE,
    SIMFS_TYPE_DIR
} simfs_type_t;

typedef struct {
    char name[SIMFS_MAX_NAME];
    char parent_path[SIMFS_MAX_PATH];
    simfs_type_t type;
    char content[SIMFS_MAX_CONTENT];
    uint32_t size;
    bool in_use;
} simfs_entry_t;

typedef struct {
    char path[SIMFS_MAX_PATH];
} simfs_context_t;

void simfs_init(void);

const char* simfs_get_cwd(void);
int simfs_set_cwd(const char *path);

int simfs_mkdir(const char *name);
int simfs_touch(const char *name);
int simfs_rm(const char *name);
int simfs_rmdir(const char *name);

int simfs_exists(const char *name, simfs_type_t *type);
int simfs_read_file(const char *name, char *buffer, uint32_t max_size);
int simfs_write_file(const char *name, const char *content);

int simfs_list_dir(const char *path, char names[][SIMFS_MAX_NAME], simfs_type_t types[], int max_entries);

int simfs_get_dir_count(void);
int simfs_get_file_count(void);

char* simfs_resolve_path(const char *name, char *full_path);

#endif
