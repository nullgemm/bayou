#ifndef C_TESTS
#define C_TESTS

#include <stdio.h>
#include <signal.h>
#include "../src/bayou.h"
#include "../sub/testoasterror/src/testoasterror.h"

union vub16 {uint16_t ub16; void* ptr;};

// I know. This is for testing only, so STFU
static void prev_element(struct bayou* bayou, uint16_t count)
{
	for (uint16_t i = 0; i < count; ++i)
	{
		bayou_element_prev(bayou);
	}
}

static void init_branch(
	struct bayou* bayou,
	uint16_t len,
	uint16_t start)
{
	union vub16 ptr;
	ptr.ptr = 0; // important
	ptr.ub16 += start;

	void** el;

	for (uint8_t i = 0; i < len; ++i)
	{
		el = bayou_add_element(bayou);

		if (el != NULL)
		{
			*el = ptr.ptr;
		}

		++(ptr.ub16);
	}
}

static void init_tree(
	struct bayou* bayou,
	struct bayou_hole* holes,
	struct bayou_branch* branches,
	void** elements,
	uint16_t holes_len,
	uint16_t branches_len,
	uint16_t elements_len)
{
	bayou_init(
		bayou,

		branches,
		branches_len,
		sizeof (struct bayou_branch),
		branches_len,

		elements,
		elements_len,
		sizeof (void*),
		elements_len,

		holes,
		holes_len,
		sizeof (struct bayou_hole),
		holes_len);

	uint16_t start = 0;
	const uint16_t len = 10;
	const uint16_t count = 5;

	init_branch(bayou, len, start);

	start += len;
	prev_element(bayou, count);
	init_branch(bayou, len, start);

	start += len;
	bayou_branch_parent(bayou);
	bayou_branch_child(bayou);
	bayou_element_set(bayou, count - 1);
	init_branch(bayou, len, start);

	start += len;
	prev_element(bayou, count);
	init_branch(bayou, len, start);

	start += len;
	bayou_branch_parent(bayou);
	bayou_branch_child(bayou);
	bayou_element_set(bayou, count / 2);
	init_branch(bayou, len, start);
}

static void test_elements_range(
	struct testoasterror* test,
	struct bayou* bayou,
	uint16_t start,
	uint16_t end,
	uint16_t num_start)
{
	union vub16 ptr;
	ptr.ptr = 0; // important
	ptr.ub16 += num_start;

	for (uint16_t i = 0; i < end - start + 1; ++i)
	{
		testoasterror(test, bayou->selected_branch->elements[i] == ptr.ptr);
		++(ptr.ub16);
	}
}

void test_init(struct testoasterror* test)
{
	struct bayou bayou;
	struct bayou_branch branches[10];
	void* elements[100];
	struct bayou_hole holes[10];

	bayou_init(
		&bayou,

		branches,
		10,
		sizeof (struct bayou_branch),
		10,

		elements,
		100,
		sizeof (void*),
		100,

		holes,
		10,
		sizeof (struct bayou_hole),
		10);

	testoasterror(test, bayou.pool_elements.dyn == false);
	testoasterror(test, bayou.pool_elements.buf == elements);
	testoasterror(test, bayou.pool_elements.cur == 0);
	testoasterror(test, bayou.pool_elements.len == 100);
	testoasterror(test, bayou.pool_elements.pointed_len == (sizeof (void*)));
	testoasterror(test, bayou.pool_elements.maximum_len == 100);

	testoasterror(test, bayou.pool_branches.dyn == false);
	testoasterror(test, bayou.pool_branches.buf == branches);
	testoasterror(test, bayou.pool_branches.cur == 1);
	testoasterror(test, bayou.pool_branches.len == 10);
	testoasterror(test, bayou.pool_branches.pointed_len == (sizeof (struct bayou_branch)));
	testoasterror(test, bayou.pool_branches.maximum_len == 10);

	testoasterror(test, bayou.pool_holes.dyn == false);
	testoasterror(test, bayou.pool_holes.buf == holes);
	testoasterror(test, bayou.pool_holes.cur == 0);
	testoasterror(test, bayou.pool_holes.len == 10);
	testoasterror(test, bayou.pool_holes.pointed_len == (sizeof (struct bayou_hole)));
	testoasterror(test, bayou.pool_holes.maximum_len == 10);

	testoasterror(test, bayou.root == bayou.pool_branches.buf);
	testoasterror(test, bayou.selected_branch == bayou.pool_branches.buf);
	testoasterror(test, bayou.selected_element == 0);

	testoasterror(test, bayou.root->parent == NULL);
	testoasterror(test, bayou.root->siblings == NULL);
	testoasterror(test, bayou.root->children == NULL);
	testoasterror(test, bayou.root->elements_len == 0);
	testoasterror(test, bayou.root->children_len == 0);
}

void test_add(struct testoasterror* test)
{
	struct bayou bayou;
	struct bayou_branch branches[10];
	void* elements[100];
	struct bayou_hole holes[10];

	bayou_init(
		&bayou,

		branches,
		10,
		sizeof (struct bayou_branch),
		10,

		elements,
		100,
		sizeof (void*),
		100,

		holes,
		10,
		sizeof (struct bayou_hole),
		10);

	init_branch(&bayou, 10, 0);

	testoasterror(test, bayou_count_elements(&bayou) == 10);
	testoasterror(test, bayou_element_get(&bayou) == 9);

	for (uint8_t i = 0; i < 10; ++i)
	{
		bayou_element_set(&bayou, i);
		testoasterror(test, bayou_element_get(&bayou) == i);
	}
}

void test_split_simple(struct testoasterror* test)
{
	struct bayou bayou;
	struct bayou_branch branches[10];
	void* elements[100];
	struct bayou_hole holes[10];

	bayou_init(
		&bayou,

		branches,
		10,
		sizeof (struct bayou_branch),
		10,

		elements,
		100,
		sizeof (void*),
		100,

		holes,
		10,
		sizeof (struct bayou_hole),
		10);

	init_branch(&bayou, 10, 0);
	prev_element(&bayou, 6);
	init_branch(&bayou, 20, 0);

	bayou_branch_root(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 4);
	bayou_branch_child(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 6);
	bayou_branch_sibling(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 20);
}

void test_split_complex(struct testoasterror* test)
{
	struct bayou bayou;
	struct bayou_hole holes[10];
	struct bayou_branch branches[10];
	void* elements[100];

	init_tree(&bayou, holes, branches, elements, 10, 10, 100);

	bayou_branch_root(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 5);
	test_elements_range(test, &bayou, 0, 4, 0);

	bayou_branch_child(&bayou);
	bayou_branch_sibling(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 10);
	test_elements_range(test, &bayou, 10, 19, 10);

	bayou_branch_parent(&bayou);
	bayou_branch_child(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 5);
	test_elements_range(test, &bayou, 5, 9, 5);

	bayou_branch_child(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 5);
	test_elements_range(test, &bayou, 20, 24, 20);

	bayou_branch_child(&bayou);
	bayou_branch_sibling(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 10);
	test_elements_range(test, &bayou, 30, 39, 30);

	bayou_branch_parent(&bayou);
	bayou_branch_child(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 3);
	test_elements_range(test, &bayou, 25, 27, 25);

	bayou_branch_child(&bayou);
	bayou_branch_sibling(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 10);
	test_elements_range(test, &bayou, 40, 49, 40);

	bayou_branch_parent(&bayou);
	bayou_branch_child(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 2);
	test_elements_range(test, &bayou, 28, 29, 28);
}

void test_count_branches(struct testoasterror* test)
{
	struct bayou bayou;
	struct bayou_hole holes[10];
	struct bayou_branch branches[10];
	void* elements[100];

	init_tree(&bayou, holes, branches, elements, 10, 10, 100);

	bayou_branch_root(&bayou);

	testoasterror(test, bayou_count_branches(&bayou) == 8);
}

void test_frag(struct testoasterror* test)
{
	struct bayou bayou;
	struct bayou_hole holes[10];
	struct bayou_branch branches[10];
	void* elements[100];

	init_tree(&bayou, holes, branches, elements, 10, 10, 100);

	bayou_branch_root(&bayou);
	bayou_branch_child(&bayou);
	bayou_element_set(&bayou, 2);
	bayou_rm_element(&bayou);

	union vub16 ptr;
	ptr.ptr = 0; // important
	ptr.ub16 += 8;

	testoasterror(test, bayou_count_elements(&bayou) == 4);
	testoasterror(test, bayou.selected_branch->elements[2] == ptr.ptr);
	testoasterror(test, bayou_should_defrag(&bayou));
}

void test_defrag(struct testoasterror* test)
{
	struct bayou bayou;
	struct bayou_hole holes[10];
	struct bayou_branch branches[10];
	void* elements[100];

	init_tree(&bayou, holes, branches, elements, 10, 10, 100);

	bayou_branch_root(&bayou);
	bayou_branch_child(&bayou);
	bayou_element_set(&bayou, 0);
	bayou_rm_element(&bayou);

	testoasterror(test, bayou_should_defrag(&bayou));

	for (uint8_t i = 0; i < 6; ++i)
	{
		bayou_defrag(&bayou);
	}

	testoasterror(test, !bayou_should_defrag(&bayou));

	bayou_branch_root(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 5);
	test_elements_range(test, &bayou, 0, 4, 0);

	bayou_branch_child(&bayou);
	bayou_branch_sibling(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 10);
	test_elements_range(test, &bayou, 9, 18, 10);

	bayou_branch_parent(&bayou);
	bayou_branch_child(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 4);
	test_elements_range(test, &bayou, 5, 8, 6);

	bayou_branch_child(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 5);
	test_elements_range(test, &bayou, 19, 23, 20);

	bayou_branch_child(&bayou);
	bayou_branch_sibling(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 10);
	test_elements_range(test, &bayou, 29, 38, 30);

	bayou_branch_parent(&bayou);
	bayou_branch_child(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 3);
	test_elements_range(test, &bayou, 24, 26, 25);

	bayou_branch_child(&bayou);
	bayou_branch_sibling(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 10);
	test_elements_range(test, &bayou, 39, 48, 40);

	bayou_branch_parent(&bayou);
	bayou_branch_child(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 2);
	test_elements_range(test, &bayou, 27, 28, 28);
}

void test_defrag_multi(struct testoasterror* test)
{
	struct bayou bayou;
	struct bayou_hole holes[10];
	struct bayou_branch branches[10];
	void* elements[100];

	init_tree(&bayou, holes, branches, elements, 10, 10, 100);

	bayou_branch_root(&bayou);
	bayou_branch_child(&bayou);
	bayou_element_set(&bayou, 0);
	bayou_rm_element(&bayou);
	bayou_branch_child(&bayou);
	bayou_branch_child(&bayou);
	bayou_branch_sibling(&bayou);
	bayou_element_set(&bayou, 9);
	bayou_rm_element(&bayou);

	testoasterror(test, bayou_should_defrag(&bayou));

	for (uint8_t i = 0; i < 7; ++i)
	{
		bayou_defrag(&bayou);
	}

	testoasterror(test, !bayou_should_defrag(&bayou));

	bayou_branch_root(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 5);
	test_elements_range(test, &bayou, 0, 4, 0);

	bayou_branch_child(&bayou);
	bayou_branch_sibling(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 10);
	test_elements_range(test, &bayou, 9, 18, 10);

	bayou_branch_parent(&bayou);
	bayou_branch_child(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 4);
	test_elements_range(test, &bayou, 5, 8, 6);

	bayou_branch_child(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 5);
	test_elements_range(test, &bayou, 19, 23, 20);

	bayou_branch_child(&bayou);
	bayou_branch_sibling(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 9);
	test_elements_range(test, &bayou, 29, 37, 30);

	bayou_branch_parent(&bayou);
	bayou_branch_child(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 3);
	test_elements_range(test, &bayou, 24, 26, 25);

	bayou_branch_child(&bayou);
	bayou_branch_sibling(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 10);
	test_elements_range(test, &bayou, 38, 47, 40);

	bayou_branch_parent(&bayou);
	bayou_branch_child(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 2);
	test_elements_range(test, &bayou, 27, 28, 28);
}

void test_rm_branches(struct testoasterror* test)
{
	struct bayou bayou;
	struct bayou_hole holes[10];
	struct bayou_branch branches[10];
	void* elements[100];

	init_tree(&bayou, holes, branches, elements, 10, 10, 100);

	bayou_branch_root(&bayou);
	bayou_branch_child(&bayou);
	bayou_branch_child(&bayou);
	bayou_branch_child(&bayou);
	bayou_rm_branch(&bayou);

	bayou_branch_root(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 5);
	testoasterror(test, bayou_count_children(&bayou) == 2);

	bayou_branch_child(&bayou);
	bayou_branch_sibling(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 10);
	testoasterror(test, bayou_count_children(&bayou) == 0);

	bayou_branch_parent(&bayou);
	bayou_branch_child(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 5);
	testoasterror(test, bayou_count_children(&bayou) == 1);

	bayou_branch_child(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 5);
	testoasterror(test, bayou_count_children(&bayou) == 1);

	bayou_branch_child(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 10);
	testoasterror(test, bayou_count_children(&bayou) == 0);
}

void test_defrag_branches(struct testoasterror* test)
{
	struct bayou bayou;
	struct bayou_hole holes[10];
	struct bayou_branch branches[10];
	void* elements[100];

	init_tree(&bayou, holes, branches, elements, 10, 10, 100);

	bayou_branch_root(&bayou);
	bayou_branch_child(&bayou);
	bayou_branch_child(&bayou);
	bayou_branch_child(&bayou);
	bayou_rm_branch(&bayou);

	testoasterror(test, bayou_should_defrag(&bayou));

	for (uint8_t i = 0; i < 3; ++i)
	{
		bayou_defrag(&bayou);
	}

	testoasterror(test, !bayou_should_defrag(&bayou));

	bayou_branch_root(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 5);
	test_elements_range(test, &bayou, 0, 4, 0);

	bayou_branch_child(&bayou);
	bayou_branch_sibling(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 10);
	test_elements_range(test, &bayou, 10, 19, 10);

	bayou_branch_parent(&bayou);
	bayou_branch_child(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 5);
	test_elements_range(test, &bayou, 5, 9, 5);

	bayou_branch_child(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 5);
	test_elements_range(test, &bayou, 20, 24, 20);

	bayou_branch_child(&bayou);
	bayou_branch_sibling(&bayou);
	testoasterror(test, bayou_count_elements(&bayou) == 10);
	test_elements_range(test, &bayou, 25, 34, 30);
}

#endif
