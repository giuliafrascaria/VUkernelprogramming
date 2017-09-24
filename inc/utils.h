#ifndef UTILS_H
#define UTILS_H

#include <inc/types.h>

#define RB_BLACK 0
#define RB_RED 1

#define container_of(type, field, ptr)  ((void*)ptr - &((type*)(0))->field)

struct rb_node {
		struct rb_node *rb_right;
		struct rb_node *rb_left;
		struct rb_node *rb_parent;
		char rb_color;
};

struct rb_node* rb_search(struct rb_node* rb_root, struct rb_node* rb_node, int(*rb_node_cmp)(struct rb_node*, struct rb_node*));
int rb_insert(struct rb_node*, struct rb_node*, int(*rb_node_cmp)(struct rb_node*, struct rb_node*));
void rb_right_rotate(struct rb_node*);
void rb_left_rotate(struct rb_node*);
void rb_swap(struct rb_node*, struct rb_node*);
void rb_remove(struct rb_node*, struct rb_node*, int(*rb_node_cmp)(struct rb_node*, struct rb_node*));
void rb_rebalance(struct rb_node*);
struct rb_node* rb_get_max(struct rb_node*);
struct rb_node* rb_get_min(struct rb_node*);



struct list_head {
	struct list_head *next, *prev;
};


#endif
