/* ============================================
 * fs/simfs/simfs.c - Simple In-Memory Filesystem
 * Separate filesystem module for LexOS
 * ============================================ */
#include "simfs.h"
#include "../../lib/string/string.h"

static simfs_entry_t entries[SIMFS_MAX_FILES + SIMFS_MAX_DIRS];
static int entry_count = 0;
static char current_path[SIMFS_MAX_PATH] = "/";

void simfs_init(void) {
    entry_count = 0;
    strcpy(current_path, "/");
    memset(entries, 0, sizeof(entries));
}

const char* simfs_get_cwd(void) {
    return current_path;
}

static int path_equals(const char *p1, const char *p2) {
    return strcmp(p1, p2) == 0;
}

char* simfs_resolve_path(const char *name, char *full_path) {
    if (name[0] == '/') {
        strcpy(full_path, name);
    } else {
        if (path_equals(current_path, "/")) {
            strcpy(full_path, "/");
            strcat(full_path, name);
        } else {
            strcpy(full_path, current_path);
            strcat(full_path, "/");
            strcat(full_path, name);
        }
    }
    return full_path;
}

static simfs_entry_t* find_entry(const char *full_path, simfs_type_t type) {
    for (int i = 0; i < entry_count; i++) {
        if (!entries[i].in_use) continue;
        
        char entry_full_path[SIMFS_MAX_PATH];
        if (path_equals(entries[i].parent_path, "/")) {
            strcpy(entry_full_path, "/");
            strcat(entry_full_path, entries[i].name);
        } else {
            strcpy(entry_full_path, entries[i].parent_path);
            strcat(entry_full_path, "/");
            strcat(entry_full_path, entries[i].name);
        }
        
        if (path_equals(entry_full_path, full_path) && entries[i].type == type) {
            return &entries[i];
        }
    }
    return NULL;
}

static simfs_entry_t* find_entry_any(const char *full_path) {
    for (int i = 0; i < entry_count; i++) {
        if (!entries[i].in_use) continue;
        
        char entry_full_path[SIMFS_MAX_PATH];
        if (path_equals(entries[i].parent_path, "/")) {
            strcpy(entry_full_path, "/");
            strcat(entry_full_path, entries[i].name);
        } else {
            strcpy(entry_full_path, entries[i].parent_path);
            strcat(entry_full_path, "/");
            strcat(entry_full_path, entries[i].name);
        }
        
        if (path_equals(entry_full_path, full_path)) {
            return &entries[i];
        }
    }
    return NULL;
}

int simfs_set_cwd(const char *path) {
    if (path_equals(path, "/") || path_equals(path, "")) {
        strcpy(current_path, "/");
        return 0;
    }
    
    if (path_equals(path, "..")) {
        if (path_equals(current_path, "/")) {
            return 0;
        }
        int len = strlen(current_path);
        for (int i = len - 1; i >= 0; i--) {
            if (current_path[i] == '/') {
                if (i == 0) {
                    strcpy(current_path, "/");
                } else {
                    current_path[i] = '\0';
                }
                return 0;
            }
        }
        strcpy(current_path, "/");
        return 0;
    }
    
    char full_path[SIMFS_MAX_PATH];
    simfs_resolve_path(path, full_path);
    
    simfs_entry_t *entry = find_entry(full_path, SIMFS_TYPE_DIR);
    if (entry) {
        strcpy(current_path, full_path);
        return 0;
    }
    
    return -1;
}

int simfs_mkdir(const char *name) {
    if (entry_count >= SIMFS_MAX_FILES + SIMFS_MAX_DIRS) {
        return -1;
    }
    
    char full_path[SIMFS_MAX_PATH];
    simfs_resolve_path(name, full_path);
    
    if (find_entry_any(full_path)) {
        return -2;
    }
    
    simfs_entry_t *new_entry = NULL;
    for (int i = 0; i < SIMFS_MAX_FILES + SIMFS_MAX_DIRS; i++) {
        if (!entries[i].in_use) {
            new_entry = &entries[i];
            break;
        }
    }
    
    if (!new_entry) return -1;
    
    new_entry->in_use = true;
    new_entry->type = SIMFS_TYPE_DIR;
    strcpy(new_entry->name, name);
    strcpy(new_entry->parent_path, current_path);
    new_entry->size = 0;
    new_entry->content[0] = '\0';
    entry_count++;
    
    return 0;
}

int simfs_touch(const char *name) {
    if (entry_count >= SIMFS_MAX_FILES + SIMFS_MAX_DIRS) {
        return -1;
    }
    
    char full_path[SIMFS_MAX_PATH];
    simfs_resolve_path(name, full_path);
    
    if (find_entry_any(full_path)) {
        return -2;
    }
    
    simfs_entry_t *new_entry = NULL;
    for (int i = 0; i < SIMFS_MAX_FILES + SIMFS_MAX_DIRS; i++) {
        if (!entries[i].in_use) {
            new_entry = &entries[i];
            break;
        }
    }
    
    if (!new_entry) return -1;
    
    new_entry->in_use = true;
    new_entry->type = SIMFS_TYPE_FILE;
    strcpy(new_entry->name, name);
    strcpy(new_entry->parent_path, current_path);
    new_entry->size = 0;
    new_entry->content[0] = '\0';
    entry_count++;
    
    return 0;
}

int simfs_rm(const char *name) {
    char full_path[SIMFS_MAX_PATH];
    simfs_resolve_path(name, full_path);
    
    simfs_entry_t *entry = find_entry(full_path, SIMFS_TYPE_FILE);
    if (!entry) {
        return -1;
    }
    
    entry->in_use = false;
    entry_count--;
    return 0;
}

int simfs_rmdir(const char *name) {
    char full_path[SIMFS_MAX_PATH];
    simfs_resolve_path(name, full_path);
    
    simfs_entry_t *entry = find_entry(full_path, SIMFS_TYPE_DIR);
    if (!entry) {
        return -1;
    }
    
    for (int i = 0; i < SIMFS_MAX_FILES + SIMFS_MAX_DIRS; i++) {
        if (entries[i].in_use && path_equals(entries[i].parent_path, full_path)) {
            return -2;
        }
    }
    
    entry->in_use = false;
    entry_count--;
    return 0;
}

int simfs_exists(const char *name, simfs_type_t *type) {
    char full_path[SIMFS_MAX_PATH];
    simfs_resolve_path(name, full_path);
    
    simfs_entry_t *entry = find_entry_any(full_path);
    if (entry) {
        if (type) *type = entry->type;
        return 1;
    }
    return 0;
}

int simfs_read_file(const char *name, char *buffer, uint32_t max_size) {
    char full_path[SIMFS_MAX_PATH];
    simfs_resolve_path(name, full_path);
    
    simfs_entry_t *entry = find_entry(full_path, SIMFS_TYPE_FILE);
    if (!entry) {
        return -1;
    }
    
    uint32_t len = strlen(entry->content);
    if (len >= max_size) len = max_size - 1;
    
    memcpy(buffer, entry->content, len);
    buffer[len] = '\0';
    return (int)len;
}

int simfs_write_file(const char *name, const char *content) {
    char full_path[SIMFS_MAX_PATH];
    simfs_resolve_path(name, full_path);
    
    simfs_entry_t *entry = find_entry(full_path, SIMFS_TYPE_FILE);
    if (!entry) {
        if (simfs_touch(name) != 0) {
            return -1;
        }
        entry = find_entry(full_path, SIMFS_TYPE_FILE);
        if (!entry) return -1;
    }
    
    uint32_t len = strlen(content);
    if (len >= SIMFS_MAX_CONTENT) len = SIMFS_MAX_CONTENT - 1;
    
    memcpy(entry->content, content, len);
    entry->content[len] = '\0';
    entry->size = len;
    return 0;
}

int simfs_list_dir(const char *path, char names[][SIMFS_MAX_NAME], simfs_type_t types[], int max_entries) {
    const char *search_path = path ? path : current_path;
    int count = 0;
    
    for (int i = 0; i < SIMFS_MAX_FILES + SIMFS_MAX_DIRS && count < max_entries; i++) {
        if (!entries[i].in_use) continue;
        
        if (path_equals(entries[i].parent_path, search_path)) {
            strcpy(names[count], entries[i].name);
            types[count] = entries[i].type;
            count++;
        }
    }
    
    return count;
}

int simfs_get_dir_count(void) {
    int count = 0;
    for (int i = 0; i < SIMFS_MAX_FILES + SIMFS_MAX_DIRS; i++) {
        if (entries[i].in_use && entries[i].type == SIMFS_TYPE_DIR &&
            path_equals(entries[i].parent_path, current_path)) {
            count++;
        }
    }
    return count;
}

int simfs_get_file_count(void) {
    int count = 0;
    for (int i = 0; i < SIMFS_MAX_FILES + SIMFS_MAX_DIRS; i++) {
        if (entries[i].in_use && entries[i].type == SIMFS_TYPE_FILE &&
            path_equals(entries[i].parent_path, current_path)) {
            count++;
        }
    }
    return count;
}
