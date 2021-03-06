#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/mlrutil.h"
#include "containers/lrec.h"

static lrece_t* lrec_find_entry(lrec_t* prec, char* key);
static void lrec_unlink(lrec_t* prec, lrece_t* pe);
static void lrec_link_at_head(lrec_t* prec, lrece_t* pe);
static void lrec_link_at_tail(lrec_t* prec, lrece_t* pe);

static void lrec_unbacked_free(lrec_t* prec);
static void lrec_free_single_line_backing(lrec_t* prec);
static void lrec_free_csv_backing(lrec_t* prec);
static void lrec_free_multiline_backing(lrec_t* prec);

// ----------------------------------------------------------------
lrec_t* lrec_unbacked_alloc() {
	lrec_t* prec = mlr_malloc_or_die(sizeof(lrec_t));
	memset(prec, 0, sizeof(lrec_t));
	prec->pfree_backing_func = lrec_unbacked_free;
	return prec;
}

// xxx cmt what this doesn't do with the line.
lrec_t* lrec_dkvp_alloc(char* line) {
	lrec_t* prec = mlr_malloc_or_die(sizeof(lrec_t));
	memset(prec, 0, sizeof(lrec_t));
	prec->psingle_line = line;
	prec->pfree_backing_func = lrec_free_single_line_backing;
	return prec;
}

// xxx cmt what this doesn't do with the line.
lrec_t* lrec_nidx_alloc(char* line) {
	lrec_t* prec = mlr_malloc_or_die(sizeof(lrec_t));
	memset(prec, 0, sizeof(lrec_t));
	prec->psingle_line  = line;
	prec->pfree_backing_func = lrec_free_single_line_backing;
	return prec;
}

lrec_t* lrec_csvlite_alloc(char* data_line) {
	lrec_t* prec = mlr_malloc_or_die(sizeof(lrec_t));
	memset(prec, 0, sizeof(lrec_t));
	prec->psingle_line = data_line;
	prec->pfree_backing_func = lrec_free_csv_backing;
	return prec;
}

lrec_t* lrec_csv_alloc(char* data_line) {
	lrec_t* prec = mlr_malloc_or_die(sizeof(lrec_t));
	memset(prec, 0, sizeof(lrec_t));
	prec->psingle_line = data_line;
	prec->pfree_backing_func = lrec_free_csv_backing;
	return prec;
}

lrec_t* lrec_xtab_alloc(slls_t* pxtab_lines) {
	lrec_t* prec = mlr_malloc_or_die(sizeof(lrec_t));
	memset(prec, 0, sizeof(lrec_t));
	prec->pxtab_lines = pxtab_lines;
	prec->pfree_backing_func = lrec_free_multiline_backing;
	return prec;
}

// ----------------------------------------------------------------
void lrec_put_no_free(lrec_t* prec, char* key, char* value) {
	lrece_t* pe = lrec_find_entry(prec, key);

	if (pe != NULL) {
		if (pe->free_flags & LREC_FREE_ENTRY_VALUE) {
			free(pe->value);
		}
		pe->value = value;
		pe->free_flags &= ~LREC_FREE_ENTRY_VALUE;
	} else {
		pe = mlr_malloc_or_die(sizeof(lrece_t));
		pe->key        = key;
		pe->value      = value;
		pe->free_flags = 0;

		if (prec->phead == NULL) {
			pe->pprev   = NULL;
			pe->pnext   = NULL;
			prec->phead = pe;
			prec->ptail = pe;
		} else {
			pe->pprev   = prec->ptail;
			pe->pnext   = NULL;
			prec->ptail->pnext = pe;
			prec->ptail = pe;
		}
		prec->field_count++;
	}
}

void lrec_put(lrec_t* prec, char* key, char* value, char free_flags) {
	lrece_t* pe = lrec_find_entry(prec, key);

	if (pe != NULL) {
		if (pe->free_flags & LREC_FREE_ENTRY_VALUE) {
			free(pe->value);
		}
		pe->value = strdup(value);
		pe->free_flags |= LREC_FREE_ENTRY_VALUE;
	} else {
		pe = mlr_malloc_or_die(sizeof(lrece_t));
		pe->key         = strdup(key);
		pe->value       = strdup(value);
		pe->free_flags  = LREC_FREE_ENTRY_KEY | LREC_FREE_ENTRY_VALUE;

		if (prec->phead == NULL) {
			pe->pprev   = NULL;
			pe->pnext   = NULL;
			prec->phead = pe;
			prec->ptail = pe;
		} else {
			pe->pprev   = prec->ptail;
			pe->pnext   = NULL;
			prec->ptail->pnext = pe;
			prec->ptail = pe;
		}
		prec->field_count++;
	}
}

// ----------------------------------------------------------------
char* lrec_get(lrec_t* prec, char* key) {
	lrece_t* pe = lrec_find_entry(prec, key);
	if (pe != NULL) {
		return pe->value;
	} else {
		return NULL;
	}
}

// ----------------------------------------------------------------
void lrec_remove(lrec_t* prec, char* key) {
	lrece_t* pe = lrec_find_entry(prec, key);
	if (pe == NULL)
		return;

	lrec_unlink(prec, pe);

	if (pe->free_flags & LREC_FREE_ENTRY_KEY) {
		free(pe->key);
	}
	if (pe->free_flags & LREC_FREE_ENTRY_VALUE) {
		if (pe->value != NULL)
			free(pe->value);
	}

	free(pe);
}

// xxx cmt this assumes new_key doesn't need freeing.
// Before:
//   "x" => "3"
//   "y" => "4"  <-- pold
//   "z" => "5"  <-- pnew
//
// Rename y to z
//
// After:
//   "x" => "3"
//   "z" => "4"
//
void lrec_rename(lrec_t* prec, char* old_key, char* new_key) {

	lrece_t* pold = lrec_find_entry(prec, old_key);
	if (pold != NULL) {
		lrece_t* pnew = lrec_find_entry(prec, new_key);

		if (pnew == NULL) { // E.g. rename "x" to "y" when "y" is not present
			if (pold->free_flags & LREC_FREE_ENTRY_KEY) {
				free(pold->key);
				pold->key = new_key;
				pold->free_flags &= ~LREC_FREE_ENTRY_KEY;
			} else {
				pold->key = new_key;
			}

		} else { // E.g. rename "x" to "y" when "y" is already present
			if (pnew->free_flags & LREC_FREE_ENTRY_VALUE) {
				free(pnew->value);
			}
			if (pold->free_flags & LREC_FREE_ENTRY_KEY) {
				free(pold->key);
				pold->free_flags &= ~LREC_FREE_ENTRY_KEY;
			}
			pold->key = new_key;
			lrec_unlink(prec, pnew);
			free(pnew);
		}
	}
}

// xxx comment
void lrec_set_name(lrec_t* prec, lrece_t* pfield, char* new_key) {
	if (pfield->free_flags & LREC_FREE_ENTRY_VALUE) {
		free(pfield->value);
	}
	pfield->key = new_key;
	pfield->free_flags |= ~LREC_FREE_ENTRY_VALUE;
}

// ----------------------------------------------------------------
void lrec_move_to_head(lrec_t* prec, char* key) {
	lrece_t* pe = lrec_find_entry(prec, key);
	if (pe == NULL)
		return;

	lrec_unlink(prec, pe);
	lrec_link_at_head(prec, pe);
}

void lrec_move_to_tail(lrec_t* prec, char* key) {
	lrece_t* pe = lrec_find_entry(prec, key);
	if (pe == NULL)
		return;

	lrec_unlink(prec, pe);
	lrec_link_at_tail(prec, pe);
}

// ----------------------------------------------------------------
static void lrec_unlink(lrec_t* prec, lrece_t* pe) {
	if (pe == prec->phead) {
		if (pe == prec->ptail) {
			prec->phead = NULL;
			prec->ptail = NULL;
		} else {
			prec->phead = pe->pnext;
			pe->pnext->pprev = NULL;
		}
	} else {
		pe->pprev->pnext = pe->pnext;
		if (pe == prec->ptail) {
			prec->ptail = pe->pprev;
		} else {
			pe->pnext->pprev = pe->pprev;
		}
	}
	prec->field_count--;
}

static void lrec_link_at_head(lrec_t* prec, lrece_t* pe) {

	// xxx factor out private methods for this: shared with lrec_put
	if (prec->phead == NULL) {
		pe->pprev   = NULL;
		pe->pnext   = NULL;
		prec->phead = pe;
		prec->ptail = pe;
	} else {
		// [b,c,d] + a
		pe->pprev   = NULL;
		pe->pnext   = prec->phead;
		prec->phead->pprev = pe;
		prec->phead = pe;
	}
	prec->field_count++;
}

static void lrec_link_at_tail(lrec_t* prec, lrece_t* pe) {

	if (prec->phead == NULL) {
		pe->pprev   = NULL;
		pe->pnext   = NULL;
		prec->phead = pe;
		prec->ptail = pe;
	} else {
		pe->pprev   = prec->ptail;
		pe->pnext   = NULL;
		prec->ptail->pnext = pe;
		prec->ptail = pe;
	}
	prec->field_count++;
}

// ----------------------------------------------------------------
void lrec_free(lrec_t* prec) {
	if (prec == NULL)
		return;
	for (lrece_t* pe = prec->phead; pe != NULL; /*pe = pe->pnext*/) {
		if ((pe->free_flags & LREC_FREE_ENTRY_KEY) && (pe->key != NULL))
			free(pe->key);
		if ((pe->free_flags & LREC_FREE_ENTRY_VALUE) && (pe->value != NULL))
			free(pe->value);
		lrece_t* ope = pe;
		pe = pe->pnext;
		free(ope);
	}
	prec->pfree_backing_func(prec);
	free(prec);
}

// ----------------------------------------------------------------
void lrec_dump(lrec_t* prec) {
	printf("field_count = %d\n", prec->field_count);
	printf("| phead: %16p | ptail %16p\n", prec->phead, prec->ptail);
	for (lrece_t* pe = prec->phead; pe != NULL; pe = pe->pnext) {
		const char* key_string = (pe == NULL) ? "none" :
			pe->key == NULL ? "null" :
			pe->key;
		const char* value_string = (pe == NULL) ? "none" :
			pe->value == NULL ? "null" :
			pe->value;
		printf(
		"| prev: %16p curr: %16p next: %16p | key: %12s | value: %12s |\n",
			pe->pprev, pe, pe->pnext,
			key_string, value_string);
	}
}

void lrec_dump_titled(char* msg, lrec_t* prec) {
	printf("%s:\n", msg);
	lrec_dump(prec);
	printf("\n");
}

// ----------------------------------------------------------------
static void lrec_unbacked_free(lrec_t* prec) {
}

static void lrec_free_single_line_backing(lrec_t* prec) {
	free(prec->psingle_line);
}

static void lrec_free_csv_backing(lrec_t* prec) {
	free(prec->psingle_line);
}

static void lrec_free_multiline_backing(lrec_t* prec) {
	slls_free(prec->pxtab_lines);
}

// ----------------------------------------------------------------
static char* static_nidx_keys[] = {
	"0",   "1",  "2",  "3",  "4",  "5",  "6",  "7",  "8",  "9",
	"10", "11", "12", "13", "14", "15", "16", "17", "18", "19",
	"20", "21", "22", "23", "24", "25", "26", "27", "28", "29",
	"30", "31", "32", "33", "34", "35", "36", "37", "38", "39",
	"40", "41", "42", "43", "44", "45", "46", "47", "48", "49",
	"50", "51", "52", "53", "54", "55", "56", "57", "58", "59",
	"60", "61", "62", "63", "64", "65", "66", "67", "68", "69",
	"70", "71", "72", "73", "74", "75", "76", "77", "78", "79",
	"80", "81", "82", "83", "84", "85", "86", "87", "88", "89",
	"90", "91", "92", "93", "94", "95", "96", "97", "98", "99", "100"
};

char* make_nidx_key(int idx, char* pfree_flags) {
	if ((0 <= idx) && (idx <= 100)) {
		*pfree_flags = 0;
		return static_nidx_keys[idx];
	} else {
		char buf[32];
		sprintf(buf, "%d", idx);
		*pfree_flags = LREC_FREE_ENTRY_KEY;
		return strdup(buf);
	}
}

// ================================================================

// ----------------------------------------------------------------
// Note on efficiency:
//
// I was imagining/hoping that strcmp has additional optimizations (e.g.
// hand-coded in assembly), so I don't *want* to re-implement it (i.e. I
// probably can't outperform it).
//
// But actual experiments show I get about a 1-2% performance gain doing it
// myself (on my particular system).

static lrece_t* lrec_find_entry(lrec_t* prec, char* key) {
#if 1
	for (lrece_t* pe = prec->phead; pe != NULL; pe = pe->pnext) {
		char* pa = pe->key;
		char* pb = key;
		while (*pa && *pb && (*pa == *pb)) {
			pa++;
			pb++;
		}
		if (*pa == 0 && *pb == 0)
			return pe;
	}
	return NULL;
#else
	for (lrece_t* pe = prec->phead; pe != NULL; pe = pe->pnext)
		if (streq(pe->key, key))
			return pe;
	return NULL;
#endif
}

// ----------------------------------------------------------------
lrec_t* lrec_literal_1(char* k1, char* v1) {
	lrec_t* prec = lrec_unbacked_alloc();
	lrec_put_no_free(prec, k1, v1);
	return prec;
}

lrec_t* lrec_literal_2(char* k1, char* v1, char* k2, char* v2) {
	lrec_t* prec = lrec_unbacked_alloc();
	lrec_put_no_free(prec, k1, v1);
	lrec_put_no_free(prec, k2, v2);
	return prec;
}

lrec_t* lrec_literal_3(char* k1, char* v1, char* k2, char* v2, char* k3, char* v3) {
	lrec_t* prec = lrec_unbacked_alloc();
	lrec_put_no_free(prec, k1, v1);
	lrec_put_no_free(prec, k2, v2);
	lrec_put_no_free(prec, k3, v3);
	return prec;
}

lrec_t* lrec_literal_4(char* k1, char* v1, char* k2, char* v2, char* k3, char* v3, char* k4, char* v4) {
	lrec_t* prec = lrec_unbacked_alloc();
	lrec_put_no_free(prec, k1, v1);
	lrec_put_no_free(prec, k2, v2);
	lrec_put_no_free(prec, k3, v3);
	lrec_put_no_free(prec, k4, v4);
	return prec;
}

void lrec_print(lrec_t* prec) {
	FILE* output_stream = stdout;
	char rs = '\n';
	char fs = ',';
	char ps = '=';
	int nf = 0;
	for (lrece_t* pe = prec->phead; pe != NULL; pe = pe->pnext) {
		if (nf > 0)
			fputc(fs, output_stream);
		fputs(pe->key, output_stream);
		fputc(ps, output_stream);
		fputs(pe->value, output_stream);
		nf++;
	}
	fputc(rs, output_stream);
}
