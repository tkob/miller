#ifndef MLRCLI_H
#define MLRCLI_H

#include "containers/sllv.h"
#include "input/lrec_reader.h"
#include "input/lrec_reader_mmap.h"
#include "mapping/mapper.h"
#include "output/lrec_writer.h"

typedef struct _cli_opts_t {
	char irs;
	char ifs;
	char ips;
	int  allow_repeat_ifs;

	char ors;
	char ofs;
	char ops;

	char* ofmt;

	int            use_mmap_reader;
	lrec_reader_t*      plrec_reader;
	reader_mmap_t* preader_mmap;
	sllv_t*        pmapper_list;
	lrec_writer_t*      plrec_writer;

	char** filenames; // null-terminated

} cli_opts_t;

cli_opts_t* parse_command_line(int argc, char** argv);
void cli_opts_free(cli_opts_t* popts);

#endif // MLRCLI_H
