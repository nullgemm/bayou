#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "bayou.h"
#include <stdio.h>

#ifndef BAYOU_STATIC
#include <stdlib.h>
#endif

// private

union p32 {void* vp32; uint32_t* up32;};

static bool alloc(void** buf, uint16_t len, uint16_t pointed_len)
{
	if (*buf == NULL)
	{
#ifndef BAYOU_STATIC
		*buf = malloc(len * pointed_len);

		if (*buf == NULL)
		{
			return false;
		}
#else
		return false;
#endif
	}

	return true;
}

static bool count_branches(
	struct bayou* bayou,
	struct bayou_branch* selected_branch,
	void* ptr)
{
	union p32 count;

	count.vp32 = ptr;
	++(*count.up32);

	return true;
}

static bool link_branch(struct bayou* bayou, struct bayou_branch* new_branch)
{
	struct bayou_branch* parent = bayou->selected_branch;

	if (bayou->selected_element < (bayou->selected_branch->elements_len - 1))
	{
		// no more space
		if (bayou->pool_branches.cur == bayou->pool_branches.len)
		{
			return false;
		}

		// split the parent
		struct bayou_branch* split_branch;
		split_branch = ((struct bayou_branch*) bayou->pool_branches.buf);
		split_branch += bayou->pool_branches.cur;

		split_branch->parent = parent;
		split_branch->siblings = new_branch;
		split_branch->children = parent->children;
		split_branch->children_len = parent->children_len;

		split_branch->elements = parent->elements;
		split_branch->elements += (bayou->selected_element + 1) * bayou->sizeof_element;

		split_branch->elements_len = parent->elements_len - bayou->selected_element - 1;
		++(bayou->pool_branches.cur);

		// update the original parent
		parent->children = split_branch;
		parent->children_len = 2;
		parent->elements_len = bayou->selected_element + 1;

		// update the original parent's children
		struct bayou_branch* process = split_branch->children;

		while (process != NULL)
		{
			process->parent = split_branch;
			process = process->siblings;
		}

		// update the new branch
		new_branch->parent = parent;
	}
	else
	{
		// update the new branch
		new_branch->parent = parent;
		new_branch->siblings = parent->children;

		// update the parent
		parent->children = new_branch;
		++(parent->children_len);
	}

	return true;
}

static bool add_branch(struct bayou* bayou)
{
	if (bayou->pool_branches.cur == bayou->pool_branches.len)
	{
		return false;
	}

	// create the new branch
	struct bayou_branch* new_branch;
	new_branch = ((struct bayou_branch*) bayou->pool_branches.buf);
	new_branch += bayou->pool_branches.cur;

	new_branch->elements = ((uint8_t*) bayou->pool_elements.buf);
	new_branch->elements += bayou->pool_elements.cur * bayou->sizeof_element;

	new_branch->parent = NULL;
	new_branch->siblings = NULL;
	new_branch->children = NULL;
	new_branch->children_len = 0;
	new_branch->elements_len = 0;
	++(bayou->pool_branches.cur);

	// adds the branch if possible
	if (link_branch(bayou, new_branch) == false)
	{
		return false;
	}

	// register the new branch
	bayou->selected_branch = new_branch;
	bayou->selected_element = 0;

	return  true;
}

static bool rm_branch(
	struct bayou* bayou,
	struct bayou_branch* selected_branch,
	void* ptr)
{
	struct bayou_branch* parent = selected_branch->parent;
	bool run = true;

	if (selected_branch == bayou->selected_branch)
	{
		struct bayou_branch* process = parent->children;

		if (process == selected_branch)
		{
			parent->children = selected_branch->siblings;
		}
		else
		{
			while (process->siblings != selected_branch)
			{
				process = process->siblings;
			}

			process->siblings = selected_branch->siblings;
		}

		--(parent->children_len);

		run = false;
	}

	// register a hole
	if (bayou->pool_holes.cur == bayou->pool_holes.len)
	{
		run = false; // FIXME remove, defrag or realloc
	}

	struct bayou_hole* new_hole;
	new_hole = (struct bayou_hole*) bayou->pool_holes.buf;
	new_hole += bayou->pool_holes.cur;

	uint16_t elements_offset = selected_branch->elements - ((uint8_t*) bayou->pool_elements.buf);
	new_hole->id = elements_offset / bayou->sizeof_element;
	new_hole->len = selected_branch->elements_len;
	++(bayou->pool_holes.cur);

	// actually delete the branch

	struct bayou_branch* last_branch;
	last_branch = (struct bayou_branch*) bayou->pool_branches.buf;
	last_branch += bayou->pool_branches.cur - 1;

	if (selected_branch != last_branch)
	{
		struct bayou_branch* process = parent->children;

		memcpy(selected_branch, last_branch, sizeof (struct bayou_branch));

		if (process == last_branch)
		{
			parent->children = selected_branch;
		}
		else
		{
			while (process->siblings != selected_branch)
			{
				process = process->siblings;
			}

			process->siblings = selected_branch;
		}
	}

	--(bayou->pool_branches.cur);

	return run;
}

// public

void bayou_free(struct bayou* bayou)
{
	if (bayou->pool_elements.dyn == true)
	{
		free(bayou->pool_elements.buf);
	}

	if (bayou->pool_branches.dyn == true)
	{
		free(bayou->pool_branches.buf);
	}
}

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
	uint16_t alloc_maximum_len_holes)
{
	// create buffers
	if (!alloc(&buf_branches, len_branches, sizeof (struct bayou_branch)))
	{
		return false;
	}

	if (!alloc(&buf_elements, len_elements, sizeof_element))
	{
		return false;
	}

	if (!alloc(&buf_holes, len_holes, sizeof(struct bayou_hole)))
	{
		return false;
	}

	// create pools
	bayou->pool_branches.dyn = (buf_branches == NULL);
	bayou->pool_branches.buf = buf_branches;
	bayou->pool_branches.cur = 0;
	bayou->pool_branches.len = len_branches;
	bayou->pool_branches.step_len = alloc_step_len_branches;
	bayou->pool_branches.maximum_len = alloc_maximum_len_branches;

	bayou->pool_elements.dyn = (buf_elements == NULL);
	bayou->pool_elements.buf = buf_elements;
	bayou->pool_elements.cur = 0;
	bayou->pool_elements.len = len_elements;
	bayou->pool_elements.step_len = alloc_step_len_elements;
	bayou->pool_elements.maximum_len = alloc_maximum_len_elements;
	bayou->sizeof_element = sizeof_element;

	bayou->pool_holes.dyn = (buf_holes == NULL);
	bayou->pool_holes.buf = buf_holes;
	bayou->pool_holes.cur = 0;
	bayou->pool_holes.len = len_holes;
	bayou->pool_holes.step_len = alloc_step_len_holes;
	bayou->pool_holes.maximum_len = alloc_maximum_len_holes;

	// create root
	bayou->selected_branch = (struct bayou_branch*) bayou->pool_branches.buf;
	bayou->selected_branch->parent = NULL;
	bayou->selected_branch->siblings = NULL;
	bayou->selected_branch->children = NULL;
	bayou->selected_branch->children_len = 0;
	bayou->selected_branch->elements_len = 0;

	// register root
	bayou->root = bayou->selected_branch;
	bayou->pool_branches.cur = 1;
	bayou->selected_element = 0;

	return true;
}

bool bayou_branch_exec(struct bayou* bayou,
	bool (*function)(struct bayou* bayou,struct bayou_branch* branch,void* ptr),
	void* ptr)
{
	struct bayou_branch* selected_branch = bayou->selected_branch;
	struct bayou_branch* parent_branch = selected_branch->parent;
	struct bayou_branch* tmp_siblings;
	struct bayou_branch* tmp_parent;

	while (selected_branch != parent_branch)
	{
		if (selected_branch->children != NULL)
		{
			selected_branch = selected_branch->children;
			continue;
		}

		do
		{
			tmp_siblings = selected_branch->siblings;
			tmp_parent = selected_branch->parent;

			if (!function(bayou, selected_branch, ptr))
			{
				return false;
			}

			if (tmp_siblings != NULL)
			{
				selected_branch = tmp_siblings;
				break;
			}

			selected_branch = tmp_parent;
		}
		while (selected_branch != parent_branch);
	}

	return true;
}

void bayou_defrag(struct bayou* bayou)
{
	// merge
	uint16_t i;
	uint16_t end;

	struct bayou_hole* hole;
	hole = (struct bayou_hole*) bayou->pool_holes.buf;

	struct bayou_hole* hole_next;
	hole_next = (struct bayou_hole*) bayou->pool_holes.buf + 1;

	i = 0;
	end = bayou->pool_holes.cur;

	while (i < (end - 1))
	{
		if ((hole->id + hole->len) == hole_next->id)
		{
			hole->len += hole_next->len;
			--(bayou->pool_holes.cur);

			memcpy(hole_next, hole_next + 1, (bayou->pool_holes.cur - i) * (sizeof (struct bayou_hole)));

			i = 0;
			--end;
			hole_next = (struct bayou_hole*) bayou->pool_holes.buf - 1;
		}
		else
		{
			++i;
			++hole_next;
		}
	}

	// defrag
	struct bayou_branch* process = (struct bayou_branch*) bayou->pool_branches.buf;

	uint8_t* elements = ((uint8_t*) bayou->pool_elements.buf);
	elements += hole->id * bayou->sizeof_element;

	uint8_t* elements_next = elements;
	elements_next += hole->len * bayou->sizeof_element;

	i = 0;
	end = bayou->pool_branches.cur;

	while (i < end)
	{
		if (process[i].elements == elements_next)
		{
			break;
		}

		++i;
	}

	if (i != end)
	{
		memcpy(elements, process[i].elements, process[i].elements_len * bayou->sizeof_element);
		hole->id += process[i].elements_len;
		process[i].elements = elements;
	}

	// remove hole when the bubble popped
	if (hole->id + hole->len == bayou->pool_elements.cur)
	{
		--(bayou->pool_holes.cur);
		bayou->pool_elements.cur -= hole->len;
		memcpy(hole, hole + 1, bayou->pool_holes.cur * (sizeof (struct bayou_hole)));
	}
}

void* bayou_add_element(struct bayou* bayou)
{
	struct bayou_pool* pool_elements = &(bayou->pool_elements);

	if (pool_elements->cur == pool_elements->len)
	{
		// no more space
		return NULL;
	}

	struct bayou_branch* selected_branch = bayou->selected_branch;

	uint8_t* elements_cur = ((uint8_t*) pool_elements->buf);
	elements_cur += pool_elements->cur * bayou->sizeof_element;

	// add at the end of the buffer if there is no element in this branch yet
	if (selected_branch->elements_len == 0)
	{
		selected_branch->elements = elements_cur;
	}

	uint16_t elements_len = selected_branch->elements_len;
	uint8_t* elements_next = selected_branch->elements;
	elements_next += elements_len * bayou->sizeof_element;

	// jump over existing data when needed or when we want to split the branch
	if ((elements_cur != elements_next) || (bayou->selected_element < (elements_len - 1)))
	{
		if (add_branch(bayou) == false)
		{
			return NULL;
		}

		selected_branch = bayou->selected_branch;
	}

	// save before incrementing
	bayou->selected_element = selected_branch->elements_len;
	++(selected_branch->elements_len);
	++(pool_elements->cur);

	// return old pointer
	return elements_cur;
}

void bayou_rm_branch(struct bayou* bayou)
{
	struct bayou_branch* parent = bayou->selected_branch->parent;

	bayou_branch_exec(bayou, rm_branch, NULL);
	bayou->selected_branch = parent;
}

void bayou_rm_element(struct bayou* bayou)
{
	uint16_t selected_element = bayou->selected_element;
	uint16_t branch_elements_len = bayou->selected_branch->elements_len;
	uint8_t* branch_elements = bayou->selected_branch->elements;

	if (branch_elements_len == 0)
	{
		return;
	}

	if ((selected_element + 1) < branch_elements_len)
	{
		memcpy(branch_elements + selected_element * bayou->sizeof_element,
			branch_elements + (selected_element + 1) * bayou->sizeof_element,
			(branch_elements_len - selected_element - 1) * bayou->sizeof_element);
	}

	uint8_t* pool_elements_cur_offset = (uint8_t*) bayou->pool_elements.buf;
	pool_elements_cur_offset += bayou->pool_elements.cur * bayou->sizeof_element;

	if (((branch_elements + branch_elements_len) < pool_elements_cur_offset)
	&& (bayou->pool_holes.cur < bayou->pool_holes.len))
	{
		struct bayou_hole* new_hole;
		new_hole = (struct bayou_hole*) bayou->pool_holes.buf;
		new_hole += bayou->pool_holes.cur;

		uint16_t elements_offset = branch_elements - ((uint8_t*) bayou->pool_elements.buf);
		new_hole->id = (elements_offset / bayou->sizeof_element) + branch_elements_len - 1;
		new_hole->len = 1;

		++(bayou->pool_holes.cur);
	}

	--(bayou->selected_branch->elements_len);
}

uint32_t bayou_count_branches(struct bayou* bayou)
{
	uint32_t count = 0;

	bayou_branch_exec(bayou, count_branches, &count);

	return count;
}
