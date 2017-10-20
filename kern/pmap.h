/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_PMAP_H
#define JOS_KERN_PMAP_H
#ifndef JOS_KERNEL
# error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <inc/memlayout.h>
#include <inc/assert.h>

struct env;

extern char bootstacktop[], bootstack[];

extern struct page_info *pages;
extern struct page_info *page_free_list;
extern struct pg_swap_entry *pgswap_free_list;
extern size_t npages;

extern pde_t *kern_pgdir;

struct pg_swap_entry{
  struct env *pse_env;
  void *pse_va;
  struct pg_swap_entry *pse_next;
};


/*
 * This macro takes a kernel virtual address -- an address that points above
 * KERNBASE, where the machine's maximum 256MB of physical memory is mapped --
 * and returns the corresponding physical address.  It panics if you pass it a
 * non-kernel virtual address.
 */
#define PADDR(kva) _paddr(__FILE__, __LINE__, kva)
#define SWAP_SLOTS_NUMBER (128 * 1024 * 1024 / PGSIZE)

static inline physaddr_t _paddr(const char *file, int line, void *kva)
{
    if ((uint32_t)kva < KERNBASE)
        _panic(file, line, "PADDR called with invalid kva %08lx", kva);
    return (physaddr_t)kva - KERNBASE;
}

/* This macro takes a physical address and returns the corresponding kernel
 * virtual address.  It panics if you pass an invalid physical address. */
#define KADDR(pa) _kaddr(__FILE__, __LINE__, pa)

static inline void *_kaddr(const char *file, int line, physaddr_t pa)
{
    if (PGNUM(pa) >= npages)
        _panic(file, line, "KADDR called with invalid pa %08lx", pa);
    return (void *)(pa + KERNBASE);
}

static inline void add_page_free_entry(struct page_info *pp){
	pp->pp_link = page_free_list;
	pp->pp_flags = 0x0;
	page_free_list = pp;
}

enum {
    /* For page_alloc, zero the returned physical page. */
    ALLOC_ZERO = 1<<0,
    ALLOC_HUGE = 1<<1,
    ALLOC_PREMAPPED = 1<<2,
};

enum {
    /* For pgdir_walk, tells whether to create normal page or huge page */
    CREATE_NORMAL = 1<<0,
    CREATE_HUGE   = 1<<1,
};

void mem_init(void);

void page_init(void);
struct page_info *page_alloc(int alloc_flags);
void page_free(struct page_info *pp);
int page_insert(pde_t *pgdir, struct page_info *pp, void *va, int perm);
void page_remove(pde_t *pgdir, void *va);
struct page_info *page_lookup(pde_t *pgdir, void *va, pte_t **pte_store);
void page_decref(struct page_info *pp);

void boot_map_region(pde_t*, uintptr_t, size_t, physaddr_t, int);

void tlb_invalidate(pde_t *pgdir, void *va);

void *mmio_map_region(physaddr_t pa, size_t size);

int  user_mem_check(struct env *env, const void *va, size_t len, int perm);
void user_mem_assert(struct env *env, const void *va, size_t len, int perm);

struct pg_swap_entry *pgswap_alloc();
void pgswap_free(struct pg_swap_entry*);

size_t page_swap_out(struct page_info*);
struct page_info *page_swap_in(struct env*, void *va);

static inline physaddr_t page2pa(struct page_info *pp)
{
    return (pp - pages) << PGSHIFT;
}

static inline struct page_info *pa2page(physaddr_t pa)
{
    if (PGNUM(pa) >= npages)
        panic("pa2page called with invalid pa");
    return &pages[PGNUM(pa)];
}

static inline void *page2kva(struct page_info *pp)
{
    return KADDR(page2pa(pp));
}

pte_t *pgdir_walk(pde_t *pgdir, const void *va, int create);

#endif /* !JOS_KERN_PMAP_H */
