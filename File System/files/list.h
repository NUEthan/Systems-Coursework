#ifndef __LIBS_LIST_H__
#define __LIBS_LIST_H__

#ifndef __ASSEMBLER__

// Offset of member in struct
#define offsetof(type, member)((size_t)(&((type *)0)->member))

// Convert pointer within struct to pointer to struct
// args:
// ptr: struct poitner to member
// type: type of struct
// member: name of member
#define to_struct(ptr, type, member)((type *)((char *)(ptr) - offsetof(type, member)))

//Struct for list entry
struct list_entry {
    struct list_entry *prev, *next;
};

typedef struct list_entry list_entry_t;
static inline void list_init(list_entry_t *elm) __attribute__((always_inline));
static inline void list_add(list_entry_t *listelm, list_entry_t *elm) __attribute__((always_inline));
static inline void list_add_before(list_entry_t *listelm, list_entry_t *elm) __attribute__((always_inline));
static inline void list_add_after(list_entry_t *listelm, list_entry_t *elm) __attribute__((always_inline));
static inline void list_del(list_entry_t *listelm) __attribute__((always_inline));
static inline void list_del_init(list_entry_t *listelm) __attribute__((always_inline));
static inline int list_empty(list_entry_t *list) __attribute__((always_inline));
static inline list_entry_t *list_next(list_entry_t *listelm) __attribute__((always_inline));
static inline list_entry_t *list_prev(list_entry_t *listelm) __attribute__((always_inline));
static inline void __list_add(list_entry_t *elm, list_entry_t *prev, list_entry_t *next) __attribute__((always_inline));
static inline void __list_del(list_entry_t *prev, list_entry_t *next) __attribute__((always_inline));

// Initializes list entry to point to itself
// - elm: List entry to init
static inline void list_init(list_entry_t *elm) {
  elm->prev = elm->next = elm;
}

// Adds new entry after specified entry in list
// - listelm: Existing list entry
// - elm: New list entry
static inline void list_add(list_entry_t *listelm, list_entry_t *elm) {
  list_add_after(listelm, elm);
}

//  Adds new entry before specified entry in list
// - listelm: Existing list entry
// - elm: New list entry
static inline void list_add_before(list_entry_t *listelm, list_entry_t *elm) {
  __list_add(elm, listelm->prev, listelm);
}

// Adds new entry after specified entry in list
// - listelm: Existing list entry
// - elm: New list entry
static inline void list_add_after(list_entry_t *listelm, list_entry_t *elm) {
  __list_add(elm, listelm, listelm->next);
}

// Removes an entry from the list
// - listelm: List entry
static inline void list_del(list_entry_t *listelm) {
  __list_del(listelm->prev, listelm->next);
}


// Removes entry from list & reinitializes it
// - listelm: List entry
static inline void list_del_init(list_entry_t *listelm) {
  list_del(listelm);
  list_init(listelm);
}

// Checks if list is empty
// - list: List to be checked
static inline int list_empty(list_entry_t *list) {
  return list->next == list;
}

// list_next: Retrieves next entry in list
// - listelm: Current list entry
static inline list_entry_t *list_next(list_entry_t *listelm) {
  return listelm->next;
}

// Retrieves previous entry in list
// - listelm: Current list entry
static inline list_entry_t *list_prev(list_entry_t *listelm) {
  return listelm->prev;
}

// Internal function to insert entry between 2 entries
// - elm: New entry to be added
// - prev: Entry before the new entry
// - next: Entry after the new entry
static inline void __list_add(list_entry_t *elm, list_entry_t *prev, list_entry_t *next) {
  prev->next = next->prev = elm;
  elm->next = next;
  elm->prev = prev;
}

// Internal function to delete list entry by adjusting adjacent entries
// - prev: Entry before deleted one
// - next: Entry after deleted one
static inline void __list_del(list_entry_t *prev, list_entry_t *next) {
  prev->next = next;
  next->prev = prev;
}

#endif /* !__ASSEMBLER__ */
#endif /* !__LIBS_LIST_H__ */
