#include "storage.h"

//=========================================================== storage_init =//
// initialize storage
// Args:
// - path: Path to file (disk image/storage)
void storage_init(const char *path) {
  blocks_init(path);  // Init block system
  directory_init();  // Init root directory
}

//=========================================================== storage_stat =//
// Get status of file/directory (inum or path)
// Args:
// - path: Path to object. If null use inum
// - inum: (object)
// - st: Pointer to struct stat (fill object info)
// Returns 0 if successful, else -1
int storage_stat(const char *path, int inum, struct stat *st) {
  if(path) inum = tree_lookup(path);  // Lookup inum if path exists

  if (inum < 0) {
    return -1;  // Object not found
  }

  inode_t *node = get_inode(inum);  // Get inode (object)

  memset(st, 0, sizeof(stat));  // Init stat struct
  st->st_ino = inum;
  st->st_uid = getuid();        // Set user ID
  st->st_mode = node->mode;     // Set mode
  st->st_nlink = node->refs;    // Set # links

  if (S_ISREG(node->mode)) {
    st->st_size = node->size;  // Set size for normal files
  }
  return 0;
}

//=========================================================== storage_read =//
// Reads data from object into buffer (starting offset)
// Args:
// - path: Path to object. If null use inum
// - inum: (object)
// - buf: Buffer to store read data
// - size: # bytes to read.
// - offset: Offset from start of object to begin reading
// Returns # bytes read
int storage_read(const char *path, int inum,  char *buf, size_t size, off_t offset) {
  if (path) inum = tree_lookup(path);  // Lookup inum if path provided

  inode_t *node = get_inode(inum);  // Get inode (object)

  assert(offset >= 0);
  assert(size >= 0);

  if (size == 0) {
    return 0;
  } else {  // Handle reading datafrom inode (specified offset & size)
    if (offset + size > node->size) {
      size = node->size - offset;
    }

    inode_t* pnode = node;

    while (offset >= BLOCK_SIZE) {
      pnode = get_inode(pnode->next_inode);
      offset -= BLOCK_SIZE;
    }

    void *block_temp = blocks_get_block(pnode->block);
    block_temp += offset;

    if (size + offset > BLOCK_SIZE) {
      memcpy(buf, block_temp, BLOCK_SIZE - offset);
      int temp_sz = size;
      temp_sz -= BLOCK_SIZE - offset;
      buf += BLOCK_SIZE - offset;

      while (temp_sz > BLOCK_SIZE) {
        pnode = get_inode(pnode->next_inode);
        block_temp = blocks_get_block(pnode->block);
        memcpy(buf, block_temp, BLOCK_SIZE);
        buf += BLOCK_SIZE;
        temp_sz -= BLOCK_SIZE;
      }
      pnode = get_inode(pnode->next_inode);
      block_temp = blocks_get_block(pnode->block);
      memcpy(buf, block_temp, temp_sz);
    } else {
      memcpy(buf, block_temp, size);
    }
    return size;
  }
}

//=========================================================== storage_write =//
// Writes data from buffer to object (starting offset)\
// Args:
// - path: Path to object. If null use inum
// - inum: (object)
// - buf: Buffer containing data to write
// - size: # bytes to write
// - offset: Offset from start of object
// Returns # bytes written
int storage_write(const char *path, int inum, const char *buf, size_t size, off_t offset) {
  if (path) inum = tree_lookup(path);  // Lookup inum if path provided
  assert(inum >= 0);

  inode_t *node = get_inode(inum);  // Get inode (object

  assert(offset >= 0);
  assert(size >= 0);

  if (size == 0) {
    return 0;
  } else {  // Handle writing data to inode (specified offset & size)
    if (size + offset > node->size) {
      if (grow_inode(node, size + offset - node->size) < 0) {
        return -1;
      }
    }

    inode_t* pnode = node;

    while (offset >= BLOCK_SIZE) {
      pnode = get_inode(pnode->next_inode);
      offset -= BLOCK_SIZE;
    }

    void *block_temp = blocks_get_block(pnode->block);
    block_temp += offset;

    if (size + offset > BLOCK_SIZE) {
      memcpy(block_temp, buf, BLOCK_SIZE - offset);
      int temp_sz = size;
      temp_sz -= BLOCK_SIZE - offset;
      buf += BLOCK_SIZE - offset;

      while (temp_sz > 0) {
        pnode = get_inode(pnode->next_inode);
        block_temp = blocks_get_block(pnode->block);
        memcpy(block_temp, buf, temp_sz > BLOCK_SIZE ? BLOCK_SIZE : temp_sz);
        temp_sz -= BLOCK_SIZE;
        buf += BLOCK_SIZE;
      }
    } else {
      memcpy(block_temp, buf, size);
    }
    return size;
  }
}

//=========================================================== storage_truncate =//
// Truncates file to specified size
// Args:
// - path: Path to file
// - size: New file size
// Returns 0 if successful, else error
int storage_truncate(const char *path, off_t size) {
  int inum = tree_lookup(path);  // Lookup inum for path
  assert(inum >= 0);

  inode_t *node = get_inode(inum);  // Get inode of file
  int node_size = node->size;

  // Grow/shrink inode to new size
  if (size >= node->size) {
    int rv = grow_inode(node, size - node_size);
    return rv;
  }
  else {
    int rv = shrink_inode(node, node_size - size);
    return rv;
  }
}

//=========================================================== storage_mknod =//
// Creates file/directory in filesystem.
// Args:
// - path: Parent path (object is created). If null use pinum
// - name: Name of new object
// - pinum: Parent inum
// - mode: Permissions of new object
// Returns 0 if successful, else -EEXIST
int storage_mknod(const char *path, const char *name, int pinum, int mode) {
  if (path) pinum = tree_lookup(path);  // Lookup parent inum

  assert(pinum >= 0);
  inode_t *directory_node = get_inode(pinum);  // Get inode of parent directory

  int inum = directory_lookup(directory_node, name);  // Check if object already exists
  if (inum >= 0) {
    return -EEXIST;
  }

  inum = alloc_inode();  // Allocate new inode for object
  inode_t *node = get_inode(inum);  // Get new inode
  node->refs = 1;
  node->mode = mode;
  node->size = 0;

  directory_put(directory_node, name, inum);  // Add new object to parent directory

  return 0;
}

//=========================================================== storage_unlink =//
// Remove object from filesystem
// Args:
// - path: Path to object. If null use pinum
// - pinum: Parent inum
// - name: Name of object
// Returns 0 if successful, else error
int storage_unlink(const char *path, int pinum, const char *name) {
  if (path) pinum = tree_lookup(path);  // Lookup parent inum
  inode_t *directory_node = get_inode(pinum);  // Get inode of parent directory

  // Unlink child object from directory
  int inum = directory_lookup(directory_node, name);
  inode_t *node = get_inode(inum);

  node->refs--;
  int rv = directory_delete(directory_node, name);

  if (node->refs <= 0) {
    free_inode(inum);
  }
  return rv;
}

//=========================================================== storage_link =//
// Creates link between 2 objects in filesystem
// Args:
// - from: Source object path. If null use from_inum
// - from_inum: Source inum
// - to_parent: Destination parent path. If null use to_pinum
// - to_pinum: Destination parent inum
// - to_child: Destination child object's name
// Returns 0 if successful, else error
int storage_link(const char *from, int from_inum, const char *to_parent, int to_pinum, const char *to_child) {
  if (from) from_inum = tree_lookup(from);  // Lookup source inum
  assert(from_inum >= 0);

  inode_t *node = get_inode(from_inum);  // Get inod eof source object
  node->refs++;

  if (to_parent) to_pinum = tree_lookup(to_parent);  // Lookup parent destination
  inode_t *to_parent_node = get_inode(to_pinum);  // Get inode of parent destination

  int rv = directory_put(to_parent_node, to_child, from_inum);  // Create link in destination directory

  return rv;
}

//=========================================================== storage_rename =//
// Renames/moves object within filesystem
// Args:
// - from_parent: Source parent path. If null use from_pinum
// - from_pinum: Source parent inum
// - from_child: Source child object name
// - to_parent: Destination parent path. If null use to_pinum
// - to_pinum: Destination parent inum
// - to_child: Destination child object name
// Returns 0 if successful, else error
int storage_rename(const char *from_parent, int from_pinum, const char *from_child, const char *to_parent, int to_pinum, const char *to_child) {
  if (from_parent) from_pinum = tree_lookup(from_parent);  // Lookup parent source inum

  inode_t* from_pnode = get_inode(from_pinum);  // Get inode of parent source
  int from_inum = directory_lookup(from_pnode, from_child);

  // Link object to new location
  storage_link(NULL, from_inum, to_parent, to_pinum, to_child);

  // Unlink from old location
  storage_unlink(from_parent, from_pinum, from_child);

  return 0;
}

//=========================================================== storage_list =//
// Lists files/directories in directory
// For retrieval
// Args:
// - path: path of directory to list
// - inum: inum of directory
// Returns a linked list of dirent_node_t containing all the directory entries.
dirent_node_t *storage_list(const char *path, int inum) {
  return directory_list(path, inum);
}

//=========================================================== split_path =//
// Splits given filesystem path into directory part filename / Seprate fullpath
// Args:
// - path: Full path
// - directory: Char array (directory part is stored)
// - name: Char array (name part is stored)
// Note: 'directory' and 'name' should be pre-allocated with sufficient space.
void split_path(const char *path, char *directory, char *name) {
  strcpy(directory, path);  // Copy full path into directoory

  int counter = 0;
  int index = strlen(path) - 1;
  while (path[index] != '/') {  // Oterate from end of path (find last '/')
    counter++;
    index--;
  }

  if (counter != strlen(path) - 1) {  // Split path at last '/'
    directory[strlen(path) - counter - 1] = '\0';  // Terminate directory path
    if (name) strcpy(name, &directory[strlen(path) - counter]);  // Copy name part
  }
  else {  // Handle cases (no parent directory)
    directory[1] = '\0';
    if (name) strcpy(name, &path[1]);
  }
}
