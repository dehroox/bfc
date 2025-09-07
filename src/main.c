#define _POSIX_C_SOURCE 200809L
#include <fcntl.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
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
static inline ssize_t readfile(const char *filePath, char **bufferPtr);

int main(int argc, char **argv) {
	if (1 == argc) {
		help();
		return EXIT_FAILURE;
	}

	/* Variable Initialization */
	static BFC self = {
		.arch		= "x64",
		.preprocess = true,
		.optimize	= true,
	};

	FILE *output = stdout;
	char *input	 = NULL;
	int opt		 = '\0';

	ssize_t fileSize = 0;
	char *buffer	 = NULL;

	/* Argument Parsing */
	while (-1 != (opt = getopt(argc, argv, "ho:a:PO"))) {
		switch (opt) {
		case 'h':
			help();
			break;
		case 'o':
			output = fopen(optarg, "we");
			if (NULL == output) {
				perror("Failed to open output file");
				return EXIT_FAILURE;
			}
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
			(void)fprintf(stderr, "Unknown Option: %c\n", opt);
			help();
			return EXIT_FAILURE;
		}
	}

	if (!(optind < argc)) {
		(void)fputs("No input provided", stderr);
		return EXIT_FAILURE;
	}
	input = argv[optind];

	/* File reading */
	fileSize = readfile(input, &buffer);
	if (fileSize < 0) {
		free(buffer);
		perror(input);
		return EXIT_FAILURE;
	}
	(void)fwrite(buffer, 1, (size_t)fileSize, output); /* temporary */

	/* Cleanup */
	free(buffer);
	return EXIT_SUCCESS;
}

static inline void help(void) {
	(void)puts("BFC - Brainfuck Compiler\n"
			   "    Usage:\n"
			   "      `bfc -o [asm output] [bf file]`\n\n"
			   "    Options:\n"
			   "      -o:  - Sets output file, by default [stdio]\n"
			   "      -a:  - Sets architecture output, by default [native]\n"
			   "      -P   - Disables preprocessing\n"
			   "      -O   - Disables optimization\n");
}

static inline ssize_t readfile(const char *filePath, char **bufferPtr) {
	if (filePath == NULL || bufferPtr == NULL) {
		return -1;
	}

	int fileDescriptor = open(filePath, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
	if (fileDescriptor < 0) {
		return -1;
	}

	struct stat fileStatus;
	if (fstat(fileDescriptor, &fileStatus) < 0) {
		close(fileDescriptor);
		return -1;
	}

	if (fileStatus.st_size < 0 || fileStatus.st_size > SSIZE_MAX) {
		close(fileDescriptor);
		return -1;
	}

	const ssize_t fileSize	= fileStatus.st_size;
	const size_t bufferSize = (size_t)fileSize + 1;

	*bufferPtr = malloc(bufferSize);
	if (*bufferPtr == NULL) {
		close(fileDescriptor);
		return -1;
	}

	ssize_t totalBytesRead = 0;
	while (totalBytesRead < fileSize) {
		ssize_t bytesRead = read(fileDescriptor, *bufferPtr + totalBytesRead,
								 (size_t)(fileSize - totalBytesRead));

		if (bytesRead <= 0) {
			if (bytesRead == 0 && totalBytesRead == fileSize) {
				break;
			}
			free(*bufferPtr);
			*bufferPtr = NULL;
			close(fileDescriptor);
			return -1;
		}

		totalBytesRead += bytesRead;
	}

	if (totalBytesRead < fileSize) {
		free(*bufferPtr);
		*bufferPtr = NULL;
		close(fileDescriptor);
		return -1;
	}

	close(fileDescriptor);
	(*bufferPtr)[totalBytesRead] = '\0';
	return totalBytesRead;
}
