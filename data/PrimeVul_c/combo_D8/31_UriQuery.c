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

static int URI_FUNC(ComposeQueryEngine)(URI_CHAR * d,
		const URI_TYPE(QueryList) * ql,
		int mc, int * cw, int * cr,
		UriBool stp, UriBool nb);

static UriBool URI_FUNC(AppendQueryItem)(URI_TYPE(QueryList) ** pn,
		int * ic, const URI_CHAR * kf, const URI_CHAR * ka,
		const URI_CHAR * vf, const URI_CHAR * va,
		UriBool pts, UriBreakConversion bc);

static UriBool getSpaceToPlus() {
	return URI_TRUE;
}

static UriBool getNormalizeBreaks() {
	return URI_TRUE;
}

int URI_FUNC(ComposeQueryCharsRequired)(const URI_TYPE(QueryList) * ql,
		int * cr) {
	return URI_FUNC(ComposeQueryCharsRequiredEx)(ql, cr,
			getSpaceToPlus(), getNormalizeBreaks());
}

int URI_FUNC(ComposeQueryCharsRequiredEx)(const URI_TYPE(QueryList) * ql,
		int * cr, UriBool stp, UriBool nb) {
	if ((ql == NULL) || (cr == NULL)) {
		return URI_ERROR_NULL;
	}

	return URI_FUNC(ComposeQueryEngine)(NULL, ql, 0, NULL,
			cr, stp, nb);
}

int URI_FUNC(ComposeQuery)(URI_CHAR * d,
						   const URI_TYPE(QueryList) * ql, int mc, int * cw) {
	return URI_FUNC(ComposeQueryEx)(d, ql, mc, cw,
			getSpaceToPlus(), getNormalizeBreaks());
}

int URI_FUNC(ComposeQueryEx)(URI_CHAR * d,
		const URI_TYPE(QueryList) * ql, int mc, int * cw,
		UriBool stp, UriBool nb) {
	if ((d == NULL) || (ql == NULL)) {
		return URI_ERROR_NULL;
	}

	if (mc < 1) {
		return URI_ERROR_OUTPUT_TOO_LARGE;
	}

	return URI_FUNC(ComposeQueryEngine)(d, ql, mc,
			cw, NULL, stp, nb);
}

int URI_FUNC(ComposeQueryMalloc)(URI_CHAR ** d,
		const URI_TYPE(QueryList) * ql) {
	return URI_FUNC(ComposeQueryMallocEx)(d, ql,
			getSpaceToPlus(), getNormalizeBreaks());
}

int URI_FUNC(ComposeQueryMallocEx)(URI_CHAR ** d,
		const URI_TYPE(QueryList) * ql,
		UriBool stp, UriBool nb) {
	int charsReq;
	int rs;
	URI_CHAR * qs;

	if (d == NULL) {
		return URI_ERROR_NULL;
	}

	rs = URI_FUNC(ComposeQueryCharsRequiredEx)(ql, &charsReq,
			stp, nb);
	if (rs != URI_SUCCESS) {
		return rs;
	}
	charsReq++;

	qs = malloc(charsReq * sizeof(URI_CHAR));
	if (qs == NULL) {
		return URI_ERROR_MALLOC;
	}

	rs = URI_FUNC(ComposeQueryEx)(qs, ql, charsReq,
			NULL, stp, nb);
	if (rs != URI_SUCCESS) {
		free(qs);
		return rs;
	}

	*d = qs;
	return URI_SUCCESS;
}

int URI_FUNC(ComposeQueryEngine)(URI_CHAR * d,
		const URI_TYPE(QueryList) * ql,
		int mc, int * cw, int * cr,
		UriBool stp, UriBool nb) {
	UriBool fi = URI_TRUE;
	int al = 0;
	URI_CHAR * w = d;

	if (d == NULL) {
		*cr = 0;
	} else {
		mc--;
	}

	while (ql != NULL) {
		const URI_CHAR * const k = ql->key;
		const URI_CHAR * const v = ql->value;
		const int wc = (nb == URI_TRUE ? 6 : 3);
		const int kl = (k == NULL) ? 0 : (int)URI_STRLEN(k);
		const int krc = wc * kl;
		const int vl = (v == NULL) ? 0 : (int)URI_STRLEN(v);
		const int vrc = wc * vl;

		if (d == NULL) {
			(*cr) += al + krc + ((v == NULL)
						? 0
						: 1 + vrc);

			if (fi == URI_TRUE) {
				al = 1;
				fi = URI_FALSE;
			}
		} else {
			if ((w - d) + al + krc > mc) {
				return URI_ERROR_OUTPUT_TOO_LARGE;
			}

			if (fi == URI_TRUE) {
				al = 1;
				fi = URI_FALSE;
			} else {
				w[0] = _UT('&');
				w++;
			}
			w = URI_FUNC(EscapeEx)(k, k + kl,
					w, stp, nb);

			if (v != NULL) {
				if ((w - d) + 1 + vrc > mc) {
					return URI_ERROR_OUTPUT_TOO_LARGE;
				}

				w[0] = _UT('=');
				w++;
				w = URI_FUNC(EscapeEx)(v, v + vl,
						w, stp, nb);
			}
		}

		ql = ql->next;
	}

	if (d != NULL) {
		w[0] = _UT('\0');
		if (cw != NULL) {
			*cw = (int)(w - d) + 1;
		}
	}

	return URI_SUCCESS;
}

UriBool URI_FUNC(AppendQueryItem)(URI_TYPE(QueryList) ** pn,
		int * ic, const URI_CHAR * kf, const URI_CHAR * ka,
		const URI_CHAR * vf, const URI_CHAR * va,
		UriBool pts, UriBreakConversion bc) {
	const int kl = (int)(ka - kf);
	const int vl = (int)(va - vf);
	URI_CHAR * k;
	URI_CHAR * v;

	if ((pn == NULL) || (ic == NULL)
			|| (kf == NULL) || (ka == NULL)
			|| (kf > ka) || (vf > va)
			|| ((kf == ka)
				&& (vf == NULL) && (va == NULL))) {
		return URI_TRUE;
	}

	*pn = malloc(1 * sizeof(URI_TYPE(QueryList)));
	if (*pn == NULL) {
		return URI_FALSE;
	}
	(*pn)->next = NULL;

	k = malloc((kl + 1) * sizeof(URI_CHAR));
	if (k == NULL) {
		free(*pn);
		*pn = NULL;
		return URI_FALSE;
	}

	k[kl] = _UT('\0');
	if (kl > 0) {
		memcpy(k, kf, kl * sizeof(URI_CHAR));
		URI_FUNC(UnescapeInPlaceEx)(k, pts, bc);
	}
	(*pn)->key = k;

	if (vf != NULL) {
		v = malloc((vl + 1) * sizeof(URI_CHAR));
		if (v == NULL) {
			free(k);
			free(*pn);
			*pn = NULL;
			return URI_FALSE;
		}

		v[vl] = _UT('\0');
		if (vl > 0) {
			memcpy(v, vf, vl * sizeof(URI_CHAR));
			URI_FUNC(UnescapeInPlaceEx)(v, pts, bc);
		}
		(*pn)->value = v;
	} else {
		v = NULL;
	}
	(*pn)->value = v;

	(*ic)++;
	return URI_TRUE;
}

void URI_FUNC(FreeQueryList)(URI_TYPE(QueryList) * ql) {
	while (ql != NULL) {
		URI_TYPE(QueryList) * nb = ql->next;
		free((URI_CHAR *)ql->key);
		free((URI_CHAR *)ql->value);
		free(ql);
		ql = nb;
	}
}

int URI_FUNC(DissectQueryMalloc)(URI_TYPE(QueryList) ** d, int * ic,
		const URI_CHAR * f, const URI_CHAR * al) {
	return URI_FUNC(DissectQueryMallocEx)(d, ic, f, al,
			getSpaceToPlus(), URI_BR_DONT_TOUCH);
}

int URI_FUNC(DissectQueryMallocEx)(URI_TYPE(QueryList) ** d, int * ic,
		const URI_CHAR * f, const URI_CHAR * al,
		UriBool pts, UriBreakConversion bc) {
	const URI_CHAR * w = f;
	const URI_CHAR * kf = f;
	const URI_CHAR * ka = NULL;
	const URI_CHAR * vf = NULL;
	const URI_CHAR * va = NULL;
	URI_TYPE(QueryList) ** pn = d;
	int nc;
	int * ia = (ic == NULL) ? &nc : ic;

	if ((d == NULL) || (f == NULL) || (al == NULL)) {
		return URI_ERROR_NULL;
	}

	if (f > al) {
		return URI_ERROR_RANGE_INVALID;
	}

	*d = NULL;
	*ia = 0;

	for (; w < al; w++) {
		switch (*w) {
		case _UT('&'):
			if (vf != NULL) {
				va = w;
			} else {
				ka = w;
			}

			if (URI_FUNC(AppendQueryItem)(pn, ia,
					kf, ka, vf, va,
					pts, bc)
					== URI_FALSE) {
				*ia = 0;
				URI_FUNC(FreeQueryList)(*d);
				return URI_ERROR_MALLOC;
			}

			if ((pn != NULL) && (*pn != NULL)) {
				pn = &((*pn)->next);
			}

			if (w + 1 < al) {
				kf = w + 1;
			} else {
				kf = NULL;
			}
			ka = NULL;
			vf = NULL;
			va = NULL;
			break;

		case _UT('='):
			if (ka == NULL) {
				ka = w;
				if (w + 1 <= al) {
					vf = w + 1;
					va = w + 1;
				}
			}
			break;

		default:
			break;
		}
	}

	if (vf != NULL) {
		va = w;
	} else {
		ka = w;
	}

	if (URI_FUNC(AppendQueryItem)(pn, ia, kf, ka,
			vf, va, pts, bc)
			== URI_FALSE) {
		*ia = 0;
		URI_FUNC(FreeQueryList)(*d);
		return URI_ERROR_MALLOC;
	}

	return URI_SUCCESS;
}

#endif