#define SIZE 64

#include "inode.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

// Prints specs of inode
// Args:
// - node: Pointer to inode
void print_inode(inode_t *node) {
  printf("inode: refs: %d, mode: %d, size: %d, block: %d\n", node->refs,
         node->mode, node->size, node->block);
}

// Get inode from inum
// Args:
// - inum: (inode)
// Returns pointer to inode, else NULL
inode_t *get_inode(int inum) {
  assert(inum < SIZE);  // inum within range
  void *ibm = get_inode_bitmap();

  if (bitmap_get(ibm, inum) == 0) {
    return NULL;  // Retunr NULL if inode not allocated
  }

  printf("getting inode %d\n", inum);
  inode_t *inode_ptr = (inode_t *)blocks_get_block(2);

  return inode_ptr + inum;  // Return inode pointers
}

// Allocates new inode & returns inum
// Returns inum of allocated inode, else -1
int alloc_inode() {
  void *ibm = get_inode_bitmap();
  printf("trying to allocate\n");

  // Loop through inode bitmap (find unallocated inode)
  for (int i = ROOT_INODE + 1; i < SIZE; i++) {
    if (bitmap_get(ibm, i) == 0) {  // Check inode = free
      bitmap_put(ibm, i, 1);  // Mark inode = used
      inode_t *node = get_inode(i);

      // Init inode
      memset(node, 0, sizeof(inode_t));
      node->refs = 0;
      node->mode = 010644;
      node->size = 0;
      node->block = alloc_block();
      node->next_inode = -1;
      node->atime = time(NULL);
      node->mtime = time(NULL);

      printf("allocating inode at %d\n", i);
      return i;  // Return allocated inum
    }
  }
  return -1;  // No free inode found
}

// Frees inode & marks as unallocated in bitmap.
// Args:
// - inum: Inode number of the inode to free.
void free_inode(int inum) {
  void *ibm = get_inode_bitmap();
  inode_t *node = get_inode(inum);

  assert(node->refs == 0);
  printf("freeing inode at %d\n", inum);

  // Shrink inode to 0
  shrink_inode(node, node->size);
  free_block(node->block);  // Free block

  memset(node, 0, sizeof(inode_t));  // Clear inode data
  bitmap_put(ibm, inum, 0);  // Mark inode = free in bitmap
}

// Grow inode size by size
// Args:
// - node: Pointer to inode
// - size: Size to increase
// Returns 0 if successful, else negative
int grow_inode(inode_t *node, int size) {
  int target_size = node->size + size;  // Calculate new target size

  printf("growing inode from %d to %d\n", node->size, target_size);

  node->size = target_size;  // Update inode size
  inode_t *pnode = node;  // Start with current inode

  while (pnode->size > BLOCK_SIZE) {
    // Check next inode expand?
    if (pnode->next_inode == -1) {
      pnode->next_inode = alloc_inode();  // Allocate new inode/link
    }

    // Update size of next node
    int sz = pnode->size;
    pnode = get_inode(pnode->next_inode);
    pnode->size = sz - BLOCK_SIZE;
  }
  return 0;
}

// Shrinks inode size by specified size
// Args:
// - node: Pointer to inode
// - size: Size to decrease inode
// Returns 0 if successful, else negative
int shrink_inode(inode_t *node, int size) {
  void *ibm = get_inode_bitmap();  // Disconnect chain from inode (deallocated)

  inode_t *pnode = node;  // Start = Current inode
  // Find inode that fits
  while (size < pnode->size - BLOCK_SIZE) {
    pnode = get_inode(pnode->next_inode);
  }

  // Determine inum to start deallocation
  int inum_to_del = pnode->next_inode;
  pnode->next_inode = -1;  // Disconnect

  inode_t *to_del = get_inode(inum_to_del);
  // deallocate inode/blocks (form inum_to_del)
  while (inum_to_del > 0) {
    bitmap_put(ibm, inum_to_del, 0);  // Mark inode = free
    free_block(to_del->block);  // Free block (inode)
    inum_to_del = to_del->next_inode;  // Move to next inode
    memset(to_del, 0, sizeof(inode_t));  // Clear inode data
  }

  // Calculate target size (after shrink)
  int target_size = node->size - size;
  printf("shrinking inode from %d to %d\n", node->size, target_size);

  // Update size of inodes
  pnode = node;
  pnode->size = target_size;

  // Update size of other inodes
  while (pnode->next_inode != -1) {
    pnode = get_inode(pnode->next_inode);
    pnode->size -= size;
  }
  return 0;
}
