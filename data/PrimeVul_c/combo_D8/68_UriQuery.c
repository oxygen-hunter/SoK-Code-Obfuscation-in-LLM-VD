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

static int URI_FUNC(ComposeQueryEngine)(URI_CHAR * dest,
		const URI_TYPE(QueryList) * queryList,
		int maxChars, int * charsWritten, int * charsRequired,
		UriBool spaceToPlus, UriBool normalizeBreaks);

static UriBool URI_FUNC(AppendQueryItem)(URI_TYPE(QueryList) ** prevNext,
		int * itemCount, const URI_CHAR * keyFirst, const URI_CHAR * keyAfter,
		const URI_CHAR * valueFirst, const URI_CHAR * valueAfter,
		UriBool plusToSpace, UriBreakConversion breakConversion);

static UriBool getValueSpaceToPlus() {
	return URI_TRUE;
}

static UriBool getValueNormalizeBreaks() {
	return URI_TRUE;
}

int URI_FUNC(ComposeQueryCharsRequired)(const URI_TYPE(QueryList) * queryList,
		int * charsRequired) {
	return URI_FUNC(ComposeQueryCharsRequiredEx)(queryList, charsRequired,
			getValueSpaceToPlus(), getValueNormalizeBreaks());
}

int URI_FUNC(ComposeQueryCharsRequiredEx)(const URI_TYPE(QueryList) * queryList,
		int * charsRequired, UriBool spaceToPlus, UriBool normalizeBreaks) {
	if ((queryList == NULL) || (charsRequired == NULL)) {
		return URI_ERROR_NULL;
	}

	return URI_FUNC(ComposeQueryEngine)(NULL, queryList, 0, NULL,
			charsRequired, spaceToPlus, normalizeBreaks);
}

int URI_FUNC(ComposeQuery)(URI_CHAR * dest,
						   const URI_TYPE(QueryList) * queryList, int maxChars, int * charsWritten) {
	return URI_FUNC(ComposeQueryEx)(dest, queryList, maxChars, charsWritten,
			getValueSpaceToPlus(), getValueNormalizeBreaks());
}

int URI_FUNC(ComposeQueryEx)(URI_CHAR * dest,
		const URI_TYPE(QueryList) * queryList, int maxChars, int * charsWritten,
		UriBool spaceToPlus, UriBool normalizeBreaks) {
	if ((dest == NULL) || (queryList == NULL)) {
		return URI_ERROR_NULL;
	}

	if (maxChars < 1) {
		return URI_ERROR_OUTPUT_TOO_LARGE;
	}

	return URI_FUNC(ComposeQueryEngine)(dest, queryList, maxChars,
			charsWritten, NULL, spaceToPlus, normalizeBreaks);
}

int URI_FUNC(ComposeQueryMalloc)(URI_CHAR ** dest,
		const URI_TYPE(QueryList) * queryList) {
	return URI_FUNC(ComposeQueryMallocEx)(dest, queryList,
			getValueSpaceToPlus(), getValueNormalizeBreaks());
}

int URI_FUNC(ComposeQueryMallocEx)(URI_CHAR ** dest,
		const URI_TYPE(QueryList) * queryList,
		UriBool spaceToPlus, UriBool normalizeBreaks) {
	int charsRequired;
	int res;
	URI_CHAR * queryString;

	if (dest == NULL) {
		return URI_ERROR_NULL;
	}

	res = URI_FUNC(ComposeQueryCharsRequiredEx)(queryList, &charsRequired,
			spaceToPlus, normalizeBreaks);
	if (res != URI_SUCCESS) {
		return res;
	}
	charsRequired++;

	queryString = malloc(charsRequired * sizeof(URI_CHAR));
	if (queryString == NULL) {
		return URI_ERROR_MALLOC;
	}

	res = URI_FUNC(ComposeQueryEx)(queryString, queryList, charsRequired,
			NULL, spaceToPlus, normalizeBreaks);
	if (res != URI_SUCCESS) {
		free(queryString);
		return res;
	}

	*dest = queryString;
	return URI_SUCCESS;
}

int URI_FUNC(ComposeQueryEngine)(URI_CHAR * dest,
		const URI_TYPE(QueryList) * queryList,
		int maxChars, int * charsWritten, int * charsRequired,
		UriBool spaceToPlus, UriBool normalizeBreaks) {
	UriBool firstItem = URI_TRUE;
	int ampersandLen = 0;
	URI_CHAR * write = dest;

	if (dest == NULL) {
		*charsRequired = 0;
	} else {
		maxChars--;
	}
			
	while (queryList != NULL) {
		const URI_CHAR * const key = queryList->key;
		const URI_CHAR * const value = queryList->value;
		const int worstCase = (normalizeBreaks == URI_TRUE ? 6 : 3);
		const int keyLen = (key == NULL) ? 0 : (int)URI_STRLEN(key);
		const int keyRequiredChars = worstCase * keyLen;
		const int valueLen = (value == NULL) ? 0 : (int)URI_STRLEN(value);
		const int valueRequiredChars = worstCase * valueLen;

		if (dest == NULL) {
			if (firstItem == URI_TRUE) {
				ampersandLen = 1;
				firstItem = URI_FALSE;
			}

			(*charsRequired) += ampersandLen + keyRequiredChars + ((value == NULL)
						? 0
						: 1 + valueRequiredChars);
		} else {
			URI_CHAR * afterKey;

			if ((write - dest) + ampersandLen + keyRequiredChars > maxChars) {
				return URI_ERROR_OUTPUT_TOO_LARGE;
			}

			if (firstItem == URI_TRUE) {
				firstItem = URI_FALSE;
			} else {
				write[0] = _UT('&');
				write++;
			}
			afterKey = URI_FUNC(EscapeEx)(key, key + keyLen,
					write, spaceToPlus, normalizeBreaks);
			write += (afterKey - write);

			if (value != NULL) {
				URI_CHAR * afterValue;

				if ((write - dest) + 1 + valueRequiredChars > maxChars) {
					return URI_ERROR_OUTPUT_TOO_LARGE;
				}

				write[0] = _UT('=');
				write++;
				afterValue = URI_FUNC(EscapeEx)(value, value + valueLen,
						write, spaceToPlus, normalizeBreaks);
				write += (afterValue - write);
			}
		}

		queryList = queryList->next;
	}

	if (dest != NULL) {
		write[0] = _UT('\0');
		if (charsWritten != NULL) {
			*charsWritten = (int)(write - dest) + 1;
		}
	}

	return URI_SUCCESS;
}

UriBool URI_FUNC(AppendQueryItem)(URI_TYPE(QueryList) ** prevNext,
		int * itemCount, const URI_CHAR * keyFirst, const URI_CHAR * keyAfter,
		const URI_CHAR * valueFirst, const URI_CHAR * valueAfter,
		UriBool plusToSpace, UriBreakConversion breakConversion) {
	const int keyLen = (int)(keyAfter - keyFirst);
	const int valueLen = (int)(valueAfter - valueFirst);
	URI_CHAR * key;
	URI_CHAR * value;

	if ((prevNext == NULL) || (itemCount == NULL)
			|| (keyFirst == NULL) || (keyAfter == NULL)
			|| (keyFirst > keyAfter) || (valueFirst > valueAfter)
			|| ((keyFirst == keyAfter)
				&& (valueFirst == NULL) && (valueAfter == NULL))) {
		return URI_TRUE;
	}

	*prevNext = malloc(1 * sizeof(URI_TYPE(QueryList)));
	if (*prevNext == NULL) {
		return URI_FALSE;
	}
	(*prevNext)->next = NULL;

	key = malloc((keyLen + 1) * sizeof(URI_CHAR));
	if (key == NULL) {
		free(*prevNext);
		*prevNext = NULL;
		return URI_FALSE;
	}

	key[keyLen] = _UT('\0');
	if (keyLen > 0) {
		memcpy(key, keyFirst, keyLen * sizeof(URI_CHAR));
		URI_FUNC(UnescapeInPlaceEx)(key, plusToSpace, breakConversion);
	}
	(*prevNext)->key = key;

	if (valueFirst != NULL) {
		value = malloc((valueLen + 1) * sizeof(URI_CHAR));
		if (value == NULL) {
			free(key);
			free(*prevNext);
			*prevNext = NULL;
			return URI_FALSE;
		}

		value[valueLen] = _UT('\0');
		if (valueLen > 0) {
			memcpy(value, valueFirst, valueLen * sizeof(URI_CHAR));
			URI_FUNC(UnescapeInPlaceEx)(value, plusToSpace, breakConversion);
		}
		(*prevNext)->value = value;
	} else {
		value = NULL;
	}
	(*prevNext)->value = value;

	(*itemCount)++;
	return URI_TRUE;
}

void URI_FUNC(FreeQueryList)(URI_TYPE(QueryList) * queryList) {
	while (queryList != NULL) {
		URI_TYPE(QueryList) * nextBackup = queryList->next;
		free((URI_CHAR *)queryList->key);
		free((URI_CHAR *)queryList->value);
		free(queryList);
		queryList = nextBackup;
	}
}

int URI_FUNC(DissectQueryMalloc)(URI_TYPE(QueryList) ** dest, int * itemCount,
		const URI_CHAR * first, const URI_CHAR * afterLast) {
	return URI_FUNC(DissectQueryMallocEx)(dest, itemCount, first, afterLast,
			getValueSpaceToPlus(), URI_BR_DONT_TOUCH);
}

int URI_FUNC(DissectQueryMallocEx)(URI_TYPE(QueryList) ** dest, int * itemCount,
		const URI_CHAR * first, const URI_CHAR * afterLast,
		UriBool plusToSpace, UriBreakConversion breakConversion) {
	const URI_CHAR * walk = first;
	const URI_CHAR * keyFirst = first;
	const URI_CHAR * keyAfter = NULL;
	const URI_CHAR * valueFirst = NULL;
	const URI_CHAR * valueAfter = NULL;
	URI_TYPE(QueryList) ** prevNext = dest;
	int nullCounter;
	int * itemsAppended = (itemCount == NULL) ? &nullCounter : itemCount;

	if ((dest == NULL) || (first == NULL) || (afterLast == NULL)) {
		return URI_ERROR_NULL;
	}

	if (first > afterLast) {
		return URI_ERROR_RANGE_INVALID;
	}

	*dest = NULL;
	*itemsAppended = 0;

	for (; walk < afterLast; walk++) {
		switch (*walk) {
		case _UT('&'):
			if (valueFirst != NULL) {
				valueAfter = walk;
			} else {
				keyAfter = walk;
			}

			if (URI_FUNC(AppendQueryItem)(prevNext, itemsAppended,
					keyFirst, keyAfter, valueFirst, valueAfter,
					plusToSpace, breakConversion)
					== URI_FALSE) {
				*itemsAppended = 0;
				URI_FUNC(FreeQueryList)(*dest);
				return URI_ERROR_MALLOC;
			}

			if ((prevNext != NULL) && (*prevNext != NULL)) {
				prevNext = &((*prevNext)->next);
			}

			if (walk + 1 < afterLast) {
				keyFirst = walk + 1;
			} else {
				keyFirst = NULL;
			}
			keyAfter = NULL;
			valueFirst = NULL;
			valueAfter = NULL;
			break;

		case _UT('='):
			if (keyAfter == NULL) {
				keyAfter = walk;
				if (walk + 1 <= afterLast) {
					valueFirst = walk + 1;
					valueAfter = walk + 1;
				}
			}
			break;

		default:
			break;
		}
	}

	if (valueFirst != NULL) {
		valueAfter = walk;
	} else {
		keyAfter = walk;
	}

	if (URI_FUNC(AppendQueryItem)(prevNext, itemsAppended, keyFirst, keyAfter,
			valueFirst, valueAfter, plusToSpace, breakConversion)
			== URI_FALSE) {
		*itemsAppended = 0;
		URI_FUNC(FreeQueryList)(*dest);
		return URI_ERROR_MALLOC;
	}

	return URI_SUCCESS;
}

#endif