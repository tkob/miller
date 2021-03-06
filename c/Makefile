# ================================================================
CCOMP=gcc
CFLAGS=-std=gnu99
IFLAGS=-I.
WFLAGS=-Wall -Werror
LFLAGS=-lm

# You can do
#
#   make -e INSTALLDIR=/usr/local/bin
#
# pending https://github.com/johnkerl/miller/issues/9
INSTALLDIR=$(HOME)/bin

CC=$(CCOMP) $(CFLAGS) $(IFLAGS) $(WFLAGS) -O3
#CCDEBUG=$(CCOMP) -g -O1 $(CFLAGS) $(IFLAGS) $(WFLAGS)
CCDEBUG=$(CCOMP) -g $(CFLAGS) $(IFLAGS) $(WFLAGS)

NON_DSL_SRCS = *.c cli/*.c lib/*.c containers/*.c stream/*.c input/*.c mapping/*.c output/*.c
PDSL_OBJS = ./dsls/put_dsl_parse.o    ./dsls/put_dsl_lexer.o    ./dsls/put_dsl_wrapper.o
FDSL_OBJS = ./dsls/filter_dsl_parse.o ./dsls/filter_dsl_lexer.o ./dsls/filter_dsl_wrapper.o

TEST_LREC_SRCS = lib/mlrutil.c lib/mlr_globals.c containers/lrec.c containers/header_keeper.c containers/sllv.c \
containers/slls.c containers/lhmslv.c \
input/file_reader_mmap.c      input/file_reader_stdio.c \
input/lrec_reader_mmap_csv.c  input/lrec_reader_stdio_csv.c \
input/lrec_reader_mmap_csvlite.c  input/lrec_reader_stdio_csvlite.c \
input/lrec_reader_mmap_dkvp.c input/lrec_reader_stdio_dkvp.c \
input/lrec_reader_mmap_nidx.c input/lrec_reader_stdio_nidx.c \
input/lrec_reader_mmap_xtab.c input/lrec_reader_stdio_xtab.c \
containers/test_lrec.c

TEST_JOIN_BUCKET_KEEPER_SRCS = \
lib/mlrutil.c lib/mlr_globals.c \
mapping/context.c \
containers/lrec.c \
containers/sllv.c containers/slls.c containers/lhmslv.c containers/hss.c containers/mixutil.c \
containers/header_keeper.c \
containers/join_bucket_keeper.c \
input/lrec_reader_in_memory.c input/lrec_readers.c \
input/lrec_reader_mmap_csv.c input/lrec_reader_stdio_csv.c \
input/lrec_reader_mmap_csvlite.c input/lrec_reader_stdio_csvlite.c \
input/lrec_reader_mmap_dkvp.c input/lrec_reader_stdio_dkvp.c \
input/lrec_reader_mmap_nidx.c input/lrec_reader_stdio_nidx.c \
input/lrec_reader_mmap_xtab.c input/lrec_reader_stdio_xtab.c \
input/file_reader_mmap.c input/file_reader_stdio.c \
containers/test_join_bucket_keeper.c

# ================================================================
# User-make: creates the executable and runs unit & regression tests (without
# valgrind)
# ----> This is the default target for anyone pulling the repo and trying to
#       build it to be able to use it. It just needs flex and the C compiler.
top: mlr tests

# Dev-make: updates the tags file, creates the executable, and runs unit &
# regression tests (with valgrind)
# ----> This is the target for a developer to run before a commit.
#       It requires ctags and valgrind in addition to flex and the C compiler.
dev: tags mlr dev-tests

install: mlr test
	cp mlr $(INSTALLDIR)

# ================================================================
tags: .always
	ctags -R .

mlr: .always dsls
	$(CC) $(NON_DSL_SRCS) $(PDSL_OBJS) $(FDSL_OBJS) $(LFLAGS) -o mlr

dsls: .always
	make -C dsls put_dsl_parse.o
	make -C dsls put_dsl_lexer.o
	make -C dsls put_dsl_wrapper.o
	make -C dsls filter_dsl_parse.o
	make -C dsls filter_dsl_lexer.o
	make -C dsls filter_dsl_wrapper.o

# ----------------------------------------------------------------
# Other executable variants

# Debug version
mlrg: .always dsls
	$(CCDEBUG) $(NON_DSL_SRCS) $(PDSL_OBJS) $(FDSL_OBJS) $(LFLAGS) -o mlrg

# Memcheck version
mlrk: .always dsls
	$(CC) -DUSE_MCHECK $(NON_DSL_SRCS) $(PDSL_OBJS) $(FDSL_OBJS) $(LFLAGS) -o mlrk

# Profile version
mlrp: .always dsls
	$(CC) -g -pg $(NON_DSL_SRCS) $(PDSL_OBJS) $(FDSL_OBJS) $(LFLAGS) -o mlrp

# ================================================================
# User-tests: don't use valgrind: they may not have it, and valgrind is most
# useful for detecting errors at build time. They don't need it to produce an
# executable.

tests: unit-test reg-test

unit-test: test-mlrutil test-lrec test-join-bucket-keeper
	./test-mlrutil
	./test-lrec
	./test-join-bucket-keeper
	@echo
	@echo DONE

reg-test:
	./test/run

# ----------------------------------------------------------------
# Use valgrind at dev time for additional reassurance.

dev-tests: dev-unit-test reg-test

# Unfortunately --error-exitcode=1 doesn't work well since there are
# valgrind-detected errors in stdlibs. :(
dev-unit-test: test-mlrutil test-lrec test-join-bucket-keeper
	#valgrind --leak-check=full --error-exitcode=1 ./a.out
	valgrind --leak-check=full ./test-mlrutil
	valgrind --leak-check=full ./test-lrec
	valgrind --leak-check=full ./test-join-bucket-keeper
	@echo
	@echo DONE

# Run this after unit-test expected output has changed, and is verified to be
# OK. (Example: after adding new test cases in test/run.)
regtest-copy:
	cp test/output/out test/expected

# ----------------------------------------------------------------
# Unit-test executables

test-lrec: .always
	$(CCDEBUG) -D__TEST_LREC_MAIN__ $(TEST_LREC_SRCS) -o test-lrec

test-mlrutil: .always
	$(CCDEBUG) -D__TEST_MLRUTIL_MAIN__ lib/mlrutil.c lib/test_mlrutil.c -o test-mlrutil

test-join-bucket-keeper: .always
	$(CCDEBUG) -D__TEST_JOIN_BUCKET_KEEPER_MAIN__ $(TEST_JOIN_BUCKET_KEEPER_SRCS) -o test-join-bucket-keeper

# ----------------------------------------------------------------
# Standalone mains
# TODO: replace the interesting content with unit tests; jettison the rest

dheap:
	$(CCDEBUG) -D__DHEAP_MAIN__ containers/dheap.c lib/mlrutil.c

slls:
	$(CCDEBUG) -D__SLLS_MAIN__ containers/slls.c lib/mlrutil.c

sllv:
	$(CCDEBUG) -D__SLLV_MAIN__ containers/sllv.c lib/mlrutil.c

hmss:
	$(CCDEBUG) -D__HMSS_MAIN__ containers/hmss.c  lib/mlrutil.c

lhms2v:
	$(CCDEBUG) -D__LHMS2V_MAIN__ containers/lhms2v.c  lib/mlrutil.c

lhmslv:
	$(CCDEBUG) -D__LHMSLV_MAIN__ containers/lhmslv.c  lib/mlrutil.c containers/slls.c

lhmss:
	$(CCDEBUG) -D__LHMSS_MAIN__ containers/lhmss.c  lib/mlrutil.c

lhmsi:
	$(CCDEBUG) -D__LHMSI_MAIN__ containers/lhmsi.c  lib/mlrutil.c

lhmsv:
	$(CCDEBUG) -D__LHMSV_MAIN__ containers/lhmsv.c  lib/mlrutil.c

lrec:
	$(CCDEBUG) -D__LREC_MAIN__ containers/lrec.c  lib/mlrutil.c

top_keeper:
	$(CCDEBUG) -D__TOP_KEEPER_MAIN__ containers/top_keeper.c lib/mlrutil.c

pk:
	$(CCDEBUG) -D__PERCENTILE_KEEPER_MAIN__ containers/percentile_keeper.c lib/mlrutil.c

ap:
	$(CCDEBUG) -D__AP_MAIN__ cli/argparse.c containers/sllv.c containers/slls.c lib/mlrutil.c

evl:
	$(CCDEBUG) -D__LREC_EVALUATORS_MAIN__ mapping/lrec_evaluators.c mapping/mlr_val.c containers/mlr_dsl_ast.c containers/sllv.c containers/slls.c lib/mlrutil.c lib/mtrand.c containers/lrec.c -lm

lrim:
	$(CCDEBUG) -D__LREC_READER_IN_MEMORY_MAIN__ input/lrec_reader_in_memory.c containers/sllv.c containers/lrec.c containers/slls.c lib/mlrutil.c lib/mlr_globals.c -lm

# ================================================================
clean:
	@rm -vf mlr mlrd mlrg mlrp tester
	@make -C dsls clean
perfclean profclean:
	@rm -vf gmon.out perf.data perf.data.old
.always:
	@true
