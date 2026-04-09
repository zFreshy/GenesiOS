#include "vfs.h"
#include "../include/kprintf.h"

static struct vfs_node g_root;

static void split_path(const char *path, char *dir, char *base) {
    int last_slash = -1;
    int len = kstrlen(path);
    for(int i = len - 1; i >= 0; i--) {
        if (path[i] == '/') {
            last_slash = i;
            break;
        }
    }
    if (last_slash == -1) {
        dir[0] = '/'; dir[1] = '\0';
        kmemcpy(base, path, len + 1);
    } else if (last_slash == 0) {
        dir[0] = '/'; dir[1] = '\0';
        kmemcpy(base, path + 1, len - 1 + 1);
    } else {
        kmemcpy(dir, path, last_slash);
        dir[last_slash] = '\0';
        kmemcpy(base, path + last_slash + 1, len - last_slash);
    }
}

void vfs_init(void) {
    kmemset(&g_root, 0, sizeof(struct vfs_node));
    g_root.name[0] = '/';
    g_root.type = VFS_DIRECTORY;
    g_root.parent = NULL;
    kprintf("  [VFS] Virtual File System initialized.\n");
    
    // Create standard directories
    vfs_create_dir("/bin");
    vfs_create_dir("/lib");
    vfs_create_dir("/home");
    vfs_create_dir("/home/documents");
    vfs_create_dir("/home/pictures");
    vfs_create_dir("/etc");
    vfs_create_dir("/dev");
    
    vfs_create_file("/lib/libc.so", 1024, NULL);
    vfs_create_file("/lib/libgui.so", 2048, NULL);
    vfs_create_file("/etc/resolv.conf", 12, (uint8_t*)"nameserver\n");
    vfs_create_file("/home/documents/notes.txt", 50, NULL);
    vfs_create_file("/home/documents/todo.txt", 100, NULL);
    vfs_create_file("/home/pictures/wallpaper.bmp", 10240, NULL);
    vfs_create_file("/home/pictures/vacation.mp4", 5000000, NULL);
}

struct vfs_node* vfs_get_root(void) {
    return &g_root;
}

struct vfs_node* vfs_find(const char *path) {
    if (path[0] == '/' && path[1] == '\0') return &g_root;
    
    struct vfs_node *current = &g_root;
    char buffer[VFS_NAME_MAX];
    int len = kstrlen(path);
    int p = 1; // skip leading slash
    
    while(p < len) {
        int next_slash = -1;
        for(int i=p; i<len; i++) {
            if (path[i] == '/') { next_slash = i; break; }
        }
        
        int chunk_len = (next_slash == -1) ? (len - p) : (next_slash - p);
        kmemcpy(buffer, path + p, chunk_len);
        buffer[chunk_len] = '\0';
        
        bool found = false;
        for(int i=0; i<current->num_children; i++) {
            if (kstrcmp(current->children[i]->name, buffer) == 0) {
                current = current->children[i];
                found = true;
                break;
            }
        }
        if (!found) return NULL;
        
        if (next_slash == -1) break;
        p = next_slash + 1;
    }
    
    return current;
}

struct vfs_node* vfs_create_dir(const char *path) {
    char dir[256];
    char base[VFS_NAME_MAX];
    split_path(path, dir, base);
    
    struct vfs_node *parent = vfs_find(dir);
    if (!parent || parent->type != VFS_DIRECTORY) return NULL;
    if (parent->num_children >= VFS_MAX_CHILDREN) return NULL;
    
    extern void* kmalloc(size_t size);
    struct vfs_node *node = (struct vfs_node*)kmalloc(sizeof(struct vfs_node));
    if (!node) return NULL;
    
    kmemset(node, 0, sizeof(struct vfs_node));
    int blen = kstrlen(base);
    kmemcpy(node->name, base, blen < VFS_NAME_MAX ? blen + 1 : VFS_NAME_MAX);
    node->type = VFS_DIRECTORY;
    node->parent = parent;
    
    parent->children[parent->num_children++] = node;
    return node;
}

struct vfs_node* vfs_create_file(const char *path, uint32_t size, uint8_t *data) {
    char dir[256];
    char base[VFS_NAME_MAX];
    split_path(path, dir, base);
    
    struct vfs_node *parent = vfs_find(dir);
    if (!parent || parent->type != VFS_DIRECTORY) return NULL;
    if (parent->num_children >= VFS_MAX_CHILDREN) return NULL;
    
    extern void* kmalloc(size_t size);
    struct vfs_node *node = (struct vfs_node*)kmalloc(sizeof(struct vfs_node));
    if (!node) return NULL;
    
    kmemset(node, 0, sizeof(struct vfs_node));
    int blen = kstrlen(base);
    kmemcpy(node->name, base, blen < VFS_NAME_MAX ? blen + 1 : VFS_NAME_MAX);
    node->type = VFS_FILE;
    node->parent = parent;
    node->size = size;
    
    if (size > 0 && data) {
        node->data = (uint8_t*)kmalloc(size);
        kmemcpy(node->data, data, size);
    }
    
    parent->children[parent->num_children++] = node;
    return node;
}