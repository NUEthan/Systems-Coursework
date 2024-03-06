#include "slist.h"
#include <string.h>
#include <alloca.h>
#include <stdlib.h>
//========================================================== s_cons =//
// Creates new node in singly linked list with given string
// Args:
// - text: String to be added
// - rest: Rest of list
// Returns pointer to created list node
slist_t *s_cons(const char *text, slist_t *rest) {
  slist_t *xs = malloc(sizeof(slist_t));     // Allocate memory for new node
  xs->data = strdup(text);                   // Duplicate string
  xs->refs = 1;                              // Init reference count
  xs->next = rest;                           // Set next node in list
  return xs;                                 // Retunr new node
}

//========================================================== s_free =//
// Frees memory allocated for singly linked list
// Traverses list & frees each node
// Args:
// - xs: Head of list to be freed.
void s_free(slist_t *xs) {
  if (xs == 0) {
    return;  // Base case: if empty list, nothing
  }

  xs->refs -= 1;  // -1 ref count

  if (xs->refs == 0) {
    s_free(xs->next);  // Recursively free next node
    free(xs->data);  // Free string data
    free(xs);  // Free current node
  }
}

//========================================================== s_explode =//
// Splits string into LOS based on given delimiter
// For parsing text into parts
// Args:
// - text: String to split
// - delim: Delimiter for splitting string
// Returns linked LOS (substrings of OG string)
slist_t *s_explode(const char *text, char delim) {
  if (*text == 0) {
    return 0;  // Base case, do nothing
  }

  int plen = 0;
  while (text[plen] != 0 && text[plen] != delim) {
    plen += 1;  // Calculate length of substring til delim
  }

  int skip = 0;
  if (text[plen] == delim) {
    skip = 1;  // Skip over delim for next recursion
  }

  slist_t *rest = s_explode(text + plen + skip, delim);  // Recursively do rest

  char *part = alloca(plen + 2);  // Allocate space for substring
  memcpy(part, text, plen);  // Copy substring

  part[plen] = 0;  // Null-terminate substring

  return s_cons(part, rest);
}
