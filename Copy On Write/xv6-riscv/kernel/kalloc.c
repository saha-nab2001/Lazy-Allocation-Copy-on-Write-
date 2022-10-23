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

//Added
/**********************************************/
struct {
  struct spinlock page_lock;
  int cnt[PGROUNDUP(PHYSTOP)>>12];
} page_reference;

void initialize_page_reference(){
  initlock(&page_reference.page_lock, "page_ref");
  acquire(&page_reference.page_lock);
  memset(page_reference.cnt, 0, (PGROUNDUP(PHYSTOP)>>12)*sizeof(int));
  release(&page_reference.page_lock);
}

void increase_page_reference(void*pa){
  acquire(&page_reference.page_lock);
  if(page_reference.cnt[(uint64)pa>>12]<0){
    panic("increase_page_reference");
  }
  page_reference.cnt[(uint64)pa>>12]+=1;
  release(&page_reference.page_lock);
}
/**********************************************/

void
kinit()
{
  initialize_page_reference();
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE) {
    //Added
    /***********************/
    increase_page_reference(p);
    /***********************/
    kfree(p);
  }
}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  //Added
  /***********************************************/
  acquire(&page_reference.page_lock);
  page_reference.cnt[(uint64)pa>>12]-=1;
  if(page_reference.cnt[(uint64)pa>>12] > 0) {
    release(&page_reference.page_lock);
    return;
  }
  release(&page_reference.page_lock);
  /***********************************************/

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

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r) {
    memset((char*)r, 5, PGSIZE); // fill with junk
    //Added
    /**************************/
    increase_page_reference((void*)r);
    /**************************/
  }
  return (void*)r;
}
