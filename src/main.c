#ifdef __linux__
#define _GNU_SOURCE
#endif

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

/* Brainfuck Instructions */
enum BrainfuckInstruction {
	moveLeft	 = '<',
	moveRight	 = '>',
	increment	 = '+',
	decrement	 = '-',
	outputChar	 = '.',
	inputChar	 = ',',
	openBracket	 = '[',
	closeBracket = ']'
};

/* Write buffer safely */
static int writeAll(int fileDescriptor, const char *buffer, size_t length) {
	size_t remaining = length;
	const char *ptr	 = buffer;
	while (remaining) {
		ssize_t written = write(fileDescriptor, ptr, remaining);
		if (written <= 0) {
			return -1;
		}
		remaining -= (size_t)written;
		ptr += written;
	}
	return 0;
}

int main(int argc, char **argv) {
	int fdOut			= STDOUT_FILENO;
	char *inputFilePath = NULL;
	char *fileBuffer;
	size_t fileSize;
	struct stat fileStats;

	const char *architecture = "x64";
	bool preprocessEnabled	 = true;
	bool optimizationEnabled = true;

	int opt;
	while ((opt = getopt(argc, argv, "ho:a:PO")) != -1) {
		switch (opt) {
		case 'h':
			write(STDOUT_FILENO,
				  "BFC - Brainfuck Compiler\n"
				  "Usage: bfc -o [asm output] -a [arch] [-P] [-O] [bf file]\n"
				  "Options:\n"
				  "  -o: Set output file (default stdout)\n"
				  "  -a: Set architecture (default x64)\n"
				  "  -P: Disable preprocessing\n"
				  "  -O: Disable optimization\n",
				  183);
			return EXIT_SUCCESS;
		case 'o':
			fdOut =
				open(optarg, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
			if (fdOut < 0) {
				perror("open output file");
				return EXIT_FAILURE;
			}
			break;
		case 'a':
			architecture = optarg;
			break;
		case 'P':
			preprocessEnabled = false;
			break;
		case 'O':
			optimizationEnabled = false;
			break;
		default:
			return EXIT_FAILURE;
		}
	}

	if (optind < argc) {
		inputFilePath = argv[optind];
	}

	if (!inputFilePath) {
		write(STDERR_FILENO, "No input provided\n", 18);
		return EXIT_FAILURE;
	}

	int fdIn = open(inputFilePath, O_RDONLY | O_CLOEXEC);
	if (fdIn < 0) {
		perror(inputFilePath);
		return EXIT_FAILURE;
	}

	if (fstat(fdIn, &fileStats) < 0) {
		perror("fstat failed");
		close(fdIn);
		return EXIT_FAILURE;
	}
	if (fileStats.st_size == 0) {
		close(fdIn);
		return EXIT_SUCCESS;
	}

	fileSize   = (size_t)fileStats.st_size;
	fileBuffer = mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fdIn, 0);
	close(fdIn);
	if (fileBuffer == MAP_FAILED) {
		perror("mmap failed");
		return EXIT_FAILURE;
	}

#ifdef MADV_SEQUENTIAL
	madvise(fileBuffer, fileSize, MADV_SEQUENTIAL);
#endif

	if (writeAll(fdOut, fileBuffer, fileSize) < 0) {
		perror("write failed");
		munmap(fileBuffer, fileSize);
		if (fdOut != STDOUT_FILENO) {
			close(fdOut);
		}
		return EXIT_FAILURE;
	}

	munmap(fileBuffer, fileSize);
	if (fdOut != STDOUT_FILENO) {
		close(fdOut);
	}

	return EXIT_SUCCESS;
}
