#ifndef JOS_INC_TYPES_H
#define JOS_INC_TYPES_H

#ifndef NULL
#define NULL ((void*) 0)
#endif

/* Represents true-or-false values */
typedef _Bool bool;
enum { false, true };

/* Explicitly-sized versions of integer types */
typedef __signed char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int64_t;
typedef unsigned long long uint64_t;

/*
 * Pointers and addresses are 32 bits long.
 * We use pointer types to represent virtual addresses,
 * uintptr_t to represent the numerical values of virtual addresses,
 * and physaddr_t to represent physical addresses.
 */
typedef int32_t intptr_t;
typedef uint32_t uintptr_t;
typedef uint32_t physaddr_t;

/* Page numbers are 32 bits long. */
typedef uint32_t ppn_t;

/* size_t is used for memory object sizes. */
typedef uint32_t size_t;
/* ssize_t is a signed version of size_t, used in case an error might be
 * returned. */
typedef int32_t ssize_t;

/* off_t is used for file offsets and lengths. */
typedef int32_t off_t;

/* Efficient min and max operations */
#define MIN(_a, _b)                                                            \
({                                                                             \
    typeof(_a) __a = (_a);                                                     \
    typeof(_b) __b = (_b);                                                     \
    __a <= __b ? __a : __b;                                                    \
})
#define MAX(_a, _b)                                                            \
({                                                                             \
    typeof(_a) __a = (_a);                                                     \
    typeof(_b) __b = (_b);                                                     \
    __a >= __b ? __a : __b;                                                    \
})

/* Rounding operations (efficient when n is a power of 2)
 * Round down to the nearest multiple of n */
#define ROUNDDOWN(a, n)                                                        \
({                                                                             \
    uint32_t __a = (uint32_t) (a);                                             \
    (typeof(a)) (__a - __a % (n));                                             \
})
/* Round up to the nearest multiple of n */
#define ROUNDUP(a, n)                                                          \
({                                                                             \
    uint32_t __n = (uint32_t) (n);                                             \
    (typeof(a)) (ROUNDDOWN((uint32_t) (a) + __n - 1, __n));                    \
})

/* Return the offset of 'member' relative to the beginning of a struct type */
#define offsetof(type, member)  ((size_t) (&((type*)0)->member))


#define __always_inline         inline __attribute__((always_inline))

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})


#define remove_entry_from_list(type, item, head, next_link) do{ \
	type **head_list = &head;															\
	type *entry = item;																		\
	while(*head_list){ 																		\
		if(entry == *head_list){														\
			*head_list = ((type *)entry)->next_link;					\
			((type *)entry)->next_link = NULL;								\
			break; 																						\
		} 																									\
		head_list = &((*((type **)head_list))->next_link);	\
						}																						\
					}while(0)
#endif /* !JOS_INC_TYPES_H */
