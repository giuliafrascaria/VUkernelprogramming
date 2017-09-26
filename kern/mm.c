#include <inc/env.h>
#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/types.h>

#include <kern/env.h>
#include <kern/pmap.h>

int vma_map(struct mm_struct *mm, void* va){
	struct vma *vma = find_vma(va, mm);
	if(!vma)
		return -1;
	int perm = __prot2perm(vma->vma_prot);
	if(vma->vma_type == VMA_BINARY){
		region_alloc(curenv, vma->vma_va, vma->vma_len, perm);
		memcpy(vma->vma_bin_va, vma->vma_file, vma->vma_bin_filesz);
		return 0;
	}
	region_alloc(curenv, va, PGSIZE, perm);
	return 0;
}

int __vma_map(struct mm_struct *mm, void* va, size_t size){
	struct vma *vma = find_vma(va, mm);
	if(!vma)
		return -1;
	int perm = __prot2perm(vma->vma_prot);
	if(vma->vma_type == VMA_BINARY){
		region_alloc(curenv, vma->vma_va, vma->vma_len, perm);
		memcpy(vma->vma_bin_va, vma->vma_file, vma->vma_bin_filesz);
		return 0;
	}
	size = MIN(size, vma->vma_len);
	region_alloc(curenv, va, ROUNDUP(size, PGSIZE), perm);
	return 0;
}

struct vma* find_vma(void *addr, struct mm_struct *mm){
	struct vma *vma = mm->mm_vma;
	while(vma){
		if(vma->vma_va <= addr && (vma->vma_va + vma->vma_len > addr) )
			break;
		vma = vma->vma_next;
	}
	return vma;
}

// Return previous vma (returns find_vma for first entry in vma_list)
struct vma* find_vma_prev(void *addr, struct mm_struct *mm){
	struct vma *vma = mm->mm_vma;
	while(vma && vma->vma_next){
		if(vma->vma_next->vma_va <= addr && (vma->vma_next->vma_va + vma->vma_next->vma_len > addr) )
			break;
		vma = vma->vma_next;
	}
	return vma;
}

void *do_map(struct mm_struct *mm,  void* file, size_t filesz, void* addr,
	unsigned int len, int prot, int type){
		struct vma *vma = mm->vma_free_list;
		if(!page_free_list)
			return (void*)-1;
		vma->vma_va = ROUNDDOWN(addr, PGSIZE);
		vma->vma_len = ROUNDUP((len + addr) - vma->vma_va, PGSIZE);
		vma->vma_type = type;
		vma->vma_mm = mm;
		vma->vma_prot = prot;
		vma->vma_file = file;
		vma->vma_bin_va = addr;
		vma->vma_bin_filesz = filesz;
		mm->vma_free_list = mm->vma_free_list->vma_next;
		__inject_vma(vma, &mm->mm_vma);
		vma_merge(vma, vma->vma_next);
		vma_merge(find_vma_prev(vma->vma_va, mm), vma);
		return vma->vma_va;
}

void vma_merge(struct vma *first, struct vma *second){
	// Try to merge two vma-s; if they are not mapping contiguous areas- do nothing
	int non_mergable = !first || !second || first == second ||
		(second->vma_va - (first->vma_va + first->vma_len) >= PGSIZE) ||
		(first->vma_prot != second->vma_prot) || first->vma_type == VMA_BINARY ||
		first->vma_type != second->vma_type;
	if(non_mergable)
	      return;
	first->vma_len += second->vma_len;
	first->vma_next = second->vma_next;
	__inject_vma(second, &(second->vma_mm->vma_free_list));
	second->vma_len = 0;
	second->vma_va = 0;
}

void vma_split(struct vma *vma, void* addr){
	struct vma *new;
	if( vma->vma_len - (addr - vma->vma_va) < PGSIZE) // Nothing to split
		return;
	addr = ROUNDDOWN(addr, PGSIZE);
	new = vma->vma_mm->vma_free_list;
	vma->vma_mm->vma_free_list = vma->vma_mm->vma_free_list->vma_next;
	new->vma_va = addr;
	new->vma_prot = vma->vma_prot;
	new->vma_type = vma->vma_type;
	new->vma_len = vma->vma_len - (addr - vma->vma_va);
	vma->vma_len = addr - vma->vma_va;
	__inject_vma(new, &(vma->vma_mm->mm_vma));
}

void do_munmap(struct mm_struct *mm, void* addr, unsigned int len){

	addr = ROUNDDOWN(addr, PGSIZE);
	len = ROUNDUP(len, PGSIZE);
	struct vma *to_delete;
	struct vma *vma = find_vma(addr, mm), *vma_prev = find_vma_prev(addr, mm);

	if(!vma)
		return;

	region_dealloc(curenv, addr, len);

	if(addr != vma->vma_va)
		vma_split(vma, addr);
	if(vma->vma_next->vma_len > len)
		vma_split(vma->vma_next, vma->vma_next->vma_va + len);
	if(addr == vma->vma_va){
		if(vma_prev == vma){
			// vma is in the beginning of vma_list;
			mm->mm_vma = vma->vma_next;
		} else {
			vma_prev->vma_next = vma->vma_next;
		}
	} else {
		to_delete = vma->vma_next;
		vma->vma_next = to_delete->vma_next;
		//vma->vma_len -= len;
		vma = to_delete;
	}
	memset(vma, 0x0, sizeof(struct vma));
	__inject_vma(vma, &mm->vma_free_list);
}


void* find_empty_space(size_t size, struct mm_struct *mm, int prot, int type){
	struct vma *vma = mm->mm_vma;
	int vma_fit;
	if(!vma)
		return (void*)0x0;
	while(vma){
		if(!vma->vma_next)
			break;
		vma_fit = vma->vma_type & type;
		vma_fit &= vma->vma_prot & prot;
		if(vma_fit && vma->vma_next->vma_va - ( vma->vma_va + vma->vma_len) >= size)
		       return vma->vma_va + vma->vma_len + 1; // there is perfect area after vma->vma_va;
		vma = vma->vma_next;
	};

	vma = mm->mm_vma;
	while(vma){
		size_t limit = (unsigned int)vma->vma_va + vma->vma_len;
		if(!vma->vma_next && ((USTACKTOP - PGSIZE) >= (size + limit))){
			return vma->vma_va + vma->vma_len + 1;
		}
		if(vma->vma_next && (vma->vma_next->vma_va - limit >= (void*)size))
			return vma->vma_va + vma->vma_len + 1;
		vma = vma->vma_next;
	}
	return (void*)-1; // There is no empty space {We don't implement swapping yet}
}


void madvise(struct mm_struct *mm, void *addr, size_t size, int flags){
	switch(flags){
		case MADV_WILLNEED:
			__vma_map(addr, mm, size);
			break;
		case MADV_DONTNEED:
			region_dealloc(container_of(mm, struct env, env_mm), addr, size);
		}
}

int vma_protect(struct mm_struct *mm, void *addr, size_t size, int flags){
	struct vma *vma = find_vma(addr, mm);
	if(!vma)
		return -1;
	if(vma->vma_prot == flags)
		return 0; // Nothing to do;
	size += addr - vma->vma_va;
	size = MIN(size, vma->vma_len);
	vma_split(vma, addr + size);
	vma->vma_prot = flags;
	return 0;
}
