#include "tcp.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define DEFAULT_INTERVAL_SECONDS 2UL
#define MAX_INTERVAL_SECONDS 3600UL

static void print_usage(const char *program_name)
{
	printf("Usage: %s [--once] [--interval seconds]\n", program_name);
	printf("  --once              Print one TCP connection snapshot and exit.\n");
	printf("  --interval seconds  Refresh interval from 1 to 3600 (default: %lu).\n",
		   DEFAULT_INTERVAL_SECONDS);
}

static int parse_interval(const char *text, unsigned long *interval)
{
	char *end = NULL;
	unsigned long value;

	errno = 0;
	value = strtoul(text, &end, 10);
	if (errno != 0 || end == text || *end != '\0' ||
		value < 1UL || value > MAX_INTERVAL_SECONDS) {
		return -1;
	}

	*interval = value;
	return 0;
}

int main(int argc, char **argv)
{
	int once = 0;
	unsigned long interval = DEFAULT_INTERVAL_SECONDS;

	for (int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
			print_usage(argv[0]);
			return EXIT_SUCCESS;
		}
		if (strcmp(argv[i], "--once") == 0) {
			once = 1;
			continue;
		}
		if (strcmp(argv[i], "--interval") == 0) {
			if (i + 1 >= argc || parse_interval(argv[i + 1], &interval) != 0) {
				fprintf(stderr, "Invalid interval; provide a whole number from 1 to %lu seconds.\n",
						MAX_INTERVAL_SECONDS);
				print_usage(argv[0]);
				return EXIT_FAILURE;
			}
			++i;
			continue;
		}

		fprintf(stderr, "Unknown argument: %s\n", argv[i]);
		print_usage(argv[0]);
		return EXIT_FAILURE;
	}

	do {
		if (conntrace_print_tcp_snapshot() != 0) {
			return EXIT_FAILURE;
		}
		if (once) {
			break;
		}
		fflush(stdout);
		Sleep((DWORD)(interval * 1000UL));
		printf("\n");
	} while (1);

	return EXIT_SUCCESS;
}
