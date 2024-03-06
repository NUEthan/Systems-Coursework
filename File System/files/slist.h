#ifndef SLIST_H
#define SLIST_H

// Singly linked list (storing strings)
typedef struct slist {
  char *data;            // Pointer to string data
  int refs;              // Reference count for string
  struct slist *next;    // Pointer to next node of list
} slist_t;

// Create new node in singly linked list containing given string
slist_t *s_cons(const char *text, slist_t *rest);

// Free memory allocated for singly linked list
void s_free(slist_t *xs);

// Split string into list of strings (based on delimiter)
slist_t *s_explode(const char *text, char delim);

#endif
