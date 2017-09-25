#ifndef MM_H
#define MM_H

#include <inc/types.h>
#include <inc/stdio.h>

struct mm_struct {
	struct vma *vma_free_list;
	struct vma *mm_common_vma;
	struct vma *mm_vma;
	size_t mm_virt_page; // amount of mapped virtual pages
	size_t mm_vma_number; // amount of vma area
};



/* Anonymous VMAs are zero-initialized whereas binary VMAs
 * are filled-in from the ELF binary.
 */
enum {
    VMA_UNUSED,
    VMA_ANON,
    VMA_BINARY,
};


enum {
	PROT_READ = 1,
	PROT_WRITE,
	PROT_EXEC,
	PROT_NONE
};

struct vma {
    int vma_type;
    void *vma_va;
    size_t vma_len;
    int vma_prot;
		struct vma *vma_next;
		struct mm_struct *vma_mm;
		void* vma_file; // For elf binary
    /* LAB 4: You may add more fields here, if required. */
};

/* Look up the first VMA which satisfies  addr < vm_end,  NULL if none. */
struct vma* find_vma(void *addr, struct mm_struct *mm);
struct vma* find_vma_prev(void *addr, struct mm_struct *mm);

void * do_map(struct mm_struct *mm,  void* file, void* addr, unsigned int len,
											int prot, int type);
void do_munmap(struct mm_struct *mm, void* addr, unsigned int len);

void vma_merge(struct vma *first, struct vma *second);
void vma_split(struct vma *vma, unsigned int addr);

static inline int __prot2perm(int prot){
	int perm = 0;
	int writable = (prot & PROT_WRITE);
	writable = writable && !(prot & PROT_EXEC);
	perm = writable? PTE_W : 0;
	perm |= PTE_U;
	return perm;
}

static inline void __inject_vma(struct vma* vma, struct vma **hlist){
	//cprintf("INJECTING %p\n", *hlist);
	while(*hlist){
		if(vma->vma_va < (*hlist)->vma_va)
			break;
		hlist = &((*hlist)->vma_next);
	}
	vma->vma_next = (*hlist);
	(*hlist) = vma;
}


#endif
