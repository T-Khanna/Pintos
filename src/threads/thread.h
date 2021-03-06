#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <hash.h>
#include <stdint.h>
#include "threads/synch.h"
#include "threads/fixed-point.h"
#ifdef USERPROG
#include <hash.h>
#include "userprog/process.h"
#endif

/* States in a thread's life cycle. */
enum thread_status
  {
    THREAD_RUNNING,     /* Running thread. */
    THREAD_READY,       /* Not running but ready to run. */
    THREAD_BLOCKED,     /* Waiting for an event to trigger. */
    THREAD_DYING        /* About to be destroyed. */
  };

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;

#define TID_ERROR ((tid_t) -1)          /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                       /* Lowest priority. */
#define PRI_DEFAULT 31                  /* Default priority. */
#define PRI_MAX 63                      /* Highest priority. */

/* Thread nice limits */
#define NICE_MIN -20                    /* Lowest nice. */
#define NICE_MAX 20                     /* Highest nice. */

/* A kernel thread or user process.

   Each thread structure is stored in its own 4 kB page.  The
   thread structure itself sits at the very bottom of the page
   (at offset 0).  The rest of the page is reserved for the
   thread's kernel stack, which grows downward from the top of
   the page (at offset 4 kB).  Here's an illustration:

        4 kB +---------------------------------+
             |          kernel stack           |
             |                |                |
             |                |                |
             |                V                |
             |         grows downward          |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             |                                 |
             +---------------------------------+
             |              magic              |
             |                :                |
             |                :                |
             |               name              |
             |              status             |
        0 kB +---------------------------------+

   The upshot of this is twofold:

      1. First, `struct thread' must not be allowed to grow too
         big.  If it does, then there will not be enough room for
         the kernel stack.  Our base `struct thread' is only a
         few bytes in size.  It probably should stay well under 1
         kB.

      2. Second, kernel stacks must not be allowed to grow too
         large.  If a stack overflows, it will corrupt the thread
         state.  Thus, kernel functions should not allocate large
         structures or arrays as non-static local variables.  Use
         dynamic allocation with malloc() or palloc_get_page()
         instead.

   The first symptom of either of these problems will probably be
   an assertion failure in thread_current(), which checks that
   the `magic' member of the running thread's `struct thread' is
   set to THREAD_MAGIC.  Stack overflow will normally change this
   value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in
   the run queue (thread.c), or it can be an element in a
   semaphore wait list (synch.c).  It can be used these two ways
   only because they are mutually exclusive: only a thread in the
   ready state is on the run queue, whereas only a thread in the
   blocked state is on a semaphore wait list. */
struct thread
  {
    /* Owned by thread.c. */
    tid_t tid;                          /* Thread identifier. */
    enum thread_status status;          /* Thread state. */
    char name[16];                      /* Name (for debugging purposes). */
    uint8_t *stack;                     /* Saved stack pointer. */

    int priority;                       /* Priority. */
    int effective_priority;             /* Effective Priority */

    int nice;                           /* Nice value for the BSD Scheduler. */
    fixed_point recent_cpu;             /* Recent CPU time received. */

    struct list_elem allelem;           /* List element for all threads list. */

    struct list_elem sleep_elem;        /* List element for sleeping_threads */
    struct semaphore timer;             /* Semaphore for controlling the
                                           sleeping threads */
    int wake_up_time;                   /* Tick value at which the thread
                                           should wake up. */
    /* Shared between thread.c and synch.c. */
    struct list_elem elem;              /* List element. */

    struct list locks_held;             /* List of currently held locks */
    struct lock *lock_to_acquire;       /* Lock currently acquired by another
                                           thread */

#ifdef USERPROG
    /* Owned by userprog/process.c. */
    uint32_t *pagedir;                  /* Page directory */
    struct list descriptors;            /* List of all file descriptors */
    struct process *process;            /* Information about the process this
                                           thread executes. */
    struct list child_processes;        /* List of child processes created
                                           by the thread. */
#endif

#ifdef VM
    struct hash supp_page_table;        /* Supplementary page table */
    struct hash mapid_page_table;       /* Mapping link between mapid and user
                                           virtual addresses */
    struct hash mmap_file_page_table;   /* Memory mapped files */
    struct hash swap_table;             /* Thread's swap table */
    void **esp;
#endif

    /* Owned by thread.c. */
    unsigned magic;                     /* Detects stack overflow. */
  };

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

void thread_init (void);
void thread_start (void);

void thread_tick (int ticks);
void thread_print_stats (void);

typedef void thread_func (void *aux);
tid_t thread_create (const char *name, int priority, thread_func *, void *);

void thread_block (void);
void thread_unblock (struct thread *);

void yield_if_higher_priority_ready(void);

struct thread *thread_current (void);
tid_t thread_tid (void);
const char *thread_name (void);

void thread_exit (void) NO_RETURN;
void thread_yield (void);

/* Performs some operation on thread t, given auxiliary data AUX. */
typedef void thread_action_func (struct thread *t, void *aux);
void thread_foreach (thread_action_func *, void *);

int thread_get_priority (void);
void thread_set_priority (int);
void update_priority(struct thread *, void *aux UNUSED);

int thread_get_nice (void);
void thread_set_nice (int);
int thread_get_recent_cpu (void);
int thread_get_load_avg (void);

/* Donates priority to the provided thread */
void thread_donate_priority(struct thread *doner, struct thread *donee);
/* Removes donation_elem from the provided thread, and gets backthe priority */
void thread_withdraw_priority(struct list *list, struct thread *t);

void thread_update_effective_priority(struct thread *t);
void thread_update_effective_p(struct thread *t, int p);

/* Returns maximum of two priorities given. */
int priority_max(int p1, int p2);

/* Function that compares list items by wake up time. */
bool cmp_wake_time(const struct list_elem *l1, const struct list_elem *l2,
                   void *aux);
/* Returns true if the first element has higher priority than the second */
bool higher_priority(const struct list_elem *l1, const struct list_elem *l2,
                        void *aux UNUSED);


struct list * get_all_list(void);

#endif /* threads/thread.h */
