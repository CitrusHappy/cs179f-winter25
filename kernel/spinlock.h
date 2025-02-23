#include "memlayout.h"
#include "param.h"

// Mutual exclusion lock.
struct spinlock {
  uint locked;       // Is the lock held?

  // For debugging:
  char *name;        // Name of lock.
  struct cpu *cpu;   // The cpu holding the lock.
  uint n;
  uint nts;
};

extern struct ref_locked{ // LAB3
  struct spinlock lock;
  int refcount[PHYSTOP/PGSIZE]; // points to linked list of pages, at the head
} ref_lock;
