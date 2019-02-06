#ifndef H_BAYOU_DEF
#define H_BAYOU_DEF

#include <stdint.h>
#include <stdbool.h>

// memory info for defragmentation
struct bayou_hole
{
	uint16_t id;
	uint16_t len;
};

// node info for browsing and manipulation
struct bayou_branch
{
	struct bayou_branch* parent;
	struct bayou_branch* siblings;

	struct bayou_branch* children;
	uint8_t* elements; // byte-sized, aliased by the user !

	uint16_t children_len;
	uint16_t elements_len;
};

// used for holes, branches, elements
// only fragments when deleting elements
struct bayou_pool
{
	bool dyn; // wether we can realloc
	void* buf;

	uint16_t cur;
	uint16_t len;

	uint16_t step_len; // length of the type contained in buf
	uint16_t maximum_len;
};

struct bayou
{
	struct bayou_pool pool_elements;
	struct bayou_pool pool_branches;
	struct bayou_pool pool_holes;
	uint16_t sizeof_element;

	struct bayou_branch* root;
	struct bayou_branch* selected_branch;
	uint16_t selected_element;
};

#endif
