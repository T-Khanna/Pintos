#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>

struct supp_page_table_entry {
  void *vaddr,                /* The virtual address of this page */
  page_status_t status,       /* The status of this page */
  struct hash_elem hash_elem  /* Bookkeeping */
}

/* The status of a page */
enum page_status_t {
  LOADED,   /* In memory */
  SWAPPED,  /* Swapped out to disk */
  ZEROED,   /* Zeroed out page */
  EXEC_LAZY /* Part of a lazy loaded executable, not yet loaded */
}

bool supp_page_table_init(struct hash *table);
void supp_page_table_destroy(struct hash *table);
struct supp_page_table_entry * supp_page_table_get(struct hash *hash,
    void *vaddr);
struct supp_page_table_entry * supp_page_table_insert(struct hash *hash,
    struct supp_page_table_entry *entry);

#endif /* vm/page.h */
