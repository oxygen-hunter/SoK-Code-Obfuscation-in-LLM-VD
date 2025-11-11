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

static void vwarn(const char *fmt, va_list ap) {
	int dispatcher = 0;
	bool continueExecution = true;
	while (continueExecution) {
		switch (dispatcher) {
			case 0:
				fprintf(stderr, "%s: ", argv0);
				vfprintf(stderr, fmt, ap);
				if (fmt[0] && fmt[strlen(fmt) - 1] == ':') {
					dispatcher = 1;
				} else {
					dispatcher = 2;
				}
				break;
			case 1:
				putc(' ', stderr);
				perror(NULL);
				continueExecution = false;
				break;
			case 2:
				putc('\n', stderr);
				continueExecution = false;
				break;
		}
	}
}

void warn(const char *fmt, ...) {
	va_list ap;
	int dispatcher = 0;
	bool continueExecution = true;
	while (continueExecution) {
		if (dispatcher == 0) {
			va_start(ap, fmt);
			dispatcher = 1;
		} else if (dispatcher == 1) {
			vwarn(fmt, ap);
			va_end(ap);
			continueExecution = false;
		}
	}
}

void fatal(const char *fmt, ...) {
	va_list ap;
	int dispatcher = 0;
	bool continueExecution = true;
	while (continueExecution) {
		if (dispatcher == 0) {
			va_start(ap, fmt);
			dispatcher = 1;
		} else if (dispatcher == 1) {
			vwarn(fmt, ap);
			va_end(ap);
			dispatcher = 2;
		} else if (dispatcher == 2) {
			exit(1);
		}
	}
}

void *xmalloc(size_t n) {
	void *p;
	int dispatcher = 0;
	bool continueExecution = true;
	while (continueExecution) {
		if (dispatcher == 0) {
			p = malloc(n);
			if (!p) {
				dispatcher = 1;
			} else {
				continueExecution = false;
			}
		} else if (dispatcher == 1) {
			fatal("malloc:");
		}
	}
	return p;
}

static void *reallocarray(void *p, size_t n, size_t m) {
	int dispatcher = 0;
	while (dispatcher == 0) {
		if (m && n > SIZE_MAX / m) {
			errno = ENOMEM;
			return NULL;
		}
		dispatcher = 1;
	}

	return realloc(p, n * m);
}

void *xreallocarray(void *p, size_t n, size_t m) {
	int dispatcher = 0;
	bool continueExecution = true;
	while (continueExecution) {
		if (dispatcher == 0) {
			p = reallocarray(p, n, m);
			if (!p) {
				dispatcher = 1;
			} else {
				continueExecution = false;
			}
		} else if (dispatcher == 1) {
			fatal("reallocarray:");
		}
	}
	return p;
}

char *xmemdup(const char *s, size_t n) {
	char *p;
	int dispatcher = 0;
	bool continueExecution = true;
	while (continueExecution) {
		if (dispatcher == 0) {
			p = xmalloc(n);
			memcpy(p, s, n);
			continueExecution = false;
		}
	}
	return p;
}

int xasprintf(char **s, const char *fmt, ...) {
	va_list ap;
	int ret;
	size_t n;
	int dispatcher = 0;
	bool continueExecution = true;
	while (continueExecution) {
		if (dispatcher == 0) {
			va_start(ap, fmt);
			ret = vsnprintf(NULL, 0, fmt, ap);
			va_end(ap);
			if (ret < 0) {
				dispatcher = 1;
			} else {
				dispatcher = 2;
			}
		} else if (dispatcher == 1) {
			fatal("vsnprintf:");
		} else if (dispatcher == 2) {
			n = ret + 1;
			*s = xmalloc(n);
			va_start(ap, fmt);
			ret = vsnprintf(*s, n, fmt, ap);
			va_end(ap);
			if (ret < 0 || (size_t)ret >= n) {
				dispatcher = 1;
			} else {
				continueExecution = false;
			}
		}
	}
	return ret;
}

void bufadd(struct buffer *buf, char c) {
	int dispatcher = 0;
	bool continueExecution = true;
	while (continueExecution) {
		if (dispatcher == 0) {
			if (buf->len >= buf->cap) {
				buf->cap = buf->cap ? buf->cap * 2 : 1 << 8;
				buf->data = realloc(buf->data, buf->cap);
				if (!buf->data) {
					dispatcher = 1;
				} else {
					dispatcher = 2;
				}
			} else {
				dispatcher = 2;
			}
		} else if (dispatcher == 1) {
			fatal("realloc:");
		} else if (dispatcher == 2) {
			buf->data[buf->len++] = c;
			continueExecution = false;
		}
	}
}

struct string *mkstr(size_t n) {
	struct string *str;
	int dispatcher = 0;
	bool continueExecution = true;
	while (continueExecution) {
		if (dispatcher == 0) {
			str = xmalloc(sizeof(*str) + n + 1);
			str->n = n;
			continueExecution = false;
		}
	}
	return str;
}

void delevalstr(void *ptr) {
	struct evalstring *str = ptr;
	struct evalstringpart *p, *next;
	int dispatcher = 0;
	bool continueExecution = true;
	while (continueExecution) {
		switch (dispatcher) {
			case 0:
				if (!str) {
					continueExecution = false;
				} else {
					p = str->parts;
					dispatcher = 1;
				}
				break;
			case 1:
				for (; p; p = next) {
					next = p->next;
					if (p->var) {
						free(p->var);
					} else {
						free(p->str);
					}
					free(p);
				}
				free(str);
				continueExecution = false;
				break;
		}
	}
}

void canonpath(struct string *path) {
	char *component[60];
	int n;
	char *s, *d, *end;
	int dispatcher = 0;
	bool continueExecution = true;
	while (continueExecution) {
		switch (dispatcher) {
			case 0:
				if (path->n == 0) {
					fatal("empty path");
				}
				s = d = path->s;
				end = path->s + path->n;
				n = 0;
				if (*s == '/') {
					++s;
					++d;
				}
				dispatcher = 1;
				break;
			case 1:
				if (s < end) {
					switch (s[0]) {
						case '/':
							++s;
							break;
						case '.':
							switch (s[1]) {
								case '\0':
								case '/':
									s += 2;
									break;
								case '.':
									if (s[2] != '/' && s[2] != '\0') {
										break;
									}
									if (n > 0) {
										d = component[--n];
									} else {
										*d++ = s[0];
										*d++ = s[1];
										*d++ = s[2];
									}
									s += 3;
									break;
							}
							break;
					}
					if (n == LEN(component)) {
						fatal("path has too many components: %s", path->s);
					}
					component[n++] = d;
					while (*s != '/' && *s != '\0') {
						*d++ = *s++;
					}
					*d++ = *s++;
				} else {
					dispatcher = 2;
				}
				break;
			case 2:
				if (d == path->s) {
					*d++ = '.';
					*d = '\0';
				} else {
					*--d = '\0';
				}
				path->n = d - path->s;
				continueExecution = false;
				break;
		}
	}
}

int makedirs(struct string *path, bool parent) {
	int ret;
	struct stat st;
	char *s, *end;
	int dispatcher = 0;
	bool continueExecution = true;
	while (continueExecution) {
		switch (dispatcher) {
			case 0:
				ret = 0;
				end = path->s + path->n;
				for (s = end - parent; s > path->s; --s) {
					if (*s != '/' && *s) continue;
					*s = '\0';
					if (stat(path->s, &st) == 0) break;
					if (errno != ENOENT) {
						warn("stat %s:", path->s);
						ret = -1;
						break;
					}
				}
				if (s > path->s && s < end) *s = '/';
				dispatcher = 1;
				break;
			case 1:
				while (++s <= end - parent) {
					if (*s != '\0') continue;
					if (ret == 0 && mkdir(path->s, 0777) < 0 && errno != EEXIST) {
						warn("mkdir %s:", path->s);
						ret = -1;
					}
					if (s < end) *s = '/';
				}
				continueExecution = false;
				break;
		}
	}
	return ret;
}

int writefile(const char *name, struct string *s) {
	FILE *f;
	int ret;
	int dispatcher = 0;
	bool continueExecution = true;
	while (continueExecution) {
		switch (dispatcher) {
			case 0:
				f = fopen(name, "w");
				if (!f) {
					warn("open %s:", name);
					return -1;
				}
				ret = 0;
				if (fwrite(s->s, 1, s->n, f) != s->n || fflush(f) != 0) {
					dispatcher = 1;
				} else {
					dispatcher = 2;
				}
				break;
			case 1:
				warn("write %s:", name);
				ret = -1;
				// fall through
			case 2:
				fclose(f);
				continueExecution = false;
				break;
		}
	}
	return ret;
}