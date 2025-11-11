#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include "util.h"

extern const char *argv0;

static void
vwarn(const char *a, va_list b)
{
	fprintf(stderr, "%s: ", argv0);
	vfprintf(stderr, a, b);
	if (a[0] && a[strlen(a) - 1] == ':') {
		putc(' ', stderr);
		perror(NULL);
	} else {
		putc('\n', stderr);
	}
}

void
warn(const char *a, ...)
{
	va_list b;

	va_start(b, a);
	vwarn(a, b);
	va_end(b);
}

void
fatal(const char *a, ...)
{
	va_list b;

	va_start(b, a);
	vwarn(a, b);
	va_end(b);
	exit(1);
}

void *
xmalloc(size_t a)
{
	void *b;

	b = malloc(a);
	if (!b)
		fatal("malloc:");

	return b;
}

static void *
reallocarray(void *a, size_t b, size_t c)
{
	if (c && b > SIZE_MAX / c) {
		errno = ENOMEM;
		return NULL;
	}
	return realloc(a, b * c);
}

void *
xreallocarray(void *a, size_t b, size_t c)
{
	a = reallocarray(a, b, c);
	if (!a)
		fatal("reallocarray:");

	return a;
}

char *
xmemdup(const char *a, size_t b)
{
	char *c;

	c = xmalloc(b);
	memcpy(c, a, b);

	return c;
}

int
xasprintf(char **a, const char *b, ...)
{
	va_list c;
	int d;
	size_t e;

	va_start(c, b);
	d = vsnprintf(NULL, 0, b, c);
	va_end(c);
	if (d < 0)
		fatal("vsnprintf:");
	e = d + 1;
	*a = xmalloc(e);
	va_start(c, b);
	d = vsnprintf(*a, e, b, c);
	va_end(c);
	if (d < 0 || (size_t)d >= e)
		fatal("vsnprintf:");

	return d;
}

void
bufadd(struct buffer *a, char b)
{
	if (a->cap <= a->len) {
		a->cap = a->cap ? a->cap * 2 : 1 << 8;
		a->data = realloc(a->data, a->cap);
		if (!a->data)
			fatal("realloc:");
	}
	a->data[a->len++] = b;
}

struct string *
mkstr(size_t a)
{
	struct string *b;

	b = xmalloc(sizeof(*b) + a + 1);
	b->n = a;

	return b;
}

void
delevalstr(void *a)
{
	struct evalstring *b = a;
	struct evalstringpart *c, *d;

	if (!b)
		return;
	for (c = b->parts; c; c = d) {
		d = c->next;
		if (c->var)
			free(c->var);
		else
			free(c->str);
		free(c);
	}
	free(b);
}

void
canonpath(struct string *a)
{
	char *c[60];
	int d;
	char *e, *f, *g;

	if (a->n == 0)
		fatal("empty path");
	e = f = a->s;
	g = a->s + a->n;
	d = 0;
	if (*e == '/') {
		++e;
		++f;
	}
	while (e < g) {
		switch (e[0]) {
		case '/':
			++e;
			continue;
		case '.':
			switch (e[1]) {
			case '\0': case '/':
				e += 2;
				continue;
			case '.':
				if (e[2] != '/' && e[2] != '\0')
					break;
				if (d > 0) {
					f = c[--d];
				} else {
					*f++ = e[0];
					*f++ = e[1];
					*f++ = e[2];
				}
				e += 3;
				continue;
			}
		}
		if (d == LEN(c))
			fatal("path has too many components: %s", a->s);
		c[d++] = f;
		while (*e != '/' && *e != '\0')
			*f++ = *e++;
		*f++ = *e++;
	}
	if (f == a->s) {
		*f++ = '.';
		*f = '\0';
	} else {
		*--f = '\0';
	}
	a->n = f - a->s;
}

int
makedirs(struct string *a, bool b)
{
	int c;
	struct stat d;
	char *e, *f;

	c = 0;
	f = a->s + a->n;
	for (e = f - b; e > a->s; --e) {
		if (*e != '/' && *e)
			continue;
		*e = '\0';
		if (stat(a->s, &d) == 0)
			break;
		if (errno != ENOENT) {
			warn("stat %s:", a->s);
			c = -1;
			break;
		}
	}
	if (e > a->s && e < f)
		*e = '/';
	while (++e <= f - b) {
		if (*e != '\0')
			continue;
		if (c == 0 && mkdir(a->s, 0777) < 0 && errno != EEXIST) {
			warn("mkdir %s:", a->s);
			c = -1;
		}
		if (e < f)
			*e = '/';
	}

	return c;
}

int
writefile(const char *a, struct string *b)
{
	FILE *c;
	int d;

	c = fopen(a, "w");
	if (!c) {
		warn("open %s:", a);
		return -1;
	}
	d = 0;
	if (fwrite(b->s, 1, b->n, c) != b->n || fflush(c) != 0) {
		warn("write %s:", a);
		d = -1;
	}
	fclose(c);

	return d;
}