#include <stdio.h>
#include <string.h>
#include "lib/minunit.h"
#include "lib/mlrutil.h"

#ifdef __TEST_MLRUTIL_MAIN__
int tests_run         = 0;
int tests_failed      = 0;
int assertions_run    = 0;
int assertions_failed = 0;

// ----------------------------------------------------------------
static char * test_canonical_mod() {
	mu_assert("error: canonical_mod -7", mlr_canonical_mod(-7, 5) == 3);
	mu_assert("error: canonical_mod -6", mlr_canonical_mod(-6, 5) == 4);
	mu_assert("error: canonical_mod -5", mlr_canonical_mod(-5, 5) == 0);
	mu_assert("error: canonical_mod -4", mlr_canonical_mod(-4, 5) == 1);
	mu_assert("error: canonical_mod -3", mlr_canonical_mod(-3, 5) == 2);
	mu_assert("error: canonical_mod -2", mlr_canonical_mod(-2, 5) == 3);
	mu_assert("error: canonical_mod -1", mlr_canonical_mod(-1, 5) == 4);
	mu_assert("error: canonical_mod  0", mlr_canonical_mod(0, 5) == 0);
	mu_assert("error: canonical_mod  1", mlr_canonical_mod(1, 5) == 1);
	mu_assert("error: canonical_mod  2", mlr_canonical_mod(2, 5) == 2);
	mu_assert("error: canonical_mod  3", mlr_canonical_mod(3, 5) == 3);
	mu_assert("error: canonical_mod  4", mlr_canonical_mod(4, 5) == 4);
	mu_assert("error: canonical_mod  5", mlr_canonical_mod(5, 5) == 0);
	mu_assert("error: canonical_mod  6", mlr_canonical_mod(6, 5) == 1);
	mu_assert("error: canonical_mod  7", mlr_canonical_mod(7, 5) == 2);
	return 0;
}

// ----------------------------------------------------------------
static char * test_scanners() {
	mu_assert("error: mlr_alloc_string_from_double", streq(mlr_alloc_string_from_double(4.25, "%.4f"), "4.2500"));
	mu_assert("error: mlr_alloc_string_from_ull", streq(mlr_alloc_string_from_ull(12345LL), "12345"));
	mu_assert("error: mlr_alloc_string_from_int", streq(mlr_alloc_string_from_int(12345), "12345"));
	return 0;
}

// xxx make UT-able intermediate
//double mlr_double_from_string_or_die(char* string);

// ----------------------------------------------------------------
static char * test_paste() {
	mu_assert("error: paste 2", streq(mlr_paste_2_strings("ab", "cd"), "abcd"));
	mu_assert("error: paste 3", streq(mlr_paste_3_strings("ab", "cd", "ef"), "abcdef"));
	mu_assert("error: paste 4", streq(mlr_paste_4_strings("ab", "cd", "ef", "gh"), "abcdefgh"));
	mu_assert("error: paste 5", streq(mlr_paste_5_strings("ab", "cd", "ef", "gh", "ij"), "abcdefghij"));
	return 0;
}

//int mlr_string_hash_func(char *str);
//int mlr_string_pair_hash_func(char* str1, char* str2);
//int slls_hash_func(sllv_t *plist);
//
//char*   mlr_get_line(FILE* input_stream, char rs, int do_strip);
//sllv_t* slls_from_line(char* line, char fs);
//char*   slls_join(sllv_t* plist, char fs); // xxx cmt mem mgt
//sllv_t* mlr_keys_from_record(lrec_t* prec);
//sllv_t* mlr_selected_values_from_record(lrec_t* prec, sllv_t* pselected_field_names, int copy_strings);
//int     slls_equals(sllv_t* pa, sllv_t* pb);

// ================================================================
static char * all_tests() {
	mu_run_test(test_canonical_mod);
	mu_run_test(test_scanners);
	mu_run_test(test_paste);
	return 0;
}

int main(int argc, char **argv) {
	char *result = all_tests();
	printf("\n");
	if (result != 0) {
		//printf("%s\n", result);
		printf("Not all unit tests passed\n");
	}
	else {
		printf("TEST_MLRUTIL: ALL UNIT TESTS PASSED\n");
	}
	printf("Tests      run:    %d\n", tests_run);
	printf("Tests      failed: %d\n", tests_failed);
	printf("Assertions run:    %d\n", assertions_run);
	printf("Assertions failed: %d\n", assertions_failed);

	return result != 0;
}
#endif // __TEST_MLRUTIL_MAIN__