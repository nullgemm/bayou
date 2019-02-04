#ifndef H_BAYOU
#define H_BAYOU

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "def.h"
#include "short.h"

void bayou_free(struct bayou* bayou);

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


bool bayou_branch_exec(struct bayou* bayou,
	bool (*function)(struct bayou* bayou,struct bayou_branch* branch,void* ptr),
	void* ptr);

void bayou_defrag(struct bayou* bayou);

void* bayou_add_element(struct bayou* bayou);

void bayou_rm_branch(struct bayou* bayou);

void bayou_rm_element(struct bayou* bayou);

uint32_t bayou_count_branches(struct bayou* bayou);

#endif
