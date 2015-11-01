/* file minunit_example.c */

#include <stdio.h>
#include "minunit.h"
#include <stdlib.h>
#include <string.h>

int tests_run = 0;

void write_file(const char *filename, const char *mode, const char *contents)
{
	FILE *f = fopen(filename, mode);
	if (f == NULL)
	{
		printf("Error opening file!\n");
		exit(1);
	}
	fprintf(f, "%s", contents);
	fclose(f);
}

void append_file(const char *filename, const char *contents)
{
	write_file(filename, "a", contents);
}

void overwrite_file(const char *filename, const char *contents)
{
	write_file(filename, "w", contents);
}


/* This function can return NULL */
char *get_file_contents(const char *filename)
{
	/* read the file and check if it makes sense */
	char * buffer = 0;
	long length;
	FILE *f = fopen (filename, "rb");

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

	/* buffer can be NULL */
	return buffer;
}

static char *test_overwrite() {
	const char *filename = "testdir/file_1";
	overwrite_file(filename, "X");

	/* read the file and check if it makes sense */
	char *file_contents = get_file_contents(filename);

	if (file_contents)
	{
		mu_assert("test_overwrite: file contents is incorrect", strcmp(file_contents, "X") == 0);
	}

	return 0;
}

static char *test_append() {
	const char *filename = "testdir/file_2";
	overwrite_file(filename, "X");
	append_file(filename, "X");

	/* read the file and check if it makes sense */
	char *file_contents = get_file_contents(filename);

	if (file_contents)
	{
		mu_assert("test_append: file contents is incorrect", strcmp(file_contents, "XX") == 0);
	}

	return 0;
}

/* test deleting file contents */
static char *test_delete_contents()
{
	const char *filename = "testdir/file_3";
	overwrite_file(filename, "Some junk.");
	overwrite_file(filename, "");
	
	/* read the file and check if it makes sense */
	char *file_contents = get_file_contents(filename);

	if (file_contents)
	{
		mu_assert("test_delete_contents: file contents is incorrect", strcmp(file_contents, "") == 0);
	}

	return 0;


}

static char * all_tests() {
	mu_run_test(test_overwrite);
	mu_run_test(test_append);
	mu_run_test(test_delete_contents);
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
