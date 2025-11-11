#include <uriparser/UriDefsConfig.h>
#if (!defined(URI_PASS_ANSI) && !defined(URI_PASS_UNICODE))
# ifdef URI_ENABLE_ANSI
#  define URI_PASS_ANSI 1
#  include "UriQuery.c"
#  undef URI_PASS_ANSI
# endif
# ifdef URI_ENABLE_UNICODE
#  define URI_PASS_UNICODE 1
#  include "UriQuery.c"
#  undef URI_PASS_UNICODE
# endif
#else
# ifdef URI_PASS_ANSI
#  include <uriparser/UriDefsAnsi.h>
# else
#  include <uriparser/UriDefsUnicode.h>
#  include <wchar.h>
# endif

#ifndef URI_DOXYGEN
# include <uriparser/Uri.h>
# include "UriCommon.h"
#endif

static int URI_FUNC(ComposeQueryEngine)(URI_CHAR * a,
		const URI_TYPE(QueryList) * b,
		int c, int * d, int * e,
		UriBool f, UriBool g);

static UriBool URI_FUNC(AppendQueryItem)(URI_TYPE(QueryList) ** a,
		int * b, const URI_CHAR * c, const URI_CHAR * d,
		const URI_CHAR * e, const URI_CHAR * f,
		UriBool g, UriBreakConversion h);

int URI_FUNC(ComposeQueryCharsRequired)(const URI_TYPE(QueryList) * a,
		int * b) {
	return URI_FUNC(ComposeQueryCharsRequiredEx)(a, b, URI_TRUE, URI_TRUE);
}

int URI_FUNC(ComposeQueryCharsRequiredEx)(const URI_TYPE(QueryList) * a,
		int * b, UriBool c, UriBool d) {
	if ((a == NULL) || (b == NULL)) {
		return URI_ERROR_NULL;
	}

	return URI_FUNC(ComposeQueryEngine)(NULL, a, 0, NULL,
			b, c, d);
}

int URI_FUNC(ComposeQuery)(URI_CHAR * a,
						   const URI_TYPE(QueryList) * b, int c, int * d) {
	return URI_FUNC(ComposeQueryEx)(a, b, c, d, URI_TRUE, URI_TRUE);
}

int URI_FUNC(ComposeQueryEx)(URI_CHAR * a,
		const URI_TYPE(QueryList) * b, int c, int * d,
		UriBool e, UriBool f) {
	if ((a == NULL) || (b == NULL)) {
		return URI_ERROR_NULL;
	}

	if (c < 1) {
		return URI_ERROR_OUTPUT_TOO_LARGE;
	}

	return URI_FUNC(ComposeQueryEngine)(a, b, c,
			d, NULL, e, f);
}

int URI_FUNC(ComposeQueryMalloc)(URI_CHAR ** a,
		const URI_TYPE(QueryList) * b) {
	return URI_FUNC(ComposeQueryMallocEx)(a, b, URI_TRUE, URI_TRUE);
}

int URI_FUNC(ComposeQueryMallocEx)(URI_CHAR ** a,
		const URI_TYPE(QueryList) * b,
		UriBool c, UriBool d) {
	int e;
	int f;
	URI_CHAR * g;

	if (a == NULL) {
		return URI_ERROR_NULL;
	}

	f = URI_FUNC(ComposeQueryCharsRequiredEx)(b, &e, c, d);
	if (f != URI_SUCCESS) {
		return f;
	}
	e++;

	g = malloc(e * sizeof(URI_CHAR));
	if (g == NULL) {
		return URI_ERROR_MALLOC;
	}

	f = URI_FUNC(ComposeQueryEx)(g, b, e, NULL, c, d);
	if (f != URI_SUCCESS) {
		free(g);
		return f;
	}

	*a = g;
	return URI_SUCCESS;
}

int URI_FUNC(ComposeQueryEngine)(URI_CHAR * a,
		const URI_TYPE(QueryList) * b,
		int c, int * d, int * e,
		UriBool f, UriBool g) {
	UriBool h = URI_TRUE;
	int i = 0;
	URI_CHAR * j = a;

	if (a == NULL) {
		*e = 0;
	} else {
		c--;
	}

	while (b != NULL) {
		const URI_CHAR * const k = b->key;
		const URI_CHAR * const l = b->value;
		const int m = (g == URI_TRUE ? 6 : 3);
		const int n = (k == NULL) ? 0 : (int)URI_STRLEN(k);
		const int o = m * n;
		const int p = (l == NULL) ? 0 : (int)URI_STRLEN(l);
		const int q = m * p;

		if (a == NULL) {
			(*e) += i + o + ((l == NULL) ? 0 : 1 + q);

			if (h == URI_TRUE) {
				i = 1;
				h = URI_FALSE;
			}
		} else {
			if ((j - a) + i + o > c) {
				return URI_ERROR_OUTPUT_TOO_LARGE;
			}

			if (h == URI_TRUE) {
				i = 1;
				h = URI_FALSE;
			} else {
				j[0] = _UT('&');
				j++;
			}
			j = URI_FUNC(EscapeEx)(k, k + n, j, f, g);

			if (l != NULL) {
				if ((j - a) + 1 + q > c) {
					return URI_ERROR_OUTPUT_TOO_LARGE;
				}

				j[0] = _UT('=');
				j++;
				j = URI_FUNC(EscapeEx)(l, l + p, j, f, g);
			}
		}

		b = b->next;
	}

	if (a != NULL) {
		j[0] = _UT('\0');
		if (d != NULL) {
			*d = (int)(j - a) + 1;
		}
	}

	return URI_SUCCESS;
}

UriBool URI_FUNC(AppendQueryItem)(URI_TYPE(QueryList) ** a,
		int * b, const URI_CHAR * c, const URI_CHAR * d,
		const URI_CHAR * e, const URI_CHAR * f,
		UriBool g, UriBreakConversion h) {
	const int i = (int)(d - c);
	const int j = (int)(f - e);
	URI_CHAR * k;
	URI_CHAR * l;

	if ((a == NULL) || (b == NULL)
			|| (c == NULL) || (d == NULL)
			|| (c > d) || (e > f)
			|| ((c == d) && (e == NULL) && (f == NULL))) {
		return URI_TRUE;
	}

	*a = malloc(1 * sizeof(URI_TYPE(QueryList)));
	if (*a == NULL) {
		return URI_FALSE;
	}
	(*a)->next = NULL;

	k = malloc((i + 1) * sizeof(URI_CHAR));
	if (k == NULL) {
		free(*a);
		*a = NULL;
		return URI_FALSE;
	}

	k[i] = _UT('\0');
	if (i > 0) {
		memcpy(k, c, i * sizeof(URI_CHAR));
		URI_FUNC(UnescapeInPlaceEx)(k, g, h);
	}
	(*a)->key = k;

	if (e != NULL) {
		l = malloc((j + 1) * sizeof(URI_CHAR));
		if (l == NULL) {
			free(k);
			free(*a);
			*a = NULL;
			return URI_FALSE;
		}

		l[j] = _UT('\0');
		if (j > 0) {
			memcpy(l, e, j * sizeof(URI_CHAR));
			URI_FUNC(UnescapeInPlaceEx)(l, g, h);
		}
		(*a)->value = l;
	} else {
		l = NULL;
	}
	(*a)->value = l;

	(*b)++;
	return URI_TRUE;
}

void URI_FUNC(FreeQueryList)(URI_TYPE(QueryList) * a) {
	while (a != NULL) {
		URI_TYPE(QueryList) * b = a->next;
		free((URI_CHAR *)a->key);
		free((URI_CHAR *)a->value);
		free(a);
		a = b;
	}
}

int URI_FUNC(DissectQueryMalloc)(URI_TYPE(QueryList) ** a, int * b,
		const URI_CHAR * c, const URI_CHAR * d) {
	return URI_FUNC(DissectQueryMallocEx)(a, b, c, d, URI_TRUE, URI_BR_DONT_TOUCH);
}

int URI_FUNC(DissectQueryMallocEx)(URI_TYPE(QueryList) ** a, int * b,
		const URI_CHAR * c, const URI_CHAR * d,
		UriBool e, UriBreakConversion f) {
	const URI_CHAR * g = c;
	const URI_CHAR * h = c;
	const URI_CHAR * i = NULL;
	const URI_CHAR * j = NULL;
	const URI_CHAR * k = NULL;
	URI_TYPE(QueryList) ** l = a;
	int m;
	int * n = (b == NULL) ? &m : b;

	if ((a == NULL) || (c == NULL) || (d == NULL)) {
		return URI_ERROR_NULL;
	}

	if (c > d) {
		return URI_ERROR_RANGE_INVALID;
	}

	*a = NULL;
	*n = 0;

	for (; g < d; g++) {
		switch (*g) {
		case _UT('&'):
			if (k != NULL) {
				j = g;
			} else {
				i = g;
			}

			if (URI_FUNC(AppendQueryItem)(l, n, h, i, k, j, e, f) == URI_FALSE) {
				*n = 0;
				URI_FUNC(FreeQueryList)(*a);
				return URI_ERROR_MALLOC;
			}

			if ((l != NULL) && (*l != NULL)) {
				l = &((*l)->next);
			}

			if (g + 1 < d) {
				h = g + 1;
			} else {
				h = NULL;
			}
			i = NULL;
			k = NULL;
			j = NULL;
			break;

		case _UT('='):
			if (i == NULL) {
				i = g;
				if (g + 1 <= d) {
					k = g + 1;
					j = g + 1;
				}
			}
			break;

		default:
			break;
		}
	}

	if (k != NULL) {
		j = g;
	} else {
		i = g;
	}

	if (URI_FUNC(AppendQueryItem)(l, n, h, i, k, j, e, f) == URI_FALSE) {
		*n = 0;
		URI_FUNC(FreeQueryList)(*a);
		return URI_ERROR_MALLOC;
	}

	return URI_SUCCESS;
}

#endif