#define _POSIX_C_SOURCE 200809L
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Instruction Set */
static const char *const MOVE_LEFT	   = "<";
static const char *const MOVE_RIGHT	   = ">";
static const char *const INCREMENT	   = "+";
static const char *const DECREMENT	   = "-";
static const char *const OUTPUT		   = ".";
static const char *const INPUT		   = ",";
static const char *const OPEN_BRACKET  = "[";
static const char *const CLOSE_BRACKET = "]";

typedef struct {
	const char *arch;
	bool preprocess;
	bool optimize;

} __attribute__((aligned(16))) BFC;

static inline void help(void);

int main(int argc, char **argv) {
	if (1 == argc) {
		help();
		return EXIT_FAILURE;
	}

	static BFC self = {
		.arch		= "x64",
		.preprocess = true,
		.optimize	= true,
	};
	static const char *output = NULL;
	static char **input		  = NULL;
	static int opt			  = '\0';

	while (-1 != (opt = getopt(argc, argv, "ho:a:PO"))) {
		switch (opt) {
		case 'h':
			help();
			break;
		case 'o':
			output = optarg;
			break;
		case 'a':
			self.arch = optarg;
			break;
		case 'P':
			self.preprocess = false;
			break;
		case 'O':
			self.optimize = false;
			break;
		default:
			/* NOLINTED here because of fprintf_s usage recommendation */
			(void)fprintf(stderr, "Unknown Option: %c\n", opt); // NOLINT
			help();
			return EXIT_FAILURE;
		}
	}

	if (!(optind < argc)) {
		(void)fputs("No positional arguments", stderr);
		return EXIT_FAILURE;
	}
	input = &argv[optind];

	return EXIT_SUCCESS;
}

static inline void help(void) {
	(void)puts("BFC - Brainfuck Compiler\n"
			   "    Usage:\n"
			   "      `bfc [bf file] -o [asm output]`\n\n"
			   "    Options:\n"
			   "      -o  - Sets output file, by default [inputname].bf.o.s\n"
			   "      -a  - Sets architecture output, by default [native]\n"
			   "	  -P - Disables preprocessing\n"
			   "      -O - Disables optimization");
}
