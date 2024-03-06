// Inode manipulation routines.
//
// Feel free to use as inspiration.

// based on cs3650 starter code
#ifndef INODE_H
#define INODE_H

#include "blocks.h"

#include <time.h>
#include "bitmap.h"

#define ROOT_INODE 1

typedef struct inode {
  int refs;   // reference count
  int mode;   // permission & type
  int size;   // bytes
  int block;  // single block pointer (if max file size <= 4K)
  int next_inode;  // Pointer to next inode (indirect block management)
  time_t atime;  // Last time accessed
  time_t mtime;  // Last time modified
} inode_t;

void print_inode(inode_t *node);
inode_t *get_inode(int inum);
int alloc_inode();
void free_inode(int inum);
int grow_inode(inode_t *node, int size);
int shrink_inode(inode_t *node, int size);
// indode_get_bnum not neeeded

#endif
