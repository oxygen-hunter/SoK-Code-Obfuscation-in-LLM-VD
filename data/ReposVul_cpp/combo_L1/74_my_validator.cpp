#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <errno.h>
#include "boinc/sched_msgs.h"
#include "boinc/error_numbers.h"
#include "boinc/boinc_db.h"
#include "sched_util.h"
#include "validate_util.h"

using namespace std;

struct OX7B4DF339 {
	char* OX1F4D6A02;
	char* OX3A9E5D9C;
	double OX6E7C4B1F;
	float OX7D2A1C9B;
};

int OX2E5F8A4B(RESULT & OX4A9D8F2B, void*& OX8C3D1B7E) {
	FILE* OX9B7E1D4C;
	std::string OX5C9E7B2A;
	int OX7F8A2C4D, OX6B1D3F7E;
	OX7B4DF339* OX2D4E9B1F = new OX7B4DF339;

	OUTPUT_FILE_INFO OX1A7C3E5B;

	log_messages.printf(MSG_DEBUG, "Start\n");

	OX7F8A2C4D = get_output_file_path(OX4A9D8F2B, OX1A7C3E5B.path);
	if (OX7F8A2C4D) {
		log_messages.printf(MSG_CRITICAL, "Unable to open file\n");
		return -1;
	}

	OX9B7E1D4C = fopen(OX1A7C3E5B.path.c_str(), "r");

	if (OX9B7E1D4C == NULL) {
		log_messages.printf(MSG_CRITICAL,
				"Open error: %s\n errno: %s Waiting...\n", OX1A7C3E5B.path.c_str(),
				errno);
		usleep(1000);
		log_messages.printf(MSG_CRITICAL, "Try again...\n");
		OX9B7E1D4C = fopen(OX1A7C3E5B.path.c_str(), "r");
		if (OX9B7E1D4C == NULL) {
			return -1;
		}
	}
	log_messages.printf(MSG_DEBUG, "Check result\n");

	char OX3D9A5F7E[256];
	fgets(OX3D9A5F7E, 256, OX9B7E1D4C);
	char * OX5D8A2C7F;
	OX5D8A2C7F = strtok(OX3D9A5F7E, " ,");
	if (OX5D8A2C7F != NULL) {
		OX2D4E9B1F->OX1F4D6A02 = OX5D8A2C7F;
	} else {
		log_messages.printf(MSG_CRITICAL, "Seek receptor failed\n");
		return -1;
	}
	OX5D8A2C7F = strtok(NULL, ",");
	if (OX5D8A2C7F != NULL) {
		OX2D4E9B1F->OX3A9E5D9C = OX5D8A2C7F;
	} else {
		log_messages.printf(MSG_CRITICAL, "Seek ligand failed\n");
		return -1;
	}
	OX5D8A2C7F = strtok(NULL, ",");
	if (OX5D8A2C7F != NULL) {
		OX2D4E9B1F->OX6E7C4B1F = strtod(OX5D8A2C7F, NULL);
	} else {
		log_messages.printf(MSG_CRITICAL, "Seek seed failed\n");
		return -1;
	}
	OX5D8A2C7F = strtok(NULL, ",");
	if (OX5D8A2C7F != NULL) {
		OX2D4E9B1F->OX7D2A1C9B = atof(OX5D8A2C7F);
	} else {
		log_messages.printf(MSG_CRITICAL, "Seek score failed\n");
		return -1;
	}

	log_messages.printf(MSG_DEBUG, "%s %s %f %f\n", OX2D4E9B1F->OX1F4D6A02, OX2D4E9B1F->OX3A9E5D9C,
			OX2D4E9B1F->OX6E7C4B1F, OX2D4E9B1F->OX7D2A1C9B);
	if (strlen(OX2D4E9B1F->OX3A9E5D9C) < 4 || strlen(OX2D4E9B1F->OX1F4D6A02) < 4) {
		log_messages.printf(MSG_CRITICAL, "%s %s Name failed\n", OX2D4E9B1F->OX1F4D6A02,
				OX2D4E9B1F->OX3A9E5D9C);
		return -1;
	}

	OX8C3D1B7E = (void*) OX2D4E9B1F;

	fclose(OX9B7E1D4C);
	return 0;
}

int OX9D4B1F7C(RESULT& OX7C5D3A1B, void* OX2F8A7D1C, RESULT const& OX3E5A9B7D, void* OX6C2D1B4E,
		bool& OX5B8E3A7C) {

	OX7B4DF339* OX1B7F4D2A = (OX7B4DF339*) OX2F8A7D1C;
	OX7B4DF339* OX4D6A3C2B = (OX7B4DF339*) OX6C2D1B4E;

	log_messages.printf(MSG_DEBUG, "%s %s %f %f -- %s %s %f %f\n",
			OX1B7F4D2A->OX1F4D6A02, OX1B7F4D2A->OX3A9E5D9C, OX1B7F4D2A->OX6E7C4B1F, OX1B7F4D2A->OX7D2A1C9B,
			OX4D6A3C2B->OX1F4D6A02, OX4D6A3C2B->OX3A9E5D9C, OX4D6A3C2B->OX6E7C4B1F, OX4D6A3C2B->OX7D2A1C9B);

	if (OX1B7F4D2A->OX7D2A1C9B > (OX4D6A3C2B->OX7D2A1C9B + 2) || OX1B7F4D2A->OX7D2A1C9B < (OX4D6A3C2B->OX7D2A1C9B - 2)
			|| OX4D6A3C2B->OX7D2A1C9B > (OX1B7F4D2A->OX7D2A1C9B + 2)
			|| OX4D6A3C2B->OX7D2A1C9B < (OX1B7F4D2A->OX7D2A1C9B - 2)) {
		log_messages.printf(MSG_CRITICAL, "%f %f -- %f %f Score failed\n",
				OX1B7F4D2A->OX6E7C4B1F, OX1B7F4D2A->OX7D2A1C9B, OX4D6A3C2B->OX6E7C4B1F, OX4D6A3C2B->OX7D2A1C9B);
		return -1;
	}
	return 0;
}

int OX8F1D2C3B(RESULT const& OX3F7E6A1B, void* OX9A4D3B2E) {
	if (OX9A4D3B2E)
		delete (OX7B4DF339*) OX9A4D3B2E;
	return 0;
}