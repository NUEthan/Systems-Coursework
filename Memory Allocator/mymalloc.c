#define _DEFAULT_SOURCE
#define _BSD_SOURCE
#include <malloc.h>
#include <debug.h>
#include <assert.h>
#include <unistd.h>  // for sbrk function
#include <string.h>  // for memset function


#define BLOCK_SIZE sizeof(block_t)  // Size of block metadata


// A block of memory managed in linked list
typedef struct block {
  size_t size;        // Size of memory block
  struct block *next; // Pointer to next block in linked list
  int is_free;        // Flag indicating if block is free(1) or not(0)
} block_t;

// Functions for navigation
block_t* request_new_space(block_t* previous_block, size_t requested_size);
block_t* search_free_block(block_t** last_block, size_t requested_size);

// Head fo linked list/ starting point of memory blocks
block_t* head = NULL;

/**
 * Allocates memory indistinguishable to malloc from <stdlib.h>
 *
 * @param s # of bytes of memory to allocate
 * @return Pointer to allocated memory or NULL if allocation failed
 *
 * Asserts that requested size > 0
 * Finds a free block or request new space if needed
 */
void *mymalloc(size_t s) {
  assert(s > 0);  // Ensure rerquested size > 0

  block_t* block;

  if (!head) {
    // When no memory is alloated, request new space
    block = request_space(NULL, s);
    if (!block) {
      return NULL;  // Allocate failed
      }
      head = block;
  } else {
        // Else, find free block or request space again
	block_t* last = head;
	block = find_if_free(&last, s);
	if (!block) {
	  // No free block then rerquest new space
          block = request_space(last, s);
	  if (!block) {
	    return NULL;  // Allocation failed
	  }
	}
  }
	debug_printf("malloc %zu bytes\n", block->size);
	return block + 1; // Return pointer to usable memory, skip metadata
}

/**
 * Finds free block of >= requested size via first-fit approach
 *
 * @param previous_block Pointer to last visited block
 * @param requested_size Size of block to find
 * @return Pointer to suitable block if found, else NULL
 *
 * Iterates through linked list from head till a free block that is large enough
 * is found
 */
block_t* search_free_block(block_t** previous_block, size_t requested_size) {
  block_t* current_block = head;
  while (current_block && (current_block->size < requested_size || !current_block->is_free)) {
    *previous_block = current_block;
    current_block = current_block->next;
  }
  return current_block;
}

/**
 * Requests new space from OS & adds new block to linked list
 *
 * @param previous_block Pointer to last block in linked list
 * @param requested_size Amount of space to request
 * @return Pointer to newly created block, else NULL(if request failed)
 *
 * Increases program's data space
 * If the last block exists, links it to new block
 */
block_t* request_new_space(block_t* previous_block, size_t requested_size) {
  block_t* new_block = sbrk(0);
  void* request = sbrk(BLOCK_SIZE + requested_size);
  if (request == (void*) -1) {
    return NULL;  // sbrk failed to allocate space
  }
  if (previous_block) {
    previous_block->next = new_block;  // Link new block to previous block
  }
  new_block->size = requested_size;
  new_block->next = NULL;
  new_block->is_free = 0;  // Mark not free
  return new_block;
}

/**
 * Allocates zero-initialized space for array of elements
 *
 * @param num_elements # of elements to allocate
 * @param size_per_elements Size of each element in bytes
 * @return Pointer to allocated memory, else NULL(allocation failed)
 *
 * This function computes the total size of memory needed, calls mymalloc to
 * allocate it, and then initializes all bytes to 0
 */
void *mycalloc(size_t num_elements, size_t size_per_element) {
  size_t total_size = num_elements * size_per_element;
  void* allocated_block = mymalloc(total_size);
  if (!allocated_block) {
    return NULL;  // Allocation failed
  }
  memset(allocated_block, 0, total_size);  // Initialized allocated memory to 0
  debug_printf("calloc %zu bytes\n", total_size);
  return allocated_block;
}

/**
 * Frees memory associated to given pointer
 *
 * @param pointer_to_free Pointer to memory to free
 *
 * Marks block of memory as free for future allocations
 * If pointer = NULL, does nothing
 */
void myfree(void *pointer_to_free) {
  if (!pointer_to_free) {
    return;  // No action if pointer = NULL
  }
  block_t* block_to_free = (block_t*)pointer_to_free - 1;  // Access block struct
  block_to_free->is_free = 1;  // Mark block as free
  debug_printf("Freed %zu bytes\n", block_to_free->size);
}
