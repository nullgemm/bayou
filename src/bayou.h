#ifndef H_BAYOU
#define H_BAYOU

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "def.h"
#include "short.h"


// can be skipped if no auto-malloc was done
void bayou_free(struct bayou* bayou);

// auto-mallocs the needed buffers if given NULL
// pointers and BAYOU_STATIC is defined
bool bayou_init(
	struct bayou* bayou,
	
	void* buf_branches,
	uint16_t len_branches,
	uint16_t alloc_step_len_branches,
	uint16_t alloc_maximum_len_branches,

	void* buf_elements,
	uint16_t len_elements,
	uint16_t alloc_step_len_elements,
	uint16_t alloc_maximum_len_elements,
	uint16_t sizeof_element,

	void* buf_holes,
	uint16_t len_holes,
	uint16_t alloc_step_len_holes,
	uint16_t alloc_maximum_len_holes);

// depth-first traversal, executes the function when going back up
bool bayou_branch_exec(struct bayou* bayou,
	bool (*function)(struct bayou* bayou,struct bayou_branch* branch,void* ptr),
	void* ptr);

// iterative defragmentation, can be called in the main loop
void bayou_defrag(struct bayou* bayou);

// inserts after the currently pointed element
void* bayou_add_element(struct bayou* bayou);

// removes the currently pointed branch and all the ones below
void bayou_rm_branch(struct bayou* bayou);

// removes the currently pointed element
void bayou_rm_element(struct bayou* bayou);

// counts the branches using the exec function
uint32_t bayou_count_branches(struct bayou* bayou);

#endif
