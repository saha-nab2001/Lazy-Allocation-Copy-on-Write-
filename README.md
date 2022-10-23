# Lazy-Allocation-Copy-on-Write-
Modified xv6 OS code to implement Lazy Allocation and COW functionality

## Lazy Allocation

One of the many neat tricks an O/S can play with page table hardware is lazy allocation of user-space heap memory. 
Xv6 applications ask the kernel for heap memory using the sbrk() system call. 
In the kernel we've given you, sbrk() allocates physical memory and maps it into the process's virtual address space. 
However, there are programs that use sbrk() to ask for large amounts of memory but never use most of it, for example to implement large sparse arrays. 
To optimize for this case, sophisticated kernels allocate user memory lazily. 
That is, sbrk() doesn't allocate physical memory, but just remembers which addresses are allocated. 
When the process first tries to use any given page of memory, the CPU generates a page fault, which the kernel handles by allocating physical memory, zeroing it, and mapping it.

## Copy On Write

When a process forks a child, all the code, data and stack variables are copied to the child processes address space and the child continues its execution. 
However, most of the pages remain unaltered (For example, the pages belonging to the code section). 
Hence, it is better to keep a single copy of pages in physical memory and make both parent and child processes’ virtual addresses point to the same memory.
A new copy of the page is deferred until there is a write to the shared page.
