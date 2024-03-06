/**
 * Vector implementation.
 *
 * - Implement each of the functions to create a working growable array (vector).
 * - Do not change any of the structs
 * - When submitting, You should not have any 'printf' statements in your vector 
 *   functions.
 *
 * IMPORTANT: The initial capacity and the vector's growth factor should be 
 * expressed in terms of the configuration constants in vect.h
 */
#define _GNU_SOURCE
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "vect.h"

/** Main data structure for the vector. */
struct vect {
  char **data;             /* Array containing the actual data. */
  unsigned int size;       /* Number of items currently in the vector. */
  unsigned int capacity;   /* Maximum number of items the vector can hold before growing. */
};

/** Construct a new empty vector. */
vect_t *vect_new() {

  vect_t *v = malloc(sizeof(vect_t));
  if (!v) return NULL;  //malloc check

  v->data = (char **) malloc(2 * sizeof(char *));
  if (!v->data) {  //malloc check
    free(v);
    return NULL;
  }

  v->size = 0;
  v->capacity = 2;

  return v;
}

/** Delete the vector, freeing all memory it occupies. */
void vect_delete(vect_t *v) {

  if (v == NULL) return; // for null

  for (unsigned int i = 0; i < v->size; i++) {
    free(v->data[i]);
  }

  free(v->data);
  free(v);
}

/** Get the element at the given index. */
const char *vect_get(vect_t *v, unsigned int idx) {

  if (!v || idx >= v->size) return NULL; //for null

  return v->data[idx];
}

/** Get a copy of the element at the given index. The caller is responsible
 *  for freeing the memory occupied by the copy. */
char *vect_get_copy(vect_t *v, unsigned int idx) {
  if (v == NULL || idx >= v->size || v->data[idx] == NULL) return NULL; // for NULL & out of bounds
  char *copy = strdup(v->data[idx]);
  if (!copy) return NULL; // for mem alloc
  return copy;

}
/** Set the element at the given index. */
void vect_set(vect_t *v, unsigned int idx, const char *elt) {
  if (!v || idx >= v->size) return; //for null
  free(v->data[idx]);

  if (elt) {
    char *copy = strdup(elt);
    if (!copy) return; // for alloc error
    v->data[idx] = copy;
  } else {
    v->data[idx] = NULL;
  }
}

/** Add an element to the back of the vector. */
void vect_add(vect_t *v, const char *elt) {
  if (!v) return;  //for null

  if (v->size == v->capacity) {
    v->capacity *= 2;
    char **newData = realloc(v->data, v->capacity * sizeof(char *));
    if (!newData) return; // for realloc error

    v->data = newData;
  }

  if (elt) {
    char *copy = strdup(elt);
    if (!copy) return; // for alloc fail
    v->data[v->size] = copy;
  } else {
    v->data[v->size] = NULL;
  }
  
  v->size++;

}

/** Remove the last element from the vector. */
void vect_remove_last(vect_t *v) {
  if (!v || v->size == 0) return; // for null and size

  free(v->data[v->size - 1]);
  v->data[v->size - 1] = NULL; //check null post free
  v->size--;
}

/** The number of items currently in the vector. */
unsigned int vect_size(vect_t *v) {
  if (!v) return 0; // for null

  /* [TODO] Complete the function */
  return v->size;
}

/** The maximum number of items the vector can hold before it has to grow. */
unsigned int vect_current_capacity(vect_t *v) {
  if (!v) return 0; // for null

  /* [TODO] Complete the function */
  return v->capacity;
}

