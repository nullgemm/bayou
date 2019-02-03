#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "def.h"
#include "short.h"

// branches manipulation

void bayou_branch_root(struct bayou* bayou)
{
	bayou->selected_branch = bayou->root;
}

bool bayou_branch_parent(struct bayou* bayou)
{
	if (bayou->selected_branch->parent!= NULL)
	{
		bayou->selected_branch = bayou->selected_branch->parent;

		return true;
	}

	return false;
}

bool bayou_branch_sibling(struct bayou* bayou)
{
	if (bayou->selected_branch->siblings != NULL)
	{
		bayou->selected_branch = bayou->selected_branch->siblings;

		return true;
	}

	return false;
}

bool bayou_branch_child(struct bayou* bayou)
{
	if (bayou->selected_branch->children!= NULL)
	{
		bayou->selected_branch = bayou->selected_branch->children;

		return true;
	}

	return false;
}

uint16_t bayou_count_children(struct bayou* bayou)
{
	return bayou->selected_branch->children_len;
}

// elements manipulation

bool bayou_element_set(struct bayou* bayou, uint16_t id)
{
	if (id < bayou->selected_branch->elements_len)
	{
		bayou->selected_element = id;

		return true;
	}

	return false;
}

uint16_t bayou_element_get(struct bayou* bayou)
{
	return bayou->selected_element;
}

bool bayou_element_prev(struct bayou* bayou)
{
	if (bayou->selected_element > 0)
	{
		--(bayou->selected_element);

		return true;
	}

	return false;
}

bool bayou_element_next(struct bayou* bayou)
{
	if ((bayou->selected_element + 1) < bayou->selected_branch->elements_len)
	{
		++(bayou->selected_element);

		return true;
	}

	return false;
}

uint16_t bayou_count_elements(struct bayou* bayou)
{
	return bayou->selected_branch->elements_len;
}

bool bayou_should_defrag(struct bayou* bayou)
{
	return bayou->pool_holes.cur > 0;
}
