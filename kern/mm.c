#include <inc/env.h>
#include <inc/stdio.h>

struct vma* find_vma(void *addr, struct mm_struct *mm){
	struct vma *vma = mm->mm_vma;
	while(vma){
		//cprintf("CONTROL #1 :: %p == %p\n",vma->vma_va , addr);
		if(vma->vma_va <= addr && (vma->vma_va + vma->vma_len > addr) )
			break;
		vma = vma->vma_next;
	}
	return vma;
}

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
	cprintf("TRYING TO MERGE %p with len %d to %p with len %d\n",first->vma_va,first->vma_len,second->vma_va,second->vma_len);
	first->vma_len += second->vma_len;
	first->vma_next = second->vma_next;
	__inject_vma(second, &(second->vma_mm->vma_free_list));
	second->vma_len = 0;
	second->vma_va = 0;
}
