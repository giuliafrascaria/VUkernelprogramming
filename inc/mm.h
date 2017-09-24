#ifndef MM_H
#define MM_H

#include <inc/types.h>

struct mm_struct {
	struct vma *vma_free_list;
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
}

struct vma {
    int vma_type;
    void *vma_va;
    size_t vma_len;
    int vma_perm;
		struct vma *vma_next, *vma_prev;
		struct mm_struct *vma_mm;
		void* vma_file; // For elf binary
    /* LAB 4: You may add more fields here, if required. */
};

/* Look up the first VMA which satisfies  addr < vm_end,  NULL if none. */
struct vma* find_vma(unsigned long addr, struct mm_struct mm);
struct vma* find_vma_prev(unsigned long addr, struct mm_struct mm);

unsigned int do_map(struct mm_struct *mm,  void* file, unsigned int addr, unsigned int len,
											int prot, int type);
unsigned int do_munmap(struct mm_struct *mm, unsigned int addr, unsigned int len);

int vma_merge(struct vma *first, struct vma *second);
int vma_split(struct vma *vma, unsigned int addr);



#endif
