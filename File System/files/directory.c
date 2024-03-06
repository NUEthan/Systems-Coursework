#include "directory.h"
#define TOTAL_DIRENTS BLOCK_SIZE / sizeof(dirent_t)

//============================================================== directory_init =//
// Initializes the root node directory
// Set up root dir with defautl vals
void directory_init() {
  int i = ROOT_INODE;
  void *ibm = get_inode_bitmap();

  if (bitmap_get(ibm, i) == 1) {
    return;  // Root inode already init
  }

  bitmap_put(ibm, i, 1);  // Mark used (root inode)
  inode_t* new_dir_inode = get_inode(i);


  memset(new_dir_inode, 0, sizeof(inode_t));  // Init inode struct
  new_dir_inode->mode = 040755;  // Set dir permissions
  new_dir_inode->refs = 1;  // Init # references
  new_dir_inode->size = 0;  // Init size
  new_dir_inode->block = alloc_block();  // Alloc block for content (directory)
  new_dir_inode->next_inode = -1;  // No inode (next)

  new_dir_inode->atime = time(NULL);  // Set time accessed
  new_dir_inode->mtime = time(NULL);  // Set time modded

  // Add entries to current & parent (directories)
  directory_put(new_dir_inode, ".", i);
  directory_put(new_dir_inode, "..", i);
}

//============================================================== directory_lookup =//
// Lookup file/directory within directory
// Args:
// - dd: Pointer to inode (directory)
// - name: Name of file/directory
// Returns inum of file/directory if found, else -ENOENT
int directory_lookup(inode_t* dd, const char* name) {
  dirent_t* dir_contents = blocks_get_block(dd->block);

  for (int i = 0; i < TOTAL_DIRENTS; i++) {
    if (strcmp(dir_contents[i].name, name) == 0) {
      return dir_contents[i].inum;  // Found requested entry
    }
  }

  return -ENOENT;  // Entry not found
}

//============================================================== tree_lookup =//
// Recursively lookup inum of path
// Args:
// - path: lookup path
// Returns inum (last component in path), else -1 
int tree_lookup(const char* path) {
  slist_t* file_path = s_explode(path, '/');
  slist_t* curr_file = file_path;
  int inode_num = ROOT_INODE;
  inode_t* root_inode = get_inode(inode_num);


  while (curr_file) {
    if (strcmp(curr_file->data, "") != 0) {
      inode_t* node = get_inode(inode_num);
      inode_num = directory_lookup(node, curr_file->data);
      if (inode_num < 0) {
        return -1;  // Componenet not found in path
      }
    }
    curr_file = curr_file->next;
  }

  return inode_num;
}

//============================================================== directory_put =//
// Add entry to directory
// Args:
// - dd: Pointer to inode (directory)
// - name: Name of new entry.
// - inum: Inum of new entry.
// Returns 0 if successful, else -1 (directory full)
int directory_put(inode_t* dd, const char* name, int inum) {
  dirent_t* dir_contents = blocks_get_block(dd->block);

  for (int i = 0; i < TOTAL_DIRENTS; i++) {
    if (dir_contents[i].filled != 1) {
      dir_contents[i].inum = inum;
      strcpy(dir_contents[i].name, name);
      dir_contents[i].filled = 1;
      return 0;  // Added entry
    }
  }
  return -1;  // No space left in directory
}

//============================================================== directory_delete =//
// Delete entry from directory
// Args:
// - dd: Pointer to inode (directory)
// - name: Name of entry
// Returns 0 if successful, else -1 (entry not found)
int directory_delete(inode_t* dd, const char* name) {
  dirent_t* dir_contents = blocks_get_block(dd->block);  // Get contents of directory

  for (int i = 0; i < TOTAL_DIRENTS; i++) {
    // Cmpare current name with target naem
    if (strcmp(dir_contents[i].name, name) == 0) {
      dir_contents[i].filled = 0; // Mark entry unfilled(deleted)
      return 0;  // Deleted entry
    }
  }

  return -1;  // Entry not found
}

//============================================================== directory_list =//
// List entries in directory
// Args:
// - path: Path to directory to list.
// - inum: Inum (directory) (use if path = null)
// Returns linked list of dirent_node_t (contains entries)
dirent_node_t *directory_list(const char* path, int inum) {
  if (path) inum = tree_lookup(path);  // If path, lookup inum

  inode_t* dd = get_inode(inum);  // Get inode of directory
  dirent_t* dir_contents = blocks_get_block(dd->block);  // Get content of directory from inode
  dirent_node_t* dirents = NULL;  // Init head of list

  // Each entry in directory
  for (int i = 0; i < TOTAL_DIRENTS; i++) {
    // Check entry filled
    if (dir_contents[i].filled == 1) {
      dirent_node_t* tmp = malloc(sizeof(dirent_node_t));  // Allocate mem for new 
      tmp->entry = dir_contents[i];  // Copy entry into new

      // IF first entry, init list
      if (!dirents) {
        dirents = tmp;
        list_init(&dirents->dirent_list);
      }
      // add entry to list
      else list_add_after(&dirents->dirent_list, &tmp->dirent_list);
    }
  }

  return dirents;  // Return list of directory entries
}

//============================================================== print_directory =//
// Print contents of directory (console)
// Args:
// - dd: Pointer to inode (directory).
void print_directory(inode_t* dd) {
  // Get contents fo directory from inode
  dirent_t* dir_contents = blocks_get_block(dd->block);

  // Go over each entry of directory
  for (int i = 0; i < TOTAL_DIRENTS; i++) {
    // If entry filled(exists) print name
    if (dir_contents[i].filled == 1) {
      printf("-%s\n", dir_contents[i].name);  // Print entry names
    }
  }
}
