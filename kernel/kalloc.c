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

struct ref_locked ref_lock;

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&ref_lock.lock, "ref_lock"); //LAB3
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE) {
    uint64 pageindex = (uint64)p / PGSIZE;
    acquire(&ref_lock.lock);
    ref_lock.refcount[pageindex] = 1; // init page ref count to 1 - kfree() = 0
    release(&ref_lock.lock);
    kfree(p);
  }
}

// LAB3
// increments ref count for the page at a specific physical address
void increment_ref(uint64 pa) {
  uint64 pageindex = (uint64)pa / PGSIZE;
  acquire(&ref_lock.lock);
  ref_lock.refcount[pageindex]++; // LAB3: set ref to 1 by incrementing by 1
  release(&ref_lock.lock);
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  // LAB3: check to see if no other procs are using this page
  uint64 pageindex = (uint64)pa / PGSIZE;
  acquire(&ref_lock.lock);
  ref_lock.refcount[pageindex]--; // decrease ref by 1
  release(&ref_lock.lock);
  
  if (ref_lock.refcount[pageindex] == 0) { // if ref == 0, no procs are using the page.
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
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r) // if 0x0, aka not null
    kmem.freelist = r->next; // then go to the next one
  release(&kmem.lock);

  if(r) { // if 0x0, aka not null
    memset((char*)r, 5, PGSIZE); // fill with junk

    uint64 pageindex = (uint64)r / PGSIZE;
    acquire(&ref_lock.lock);
    ref_lock.refcount[pageindex] = 1; // set ref to 1
    release(&ref_lock.lock);
  }

  return (void*)r;
}