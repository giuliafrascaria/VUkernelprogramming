#include <inc/utils.h>

void delete_case1(struct rb_node *rb_node);

struct rb_node* sibling(struct rb_node *rb_node){
	if (rb_node == rb_node->rb_parent->rb_left)
		return rb_node->rb_parent->rb_right;
	else
		return rb_node->rb_parent->rb_left;
}

void delete_case6(struct rb_node *rb_node){
	struct rb_node *s = sibling(rb_node);

	s->rb_color = rb_node->rb_parent->rb_color;
    rb_node->rb_parent->rb_color = RB_BLACK;

	if (rb_node == rb_node->rb_parent->rb_left) {
        s->rb_right->rb_color = RB_BLACK;
		rb_left_rotate(rb_node->rb_parent);
	} else {
		s->rb_left->rb_color = RB_BLACK;
		rb_right_rotate(rb_node->rb_parent);
	}
}

void delete_case5(struct rb_node *rb_node){
	struct rb_node *s = sibling(rb_node);

	if  (s->rb_color == RB_BLACK) {
		if ((rb_node == rb_node->rb_parent->rb_left) &&
		    (s->rb_right->rb_color == RB_BLACK) &&
		    (s->rb_left->rb_color == RB_RED)) {
			s->rb_color = RB_RED;
			s->rb_left->rb_color = RB_BLACK;
			rb_right_rotate(s);
		} else if ((rb_node == rb_node->rb_parent->rb_right) &&
		           (s->rb_left->rb_color == RB_BLACK) &&
		           (s->rb_right->rb_color == RB_RED)) {
			s->rb_color = RB_RED;
			s->rb_right->rb_color = RB_BLACK;
			rb_left_rotate(s);
		}
	}
	delete_case6(rb_node);
}

void delete_case4(struct rb_node *rb_node){
	struct rb_node *s = sibling(rb_node);

	if ((rb_node->rb_parent->rb_color == RB_RED) &&
	    (s->rb_color == RB_BLACK) &&
	    (s->rb_left->rb_color == RB_BLACK) &&
	    (s->rb_right->rb_color == RB_BLACK)) {
		s->rb_color = RB_RED;
		rb_node->rb_parent->rb_color = RB_BLACK;
	} else
		delete_case5(rb_node);
}

void delete_case3(struct rb_node *rb_node){
	struct rb_node *s = sibling(rb_node);

	if ((rb_node->rb_parent->rb_color == RB_BLACK) &&
	    (s->rb_color == RB_BLACK) &&
	    (s->rb_left->rb_color == RB_BLACK) &&
	    (s->rb_right->rb_color == RB_BLACK)) {
		s->rb_color = RB_RED;
		delete_case1(rb_node->rb_parent);
	} else
		delete_case4(rb_node);
}

void delete_case2(struct rb_node *rb_node){
	struct rb_node *s = sibling(rb_node);

	if (s->rb_color == RB_RED) {
		rb_node->rb_parent->rb_color = RB_RED;
		s->rb_color = RB_BLACK;
		if (rb_node == rb_node->rb_parent->rb_left)
			rb_left_rotate(rb_node->rb_parent);
		else
			rb_right_rotate(rb_node->rb_parent);
	}
	delete_case3(rb_node);
}

void delete_case1(struct rb_node *rb_node){
	if (rb_node->rb_parent != NULL)
		delete_case2(rb_node);
}


struct rb_node* rb_search(struct rb_node *rb_root, struct rb_node *rb_node, int(*rb_node_cmp)(struct rb_node*, struct rb_node*)){
	while(rb_root){
		if(rb_node_cmp(rb_root, rb_node)){
			return rb_root;
		}
		if(rb_node_cmp(rb_root, rb_node) > 0){
			if(!rb_node->rb_right)
				return rb_root;
			rb_root = rb_node->rb_right;
		} else{
			if(!rb_node->rb_left)
				return rb_root;
			rb_root = rb_node->rb_left;
		}
	}
	return NULL;
}


struct rb_node* __rb_search(struct rb_node *rb_root, struct rb_node *rb_node, int(*rb_node_cmp)(struct rb_node*, struct rb_node*)){
	while(rb_root){
		if(rb_node_cmp(rb_root, rb_node)){
			return rb_root;
		}
		if(rb_node_cmp(rb_root, rb_node) > 0){
			if(!rb_node->rb_right)
				return NULL;
			rb_root = rb_node->rb_right;
		} else{
			if(!rb_node->rb_left)
				return NULL;
			rb_root = rb_node->rb_left;
		}
	}
	return NULL;
}

void rb_left_rotate(struct rb_node *rb_node) {
	struct rb_node *pivot = rb_node->rb_right;
	pivot->rb_parent = rb_node->rb_parent;
	if (rb_node->rb_parent != NULL) {
		if (rb_node->rb_parent->rb_left == rb_node)
			rb_node->rb_parent->rb_left = pivot;
		else
      rb_node->rb_parent->rb_right = pivot;
  }

	rb_node->rb_right = pivot->rb_left;
	if (pivot->rb_left != NULL)
		pivot->rb_left->rb_parent = rb_node;

	rb_node->rb_parent = pivot;
	pivot->rb_left = rb_node;
}

void rb_right_rotate(struct rb_node *rb_node){
	struct rb_node *pivot = rb_node->rb_left;

	pivot->rb_parent = rb_node->rb_parent;
	if (rb_node->rb_parent != NULL) {
	  if (rb_node->rb_parent->rb_left == rb_node)
	    rb_node->rb_parent->rb_left = pivot;
	  else
	    rb_node->rb_parent->rb_right = pivot;
	}

	rb_node->rb_left = pivot->rb_right;
	if (pivot->rb_right != NULL)
		pivot->rb_right->rb_parent = rb_node;

	rb_node->rb_parent = pivot;
	pivot->rb_right = rb_node;
}

int rb_insert(struct rb_node *rb_root, struct rb_node *rb_node, int(*rb_node_cmp)(struct rb_node*, struct rb_node*)){
	struct rb_node *rb_proot;
	if(!__rb_search(rb_root, rb_node, rb_node_cmp))
		return -1; //node is already in rb_tree;
	rb_proot = rb_search(rb_root, rb_node, rb_node_cmp);
	if(rb_node_cmp(rb_proot, rb_node) > 0)
		rb_proot->rb_right = rb_node;
	else
		rb_proot->rb_left = rb_node;
	rb_node->rb_parent = rb_proot;
	rb_node->rb_color = RB_RED;
	rb_rebalance(rb_node);
	return 0;
}

void rb_swap(struct rb_node *rb_node_1, struct rb_node *rb_node_2){
	// Swaps right, left, parent, color values; Also swaps two parents references;
	struct rb_node *rb_node_tmp, **rb_node_tmp2, tmp;
	char rb_color_tmp;
	//
	// rb_node_tmp = rb_node_1->rb_right;
	// rb_node_1->rb_right = rb_node_2->rb_right;
	// rb_node_2->rb_right = rb_node_tmp;
	//
	// rb_node_tmp = rb_node_1->rb_left;
	// rb_node_1->rb_left = rb_node_2->rb_left;
	// rb_node_2->rb_left = rb_node_tmp;
	//
	// rb_node_tmp = rb_node_1->rb_parent;
	// rb_node_1->rb_parent = rb_node_2->rb_parent;
	// rb_node_2->rb_parent = rb_node_tmp;

	tmp = *rb_node_1;
	*rb_node_1 = *rb_node_2;
	*rb_node_2 = *rb_node_1;

	rb_node_tmp2 = rb_node_1->rb_parent->rb_left == rb_node_2? &rb_node_1->rb_parent->rb_left : &rb_node_1->rb_parent->rb_right ;
	rb_node_tmp = *rb_node_tmp2;
	*rb_node_tmp2 = rb_node_2->rb_parent->rb_left == rb_node_1? rb_node_2->rb_parent->rb_left : rb_node_2->rb_parent->rb_right;
	rb_node_tmp2 = rb_node_2->rb_parent->rb_left == rb_node_1? &rb_node_2->rb_parent->rb_left : &rb_node_2->rb_parent->rb_right ;
	*rb_node_tmp2 = rb_node_tmp;

	rb_color_tmp = rb_node_1->rb_color;
	rb_node_1->rb_color = rb_node_2->rb_color;
	rb_node_2->rb_color = rb_color_tmp;
}

void rb_remove(struct rb_node *rb_root, struct rb_node *rb_node, int(*rb_node_cmp)(struct rb_node*, struct rb_node*)){
	struct rb_node *rb_proot, *tmp;
	rb_proot = rb_search(rb_root, rb_node, rb_node_cmp);
	if(!rb_proot)
		return; // node is not in rb_tree;

	if(rb_proot->rb_right){
			tmp = rb_get_min(rb_proot->rb_right);
			rb_swap(rb_proot, tmp);
	} else if(rb_proot->rb_left){
			tmp = rb_get_max(rb_proot->rb_left);
			rb_swap(rb_proot, tmp);
	}

	if(rb_proot->rb_color == RB_RED){
		if(rb_proot->rb_right){
			*rb_proot = *(rb_proot->rb_right);
		} else if(rb_proot->rb_left){
			*rb_proot = *(rb_proot->rb_left);
		} else {
			if(rb_proot->rb_parent->rb_left == rb_root)
				rb_proot->rb_parent->rb_left = NULL;
			else
				rb_proot->rb_parent->rb_right = NULL;
		}
	} else {
		// Black node to remove
		if(rb_proot->rb_right){
				*rb_proot = *(rb_proot->rb_right);
				rb_proot->rb_color = RB_BLACK;
		} else if(rb_proot->rb_left){
				*rb_proot = *(rb_proot->rb_left);
				rb_proot->rb_color = RB_BLACK;
		} else {
			if(rb_proot)
				delete_case1(rb_proot);
		}
	}
}
