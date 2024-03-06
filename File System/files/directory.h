#ifndef DIRECTORY_H
#define DIRECTORY_H

// Max length for directory name
#define DIR_NAME_LENGTH 48

#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#include "list.h"
#include "inode.h"
#include "slist.h"
#include "blocks.h"

// dirent struct (directory entry)
typedef struct dirent {
  char name[DIR_NAME_LENGTH];  // Name of file/directory
  int inum;  		       // Inum (entry)
  char _reserved[12];          // Reserved space
  int filled;                  // Flag (if entry used)
} dirent_t;

// dirent_node struct for linked list (directory entries)
typedef struct dirent_node {
  dirent_t entry;              // Directory entry
  list_entry_t dirent_list;    // List entry (link nodes)
} dirent_node_t;

// Init root node directory
void directory_init();

// Find inum of file/directory within given inode dir
int directory_lookup(inode_t *dd, const char *name);

// Lookup inum for given path
int tree_lookup(const char *path);

// Add file/directory to directory inode
int directory_put(inode_t *dd, const char *name, int inum);

// Delete fike/directory to directory inode
int directory_delete(inode_t *dd, const char *name);

// Create list of directory entry for path
dirent_node_t *directory_list(const char *path, int inum);

// Print content of direcotry inode (console)
void print_directory(inode_t *dd);

#endif // DIRECTORY_H
