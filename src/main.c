#include <stdio.h>
#include <stdlib.h>

/* Instruction Set */
#define MOVE_LEFT	  "<"
#define MOVE_RIGHT	  ">"
#define INCREMENT	  "+"
#define DECREMENT	  "-"
#define OUTPUT		  "."
#define INPUT		  ","
#define OPEN_BRACKET  "["
#define CLOSE_BRACKET "]"

static inline void help(void);

int main(int argc, char **argv) {

	help();

	return EXIT_SUCCESS;
	(void)argc;
	(void)argv;
}

static inline void help(void) {
	(void)puts("BFC - Brainfuck Compiler\n"
			   "    Usage:\n"
			   "      `bfc [bf file] -o [asm output]`\n\n"
			   "    Options:\n"
			   "      -o  - Sets output file, by default [inputname].bf.o.s\n"
			   "      -a  - Sets architecture output, by default [native]\n"
			   "	  -np - Disables preprocessing\n"
			   "      -no - Disables optimization");
}
