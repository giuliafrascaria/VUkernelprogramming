#ifndef MM_H
#define MM_H

#include <inc/mmu.h>
#include <inc/types.h>
#include <inc/stdio.h>

struct mm_struct {
	struct vma *vma_free_list;
	struct vma *mm_common_vma;
	struct vma *mm_vma;
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
	MADV_DONTNEED,
	MADV_WILLNEED,
};

enum {
	PROT_READ = 1,
	PROT_WRITE,
	PROT_EXEC,
	PROT_NONE
};

struct vma {
    char vma_type;
		char vma_prot;
    void *vma_va;
    size_t vma_len;
		struct vma *vma_next;
		struct mm_struct *vma_mm;
		void* vma_file; // For elf binary
		void* vma_bin_va;
		size_t vma_bin_filesz;
    /* LAB 4: You may add more fields here, if required. */
};

int vma_map(struct mm_struct *mm, void* va);
/* Look up the first VMA which satisfies  addr < vm_end,  NULL if none. */
struct vma* find_vma(void *addr, struct mm_struct *mm);
struct vma* find_vma_prev(void *addr, struct mm_struct *mm);
void* find_empty_space(size_t, struct mm_struct*, int type, int prot);

void* do_map(struct mm_struct *mm,  void* file, size_t filesz, void* addr, unsigned int len,
											int prot, int type);
void do_munmap(struct mm_struct *mm, void* addr, unsigned int len);

void vma_merge(struct vma *first, struct vma *second);
void vma_split(struct vma *vma, void* addr);

void madvise(struct mm_struct *mm, void *addr, size_t size, int flags);
int vma_protect(struct mm_struct *mm, void *addr, size_t size, int flags);

static inline int __prot2perm(int prot){
	int perm = 0;
	int writable = (prot & PROT_WRITE);
	//writable = writable && !(prot & PROT_EXEC);
	perm = writable? PTE_W : 0;
	perm |= PTE_U;
	return perm;
}

static inline void __inject_vma(struct vma* vma, struct vma **hlist){
	while(*hlist){
		if(vma->vma_va < (*hlist)->vma_va)
			break;
		hlist = &((*hlist)->vma_next);
	}
	vma->vma_next = (*hlist);
	(*hlist) = vma;
}


#endif
