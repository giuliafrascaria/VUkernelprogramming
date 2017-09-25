#include <inc/env.h>
#include <inc/stdio.h>
#include <inc/string.h>

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

void *do_map(struct mm_struct *mm,  void* file, void* addr,
	unsigned int len, int prot, int type){
		cprintf("MAPPING %p with len %d\n", addr, len);
		struct vma *vma = mm->vma_free_list;
		vma->vma_va = ROUNDDOWN(addr, PGSIZE);
		vma->vma_len = ROUNDUP((len + addr) - vma->vma_va, PGSIZE);
		vma->vma_type = type;
		vma->vma_prot = prot;
		vma->vma_file = file;
		mm->vma_free_list = mm->vma_free_list->vma_next;
		__inject_vma(vma, &mm->mm_vma);
		// cprintf("CHECKING \n");
		// for(struct vma *vv = mm->mm_vma; vv; vv = vv->vma_next)
		// 	cprintf("mapped :: va=%p, len=%d\n",vv->vma_va, vv->vma_len);
		++mm->mm_vma_number;
		vma_merge(vma, vma->vma_next);
		vma_merge(find_vma_prev(vma->vma_va, mm), vma);
		return vma->vma_va;
}

void vma_merge(struct vma *first, struct vma *second){
	// Try to merge two vma-s; if they are not mapping contiguous areas- do nothing
	int non_mergable = !first || !second || first == second ||
		(second->vma_va - (first->vma_va + first->vma_len) >= PGSIZE) ||
		(first->vma_prot != second->vma_prot) ||
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
	if( vma->vma_len - (addr - vma->vma_va) >= PGSIZE) // Nothing to split
		return;
	addr = ROUNDDOWN(addr, PGSIZE);
	new = vma->vma_mm->vma_free_list;
	vma->vma_mm->vma_free_list = vma->vma_mm->vma_free_list->vma_next;
       	new->vma_next = vma->vma_next;
	vma->vma_next = new;
	new->vma_va = addr;
	new->vma_prot = vma->vma_prot;
	new->vma_type = vma->vma_type;
	new->vma_len = vma->vma_len - (addr - vma->vma_va);
}

void do_munmap(struct mm_struct *mm, void* addr, unsigned int len){
	addr = ROUNDDOWN(addr, PGSIZE);
	len = ROUNDUP(len, PGSIZE);
	struct vma *vma = find_vma(addr, mm), *vma_prev = find_vma_prev(addr, addr);

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
		struct vma *to_delete = vma->vma_next;
		vma->vma_next = to_delete->vma_next;
		vma = to_delete;
	}
	memset(vma, 0x0, sizeof(struct vma));
	__inject_vma(vma, &mm->vma_free_list);
}
