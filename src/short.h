#ifndef H_BAYOU_SHORT
#define H_BAYOU_SHORT

#include <stdint.h>
#include <stdbool.h>
#include "def.h"

void bayou_branch_root(struct bayou* bayou);
bool bayou_branch_parent(struct bayou* bayou);
bool bayou_branch_sibling(struct bayou* bayou);
bool bayou_branch_child(struct bayou* bayou);
uint16_t bayou_count_children(struct bayou* bayou);

bool bayou_element_set(struct bayou* bayou, uint16_t id);
uint16_t bayou_element_get(struct bayou* bayou);
bool bayou_element_prev(struct bayou* bayou);
bool bayou_element_next(struct bayou* bayou);
uint16_t bayou_count_elements(struct bayou* bayou);
bool bayou_should_defrag(struct bayou* bayou);

#endif
