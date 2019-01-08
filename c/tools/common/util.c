#include <stdio.h>

ssize_t flen(FILE *f)
{
	ssize_t len;
	ssize_t pos = ftell(f);
	fseek(f, 0, SEEK_END);
	len = ftell(f);
	fseek(f, pos, SEEK_SET);

	return len;
}