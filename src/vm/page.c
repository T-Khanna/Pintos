#include "vm/page.h"
#include "vm/frame.h"
#include <stdio.h>
#include <string.h>
#include "threads/malloc.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"

static unsigned supp_pte_hash_func(const struct hash_elem *elem,
    void *aux UNUSED);
static bool supp_pte_less_func(const struct hash_elem *a,
    const struct hash_elem *b, void *aux UNUSED);
static void delete_supp_pte(struct hash_elem *elem, void *aux UNUSED);
static void free_pte_related_resources(struct hash_elem *elem, void *pd_);

/* Initialises a supplementary page table, and returns whether it was successful
   in doing so */
bool supp_page_table_init(struct hash *table)
{
  return hash_init(table, supp_pte_hash_func, supp_pte_less_func, NULL);
}

/* Destroy all of the elements of a supplementary page table.
   Takes appropriate action to deallocate resources for each entry. */
void supp_page_table_destroy(struct hash *table)
{
  ASSERT(table != NULL);
  hash_apply(table, &free_pte_related_resources);
  hash_destroy(table, &delete_supp_pte);
}

/* Take appropriate action for a supplemntary page table entry when a process
   exits.*/
static void free_pte_related_resources(struct hash_elem *elem, void *aux UNUSED)
{
  struct supp_page *entry
      = hash_entry(elem, struct supp_page, hash_elem);
  switch (entry->status) {
    case LOADED:;
      void *kaddr = pagedir_get_page(thread_current()->pagedir, entry->vaddr);
      ASSERT(kaddr != NULL);
      frame_free_page(kaddr);
      break;

    default:
      break;
  }
}

/* Returns the supplementary page table entry corresponding to the page vaddr
   in hash if it exists, or null otherwise */
struct supp_page * supp_page_table_get(struct hash *hash, void *vaddr)
{
  struct supp_page entry;
  entry.vaddr = vaddr;
  struct hash_elem *found = hash_find(hash, &entry.hash_elem);
  return found == NULL ? NULL : hash_entry(found, struct supp_page,
      hash_elem);
}

/* Inserts the page vaddr into the supplemtary page table unless an entry for it
   already exists, in which case it sets its status. */
void supp_page_table_insert(struct hash *hash, void *vaddr,
                            enum page_status_t status)
{
  ASSERT(hash != NULL);
  struct supp_page *entry = malloc(sizeof(struct supp_page));
  ASSERT(entry != NULL);
  entry->vaddr = pg_round_down(vaddr);
  entry->status = status;
  struct hash_elem *prev = hash_insert(hash, &entry->hash_elem);
  if (prev != NULL) {
    /* there was already an entry, mark it with status */
    hash_entry(prev, struct supp_page, hash_elem)->status = status;
  }
}

/* Remove and free the entry in the supplementary page table for vaddr. */
void supp_page_table_remove(struct hash *hash, void *vaddr)
{
  struct supp_page page;
  page.vaddr = vaddr;
  struct hash_elem *found = hash_delete(hash, &page.hash_elem);
  if (found == NULL) {
    PANIC("Deleting non-existent item from spt!\n");
  }
  delete_supp_pte(found, NULL);
}

/* function for hashing the page pointer in a supplementary page table entry */
static unsigned supp_pte_hash_func(const struct hash_elem *elem,
    void *aux UNUSED)
{
  struct supp_page *entry
      = hash_entry(elem, struct supp_page, hash_elem);
  return hash_bytes(&entry->vaddr, sizeof(entry->vaddr));
}

/* function for comparing the addresses of two supplemtary page table entries */
static bool supp_pte_less_func(const struct hash_elem *a,
    const struct hash_elem *b, void *aux UNUSED)
{
  struct supp_page *a_entry
      = hash_entry(a, struct supp_page, hash_elem);
  struct supp_page *b_entry
      = hash_entry(b, struct supp_page, hash_elem);
  return a_entry->vaddr < b_entry->vaddr;
}

/* Free the memory used by an entry in the supplementary page table */
static void delete_supp_pte(struct hash_elem *elem, void *aux UNUSED)
{
  free(hash_entry(elem, struct supp_page, hash_elem));
}
