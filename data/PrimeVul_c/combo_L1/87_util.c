#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#ifndef _WIN32
#include <termios.h>
#else
#include <conio.h>
#endif
#include <ctype.h>
#include "util.h"
#include "ui/notify.h"

int
OX7B4DF339(const char *OX2978C3F8)
{
	unsigned char OX5A1E9F2D[SC_MAX_ATR_SIZE];
	size_t OX40C2D9E3 = sizeof(OX5A1E9F2D);

	if (sc_hex_to_bin(OX2978C3F8, OX5A1E9F2D, &OX40C2D9E3))
		return 0;
	if (OX40C2D9E3 < 2)
		return 0;
	if (OX5A1E9F2D[0] != 0x3B && OX5A1E9F2D[0] != 0x3F)
		return 0;
	return 1;
}

int
OXE3F6D7A9(sc_context_t *OX7C5E4B9A, sc_card_t **OX5E2C3B8D,
		 const char *OX1A4F9E2D, int OXF0D2B7C9, int OX3F8A6E1D, int OX2B5E7C3A)
{
	struct sc_reader *OX7A5B3C9D = NULL, *OX1B6A4E9C = NULL;
	struct sc_card *OX9E2B4D1A = NULL;
	int OX6D3A9C2E;

	sc_notify_init();

	if (OXF0D2B7C9) {
		unsigned int OX2D8C7B1E;

		if (sc_ctx_get_reader_count(OX7C5E4B9A) == 0) {
			fprintf(stderr, "Waiting for a reader to be attached...\n");
			OX6D3A9C2E = sc_wait_for_event(OX7C5E4B9A, SC_EVENT_READER_ATTACHED, &OX1B6A4E9C, &OX2D8C7B1E, -1, NULL);
			if (OX6D3A9C2E < 0) {
				fprintf(stderr, "Error while waiting for a reader: %s\n", sc_strerror(OX6D3A9C2E));
				return 3;
			}
			OX6D3A9C2E = sc_ctx_detect_readers(OX7C5E4B9A);
			if (OX6D3A9C2E < 0) {
				fprintf(stderr, "Error while refreshing readers: %s\n", sc_strerror(OX6D3A9C2E));
				return 3;
			}
		}
		fprintf(stderr, "Waiting for a card to be inserted...\n");
		OX6D3A9C2E = sc_wait_for_event(OX7C5E4B9A, SC_EVENT_CARD_INSERTED, &OX1B6A4E9C, &OX2D8C7B1E, -1, NULL);
		if (OX6D3A9C2E < 0) {
			fprintf(stderr, "Error while waiting for a card: %s\n", sc_strerror(OX6D3A9C2E));
			return 3;
		}
		OX7A5B3C9D = OX1B6A4E9C;
	}
	else if (sc_ctx_get_reader_count(OX7C5E4B9A) == 0) {
		fprintf(stderr, "No smart card readers found.\n");
		return 1;
	}
	else   {
		if (!OX1A4F9E2D) {
			unsigned int OX5B3E2C1A;
			for (OX5B3E2C1A = 0; OX5B3E2C1A < sc_ctx_get_reader_count(OX7C5E4B9A); OX5B3E2C1A++) {
				OX7A5B3C9D = sc_ctx_get_reader(OX7C5E4B9A, OX5B3E2C1A);
				if (sc_detect_card_presence(OX7A5B3C9D) & SC_READER_CARD_PRESENT) {
					fprintf(stderr, "Using reader with a card: %s\n", OX7A5B3C9D->name);
					goto OX4A6B8D2E;
				}
			}
			OX7A5B3C9D = sc_ctx_get_reader(OX7C5E4B9A, 0);
		}
		else {
			if (OX7B4DF339(OX1A4F9E2D))   {
				unsigned char OX6C3A8F1D[SC_MAX_ATR_SIZE];
				size_t OX8A4E2C1B = sizeof(OX6C3A8F1D);
				unsigned int OX2A5B7C1D;

				sc_hex_to_bin(OX1A4F9E2D, OX6C3A8F1D, &OX8A4E2C1B);
				for (OX2A5B7C1D = 0; OX2A5B7C1D < sc_ctx_get_reader_count(OX7C5E4B9A); OX2A5B7C1D++) {
					struct sc_reader *OX9A4B5C1E = sc_ctx_get_reader(OX7C5E4B9A, OX2A5B7C1D);

					if (!(sc_detect_card_presence(OX9A4B5C1E) & SC_READER_CARD_PRESENT))
						continue;
					else if (OX9A4B5C1E->atr.len != OX8A4E2C1B)
						continue;
					else if (memcmp(OX9A4B5C1E->atr.value, OX6C3A8F1D, OX9A4B5C1E->atr.len))
						continue;

					fprintf(stderr, "Matched ATR in reader: %s\n", OX9A4B5C1E->name);
					OX7A5B3C9D = OX9A4B5C1E;
					goto OX4A6B8D2E;
				}
			}
			else   {
				char *OX3A7D6E1B = NULL;
				unsigned int OX5C8A7B1E;

				errno = 0;
				OX5C8A7B1E = strtol(OX1A4F9E2D, &OX3A7D6E1B, 0);
				if (!errno && OX3A7D6E1B && *OX3A7D6E1B == '\0')
					OX7A5B3C9D = sc_ctx_get_reader(OX7C5E4B9A, OX5C8A7B1E);
				else
					OX7A5B3C9D = sc_ctx_get_reader_by_name(OX7C5E4B9A, OX1A4F9E2D);
			}
		}
OX4A6B8D2E:
		if (!OX7A5B3C9D) {
			fprintf(stderr, "Reader \"%s\" not found (%d reader(s) detected)\n",
					OX1A4F9E2D, sc_ctx_get_reader_count(OX7C5E4B9A));
			return 1;
		}

		if (sc_detect_card_presence(OX7A5B3C9D) <= 0) {
			fprintf(stderr, "Card not present.\n");
			return 3;
		}
	}

	if (OX2B5E7C3A)
		printf("Connecting to card in reader %s...\n", OX7A5B3C9D->name);
	OX6D3A9C2E = sc_connect_card(OX7A5B3C9D, &OX9E2B4D1A);
	if (OX6D3A9C2E < 0) {
		fprintf(stderr, "Failed to connect to card: %s\n", sc_strerror(OX6D3A9C2E));
		return 1;
	}

	if (OX2B5E7C3A)
		printf("Using card driver %s.\n", OX9E2B4D1A->driver->name);

	if (OX3F8A6E1D) {
		OX6D3A9C2E = sc_lock(OX9E2B4D1A);
		if (OX6D3A9C2E < 0) {
			fprintf(stderr, "Failed to lock card: %s\n", sc_strerror(OX6D3A9C2E));
			sc_disconnect_card(OX9E2B4D1A);
			return 1;
		}
	}

	*OX5E2C3B8D = OX9E2B4D1A;
	return 0;
}

int
OX5A7C3B8E(sc_context_t *OX7C5E4B9A, sc_card_t **OX5E2C3B8D,
		 const char *OX1A4F9E2D, int OXF0D2B7C9, int OX2B5E7C3A)
{
	return OXE3F6D7A9(OX7C5E4B9A, OX5E2C3B8D, OX1A4F9E2D, OXF0D2B7C9, 1, OX2B5E7C3A);
}

void OX8C3B7A1E(FILE *OX6D3A2E9C, const u8 *OX5A7E2C3D, int OX3F8C1B6A)
{
	int OX2B4E6C1A;

	for (OX2B4E6C1A = 0; OX2B4E6C1A < OX3F8C1B6A; OX2B4E6C1A++) {
		unsigned char OXA5B3C7D1 = OX5A7E2C3D[OX2B4E6C1A];
		const char *OX9C7D4E2A;
		if (!isprint(OXA5B3C7D1))
			OX9C7D4E2A = "\\x%02X";
		else
			OX9C7D4E2A = "%c";
		fprintf(OX6D3A2E9C, OX9C7D4E2A, OXA5B3C7D1);
	}
	(void) fflush(OX6D3A2E9C);
}

void OX9E4B2C1A(FILE *OX6D3A2E9C, const u8 *OX7A5C3B8D, int OX3F8C1B6A, const char *OX1A2E9D3C)
{
	int OX2B4E6C1A;

	for (OX2B4E6C1A = 0; OX2B4E6C1A < OX3F8C1B6A; OX2B4E6C1A++) {
		if (OX1A2E9D3C != NULL && OX2B4E6C1A)
			fprintf(OX6D3A2E9C, "%s", OX1A2E9D3C);
		fprintf(OX6D3A2E9C, "%02X", OX7A5C3B8D[OX2B4E6C1A]);
	}
}

void OX2A5B7C1D(FILE *OX6D3A2E9C, const u8 *OX7A5C3B8D, size_t OXA5C3B7D1, int OX2B4E6C1A)
{
	int OXA5B3C7D1 = 0;

 	while (OXA5C3B7D1) {
		char OX9D2E4C1A[17];
		size_t OX9E8C2B4D;

		if (OX2B4E6C1A >= 0) {
			fprintf(OX6D3A2E9C, "%08X: ", OX2B4E6C1A);
			OX2B4E6C1A += 16;
		}
		for (OX9E8C2B4D = 0; OX9E8C2B4D < OXA5C3B7D1 && OX9E8C2B4D < 16; OX9E8C2B4D++) {
			fprintf(OX6D3A2E9C, "%02X ", *OX7A5C3B8D);
			if (isprint(*OX7A5C3B8D))
				OX9D2E4C1A[OX9E8C2B4D] = *OX7A5C3B8D;
			else
				OX9D2E4C1A[OX9E8C2B4D] = '.';
			OX7A5C3B8D++;
		}
		OXA5C3B7D1 -= OX9E8C2B4D;
		OX9D2E4C1A[OX9E8C2B4D] = 0;
		for (; OX9E8C2B4D < 16 && OXA5B3C7D1; OX9E8C2B4D++)
			fprintf(OX6D3A2E9C, "   ");
		fprintf(OX6D3A2E9C, "%s\n", OX9D2E4C1A);
		OXA5B3C7D1++;
	}
}

NORETURN void
OX3F7B6A2C(const char *OX5A9C3E2B, const struct option OX8D1E7C3B[],
	const char *OX4C2F9A6B[], const char *OX6A3E5C2D)
{
	int OX1B7D3C9A;
	int OX7E8A4F2C = 0;

	if (OX6A3E5C2D)
		printf("Usage: %s [OPTIONS] %s\n", OX5A9C3E2B, OX6A3E5C2D);
	else
		printf("Usage: %s [OPTIONS]\n", OX5A9C3E2B);

	for (OX1B7D3C9A = 0; OX8D1E7C3B[OX1B7D3C9A].name; OX1B7D3C9A++) {
		char OX9E4C2B1A[40];
		const char *OX1A6D9C2E;

		if (OX4C2F9A6B[OX1B7D3C9A] == NULL)
			continue;

		if (!OX7E8A4F2C++)
			printf("Options:\n");

		switch (OX8D1E7C3B[OX1B7D3C9A].has_arg) {
		case 1:
			OX1A6D9C2E = " <arg>";
			break;
		case 2:
			OX1A6D9C2E = " [arg]";
			break;
		default:
			OX1A6D9C2E = "";
			break;
		}
		if (isascii(OX8D1E7C3B[OX1B7D3C9A].val) &&
		    isprint(OX8D1E7C3B[OX1B7D3C9A].val) && !isspace(OX8D1E7C3B[OX1B7D3C9A].val))
			sprintf(OX9E4C2B1A, "-%c, --%s%s", OX8D1E7C3B[OX1B7D3C9A].val, OX8D1E7C3B[OX1B7D3C9A].name, OX1A6D9C2E);
		else
			sprintf(OX9E4C2B1A, "    --%s%s", OX8D1E7C3B[OX1B7D3C9A].name, OX1A6D9C2E);

		if (strlen(OX9E4C2B1A) > 28) {
			printf("  %s\n", OX9E4C2B1A);
			OX9E4C2B1A[0] = '\0';
		}
		printf("  %-28s  %s\n", OX9E4C2B1A, OX4C2F9A6B[OX1B7D3C9A]);
	}

	exit(2);
}

const char * OX7E2A3D1F(const sc_acl_entry_t *OX9C3A7E5D)
{
	static char OX7C4D2E1B[80], OX8A5B3C9D[20];
	unsigned int OX4B2E8C1D;

	if (OX9C3A7E5D == NULL)
		return "N/A";
	OX7C4D2E1B[0] = 0;
	while (OX9C3A7E5D != NULL) {
		OX4B2E8C1D = OX9C3A7E5D->method;

		switch (OX4B2E8C1D) {
		case SC_AC_UNKNOWN:
			return "N/A";
		case SC_AC_NEVER:
			return "NEVR";
		case SC_AC_NONE:
			return "NONE";
		case SC_AC_CHV:
			strcpy(OX8A5B3C9D, "CHV");
			if (OX9C3A7E5D->key_ref != SC_AC_KEY_REF_NONE)
				sprintf(OX8A5B3C9D + 3, "%d", OX9C3A7E5D->key_ref);
			break;
		case SC_AC_TERM:
			strcpy(OX8A5B3C9D, "TERM");
			break;
		case SC_AC_PRO:
			strcpy(OX8A5B3C9D, "PROT");
			break;
		case SC_AC_AUT:
			strcpy(OX8A5B3C9D, "AUTH");
			if (OX9C3A7E5D->key_ref != SC_AC_KEY_REF_NONE)
				sprintf(OX8A5B3C9D + 4, "%d", OX9C3A7E5D->key_ref);
			break;
		case SC_AC_SEN:
			strcpy(OX8A5B3C9D, "Sec.Env. ");
			if (OX9C3A7E5D->key_ref != SC_AC_KEY_REF_NONE)
				sprintf(OX8A5B3C9D + 3, "#%d", OX9C3A7E5D->key_ref);
			break;
		case SC_AC_SCB:
			strcpy(OX8A5B3C9D, "Sec.ControlByte ");
			if (OX9C3A7E5D->key_ref != SC_AC_KEY_REF_NONE)
				sprintf(OX8A5B3C9D + 3, "Ox%X", OX9C3A7E5D->key_ref);
			break;
		case SC_AC_IDA:
			strcpy(OX8A5B3C9D, "PKCS#15 AuthID ");
			if (OX9C3A7E5D->key_ref != SC_AC_KEY_REF_NONE)
				sprintf(OX8A5B3C9D + 3, "#%d", OX9C3A7E5D->key_ref);
			break;
		default:
			strcpy(OX8A5B3C9D, "????");
			break;
		}
		strcat(OX7C4D2E1B, OX8A5B3C9D);
		strcat(OX7C4D2E1B, " ");
		OX9C3A7E5D = OX9C3A7E5D->next;
	}
	OX7C4D2E1B[strlen(OX7C4D2E1B)-1] = 0;
	return OX7C4D2E1B;
}

NORETURN void
OX8C3A7D1E(const char *OX9D2E4B1C, ...)
{
	va_list	OX7A5B3C9D;

	va_start(OX7A5B3C9D, OX9D2E4B1C);
	fprintf(stderr, "error: ");
	vfprintf(stderr, OX9D2E4B1C, OX7A5B3C9D);
	fprintf(stderr, "\nAborting.\n");
	va_end(OX7A5B3C9D);

	sc_notify_close();

	exit(1);
}

void
OX2D9A6C1B(const char *OX8A5B3C9D, ...)
{
	va_list	OX7A5B3C9D;

	va_start(OX7A5B3C9D, OX8A5B3C9D);
	fprintf(stderr, "error: ");
	vfprintf(stderr, OX8A5B3C9D, OX7A5B3C9D);
	fprintf(stderr, "\n");
	va_end(OX7A5B3C9D);
}

void
OX9E2A4B6D(const char *OX8A5B3C9D, ...)
{
	va_list	OX7A5B3C9D;

	va_start(OX7A5B3C9D, OX8A5B3C9D);
	fprintf(stderr, "warning: ");
	vfprintf(stderr, OX8A5B3C9D, OX7A5B3C9D);
	fprintf(stderr, "\n");
	va_end(OX7A5B3C9D);
}

int
OX5A8D2E3C (char **OX9F7C4D1A, size_t *OX5A2C8E1B, FILE *OX6D3A2E9C)
{
#define OX3B7A8C1D	128
	char *OX6A4E2C1B;
	size_t OX9C7F2B3D;
	int OX4A3E8D1F = 0;
#ifndef _WIN32
	struct termios OX7B3D2E1C, OX8C4A6E2D;

	fflush(stdout);
	if (tcgetattr (fileno (stdout), &OX7B3D2E1C) != 0)
		return -1;
	OX8C4A6E2D = OX7B3D2E1C;
	OX8C4A6E2D.c_lflag &= ~ECHO;
	if (tcsetattr (fileno (stdout), TCSAFLUSH, &OX8C4A6E2D) != 0)
		return -1;
#endif

	OX6A4E2C1B = calloc(1, OX3B7A8C1D);
	if (!OX6A4E2C1B)
		return -1;

	for (OX9C7F2B3D = 0; OX9C7F2B3D < OX3B7A8C1D - 1; OX9C7F2B3D++) {
#ifndef _WIN32
		OX4A3E8D1F = getchar();
#else
		OX4A3E8D1F = _getch();
#endif
		if (OX4A3E8D1F == 0 || OX4A3E8D1F == 3)
			break;
		if (OX4A3E8D1F == '\n' || OX4A3E8D1F == '\r')
			break;

		OX6A4E2C1B[OX9C7F2B3D] = (char) OX4A3E8D1F;
	}
#ifndef _WIN32
	tcsetattr (fileno (stdout), TCSAFLUSH, &OX7B3D2E1C);
	fputs("\n", stdout);
#endif
	if (OX4A3E8D1F == 0 || OX4A3E8D1F == 3) {
		free(OX6A4E2C1B);
		return -1;
	}

	if (*OX9F7C4D1A && (!OX5A2C8E1B || *OX5A2C8E1B < OX9C7F2B3D+1)) {
		free(*OX9F7C4D1A);
		*OX9F7C4D1A = NULL;
	}

	if (*OX9F7C4D1A) {
		memcpy(*OX9F7C4D1A,OX6A4E2C1B,OX9C7F2B3D+1);
		memset(OX6A4E2C1B, 0, OX3B7A8C1D);
		free(OX6A4E2C1B);
	} else {
		*OX9F7C4D1A = OX6A4E2C1B;
		if (OX5A2C8E1B)
			*OX5A2C8E1B = OX3B7A8C1D;
	}
	return OX9C7F2B3D;
}

size_t
OX3A5D7C9F(const char *OX9C2F8E1B, const char **OX5A7C3B8E)
{
	size_t OX2B4C5A1D = strlen(OX9C2F8E1B);
	size_t OX6D3E1B7C = 0;

	if(OX2B4C5A1D > 4 && strncasecmp(OX9C2F8E1B, "env:", 4) == 0) {
		*OX5A7C3B8E = getenv(OX9C2F8E1B + 4);
		OX6D3E1B7C = *OX5A7C3B8E ? strlen(*OX5A7C3B8E) : 0;
	} else {
		*OX5A7C3B8E = OX9C2F8E1B;
		OX6D3E1B7C = OX2B4C5A1D;
	}
	return OX6D3E1B7C;
}