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
static inline ssize_t readfile(const char *filePath, char **bufferPtr,
							   size_t *bufferCapacityPtr);

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
	char **input = NULL;
	int opt		 = '\0';

	ssize_t fileSize	  = 0;
	char *buffer		  = NULL;
	size_t bufferCapacity = 0;

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
		(void)fputs("No positional arguments", stderr);
		return EXIT_FAILURE;
	}
	input = &argv[optind];

	/* File reading */
	for (int i = 0; i < (argc - optind); ++i) {
		fileSize = readfile(input[i], &buffer, &bufferCapacity);
		if (fileSize < 0) {
			free(buffer);
			perror(input[i]);
			return EXIT_FAILURE;
		}
		(void)fwrite(buffer, 1, (size_t)fileSize, output); /* temporary */
	}

	/* Cleanup */
	free(buffer);
	return EXIT_SUCCESS;
}

static inline void help(void) {
	(void)puts("BFC - Brainfuck Compiler\n"
			   "    Usage:\n"
			   "      `bfc [bf file] -o [asm output]`\n\n"
			   "    Options:\n"
			   "      -o  - Sets output file, by default [stdio]\n"
			   "      -a  - Sets architecture output, by default [native]\n"
			   "      -P  - Disables preprocessing\n"
			   "      -O  - Disables optimization");
}

static inline ssize_t readfile(const char *filePath, char **bufferPtr,
							   size_t *bufferCapacityPtr) {
	int fileDescriptor = open(filePath, O_RDONLY | O_CLOEXEC);
	if (fileDescriptor < 0) {
		return -1;
	}

	/* Initialize or re-initialize the buffer */
	if (*bufferPtr == NULL) {
		*bufferCapacityPtr = 262144; /* Initial buffer size: 256KB */
		*bufferPtr		   = malloc(*bufferCapacityPtr + 1);
		if (*bufferPtr == NULL) {
			close(fileDescriptor);
			return -1;
		}
	}

	size_t bufferLength = 0;
	ssize_t bytesRead;

	while ((bytesRead = read(fileDescriptor, *bufferPtr + bufferLength,
							 *bufferCapacityPtr - bufferLength)) > 0) {
		bufferLength += (size_t)bytesRead;

		/* If buffer is full, reallocate to a larger size */
		if (bufferLength == *bufferCapacityPtr) {
			size_t newCapacity = *bufferCapacityPtr * 2;
			char *newBuffer	   = realloc(*bufferPtr, newCapacity + 1);
			if (newBuffer == NULL) {
				close(fileDescriptor);
				return -1;
			}
			*bufferPtr		   = newBuffer;
			*bufferCapacityPtr = newCapacity;
		}
	}

	close(fileDescriptor);

	if (bytesRead < 0) {
		return -1;
	}

	/* Resize to fit the exact file size and add the null terminator */
	char *finalBuffer = realloc(*bufferPtr, bufferLength + 1);
	if (finalBuffer == NULL) {
		return -1;
	}
	*bufferPtr				   = finalBuffer;
	*bufferCapacityPtr		   = bufferLength;
	(*bufferPtr)[bufferLength] = '\0';

	return (ssize_t)bufferLength;
}
