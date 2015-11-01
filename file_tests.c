/* file minunit_example.c */

#include <stdio.h>
#include "minunit.h"
#include <stdlib.h>
#include <string.h>

int tests_run = 0;

static char *test_overwrite() {
	const char *filename = "testdir/file_1";
	/* Write to file */
	FILE *f = fopen(filename, "w");
	if (f == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}

	fprintf(f, "X");
	fclose(f);

	/* read the file and check if it makes sense */
	char * buffer = 0;
	long length;
	f = fopen (filename, "rb");

	if (f)
	{
		fseek (f, 0, SEEK_END);
		length = ftell (f);
		fseek (f, 0, SEEK_SET);
		buffer = malloc (length);
		if (buffer)
		{
			fread (buffer, 1, length, f);
		}
		fclose (f);
	}

	if (buffer)
	{
		mu_assert("error, file contents is incorrect", strcmp(buffer, "X") == 0);
	}

	return 0;
}

static char * all_tests() {
	mu_run_test(test_overwrite);
	return 0;
}


int main(void) {
	char *result = all_tests();
	if (result != 0) {
		printf("%s\n", result);
	}
	else {
		printf("ALL TESTS PASSED\n");
	}
	printf("Tests run: %d\n", tests_run);

	return result != 0;
}
