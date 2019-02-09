#include "../sub/testoasterror/src/testoasterror.h"

// source include
#include "tests.c"

#define BAYOU_TEST_FUNCS 12
#define BAYOU_TEST_SLOTS 200

int main()
{
	bool results[BAYOU_TEST_SLOTS];

	void (*funcs[BAYOU_TEST_FUNCS])(struct testoasterror*) =
	{
		test_init,
		test_add,
		test_split_simple,
		test_split_complex,
		test_count_branches,
		test_frag,
		test_defrag,
		test_defrag_multi,
		test_rm_branches,
		test_defrag_branches,
		test_limit_branches,
		test_limit_elements
	};

	struct testoasterror test;

	testoasterror_init(
		&test,
		results,
		BAYOU_TEST_SLOTS,
		funcs,
		BAYOU_TEST_FUNCS);

	testoasterror_run(&test);

	return 0;
}
