// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

struct { // LAB3
  struct spinlock lock;
  int refcount[PHYPAGES]; // points to linked list of pages, at the head
} kpage;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&kpage.lock, "kpage"); //LAB3
  for (int i = 0; i < PHYPAGES; i++) {
    kpage.refcount[i] = 0; // init page ref counts to 0
  }
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    kfree(p);
}

// LAB3
// increments ref count for the page at a specific physical address
void increment_ref(uint64 pa) {
  uint64 pageindex = (pa - KERNBASE) / PGSIZE;
  acquire(&kpage.lock);
  kpage.refcount[pageindex]++; // LAB3: set ref to 1 by incrementing by 1
  release(&kpage.lock);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  // LAB3: check to see if no other procs are using this page
  uint64 pageindex = ((uint64)pa - KERNBASE) / PGSIZE;
  acquire(&kpage.lock);
  // if ref > 0, a proc is still using it.
  if (kpage.refcount[pageindex] > 0) {
    kpage.refcount[pageindex]--; // decrease ref by 1
    release(&kpage.lock);
    return; // skips kfree (for now)
  }
  release(&kpage.lock);

  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  // Fill with junk to catch dangling refs.
  memset(pa, 1, PGSIZE);

  r = (struct run*)pa;

  acquire(&kmem.lock);
  r->next = kmem.freelist;
  kmem.freelist = r;
  release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;
  uint64 pa; // physical address of a page

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r) { // LAB3: r exists, so it is not zero
    pa = (uint64)r; 
    increment_ref(pa); // set ref to 1 by incrementing by 1
    memset((char*)r, 5, PGSIZE); // fill with junk
  }

  return (void*)r;
}