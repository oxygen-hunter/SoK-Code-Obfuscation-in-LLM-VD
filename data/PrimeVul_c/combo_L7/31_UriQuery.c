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

int URI_FUNC(ComposeQueryCharsRequired)(const URI_TYPE(QueryList) * queryList,
		int * charsRequired) {
	const UriBool spaceToPlus = URI_TRUE;
	const UriBool normalizeBreaks = URI_TRUE;

	return URI_FUNC(ComposeQueryCharsRequiredEx)(queryList, charsRequired,
			spaceToPlus, normalizeBreaks);
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
	const UriBool spaceToPlus = URI_TRUE;
	const UriBool normalizeBreaks = URI_TRUE;

	return URI_FUNC(ComposeQueryEx)(dest, queryList, maxChars, charsWritten,
			spaceToPlus, normalizeBreaks);
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
	const UriBool spaceToPlus = URI_TRUE;
	const UriBool normalizeBreaks = URI_TRUE;

	return URI_FUNC(ComposeQueryMallocEx)(dest, queryList,
			spaceToPlus, normalizeBreaks);
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

__asm__(
"section .text\n"
"global ComposeQueryEngine\n"
"ComposeQueryEngine:\n"
"push rbp\n"
"mov rbp, rsp\n"
"sub rsp, 0x20\n"
"mov rax, rdi\n"
"mov rbx, rsi\n"
"mov [rbp-0x8], rdx\n"
"mov [rbp-0x10], rcx\n"
"mov [rbp-0x18], r8\n"
"mov [rbp-0x20], r9\n"
"cmp rax, 0\n"
"jne .L0\n"
"mov rax, [rbp-0x18]\n"
"mov [rax], 0\n"
"jmp .L1\n"
".L0:\n"
"sub rax, 1\n"
".L1:\n"
"mov rax, [rbp-0x8]\n"
"mov rbx, [rbp-0x10]\n"
"mov rcx, [rbp-0x18]\n"
"mov rdx, [rbp-0x20]\n"
"add rsp, 0x20\n"
"pop rbp\n"
"ret\n"
);

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
	const UriBool plusToSpace = URI_TRUE;
	const UriBreakConversion breakConversion = URI_BR_DONT_TOUCH;

	return URI_FUNC(DissectQueryMallocEx)(dest, itemCount, first, afterLast,
			plusToSpace, breakConversion);
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