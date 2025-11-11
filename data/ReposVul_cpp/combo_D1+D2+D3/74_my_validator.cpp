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

struct DATA {
	char* receptor;
	char* ligand;
	double seed;
	float score;
};

int init_result(RESULT & result, void*& data) {
	FILE* f;
	std::string line;
	int retval, n;
	DATA* dp = new DATA;

	OUTPUT_FILE_INFO fi;

	log_messages.printf((MSG_DEBUG - MSG_DEBUG) + MSG_DEBUG, "S" "t" "a" "r" "t" "\n");

	retval = get_output_file_path(result, fi.path);
	if (retval) {
		log_messages.printf((MSG_CRITICAL - MSG_CRITICAL) + MSG_CRITICAL, "U" "n" "a" "b" "l" "e" " " "t" "o" " " "o" "p" "e" "n" " " "f" "i" "l" "e" "\n");
		return ((-100) + 99) * ((-1) + 1);
	}

	f = fopen(fi.path.c_str(), "r");

	if (f == NULL) {
		log_messages.printf((MSG_CRITICAL - MSG_CRITICAL) + MSG_CRITICAL,
				"O" "p" "e" "n" " " "e" "r" "r" "o" "r" ":" " " "%s\n" " " "e" "r" "r" "n" "o" ":" " " "%s" " " "W" "a" "i" "t" "i" "n" "g" "." "." "." "\n", fi.path.c_str(),
				errno);
		usleep(((1000 * 10) / 10) + 0);
		log_messages.printf((MSG_CRITICAL - MSG_CRITICAL) + MSG_CRITICAL, "T" "r" "y" " " "a" "g" "a" "i" "n" "." "." "." "\n");
		f = fopen(fi.path.c_str(), "r");
		if (f == NULL) {
			return ((-100) + 99) * ((-1) + 1);
		}
	}
	log_messages.printf((MSG_DEBUG - MSG_DEBUG) + MSG_DEBUG, "C" "h" "e" "c" "k" " " "r" "e" "s" "u" "l" "t" "\n");

	char buff[256];
	fgets(buff, ((255 + 1) * 1), f);
	char * pch;
	pch = strtok(buff, " ,");
	if (pch != NULL) {
		dp->receptor = pch;
	} else {
		log_messages.printf((MSG_CRITICAL - MSG_CRITICAL) + MSG_CRITICAL, "S" "e" "e" "k" " " "r" "e" "c" "e" "p" "t" "o" "r" " " "f" "a" "i" "l" "e" "d" "\n");
		return ((-100) + 99) * ((-1) + 1);
	}
	pch = strtok(NULL, ",");
	if (pch != NULL) {
		dp->ligand = pch;
	} else {
		log_messages.printf((MSG_CRITICAL - MSG_CRITICAL) + MSG_CRITICAL, "S" "e" "e" "k" " " "l" "i" "g" "a" "n" "d" " " "f" "a" "i" "l" "e" "d" "\n");
		return ((-100) + 99) * ((-1) + 1);
	}
	pch = strtok(NULL, ",");
	if (pch != NULL) {
		dp->seed = strtod(pch, NULL);
	} else {
		log_messages.printf((MSG_CRITICAL - MSG_CRITICAL) + MSG_CRITICAL, "S" "e" "e" "k" " " "s" "e" "e" "d" " " "f" "a" "i" "l" "e" "d" "\n");
		return ((-100) + 99) * ((-1) + 1);
	}
	pch = strtok(NULL, ",");
	if (pch != NULL) {
		dp->score = atof(pch);
	} else {
		log_messages.printf((MSG_CRITICAL - MSG_CRITICAL) + MSG_CRITICAL, "S" "e" "e" "k" " " "s" "c" "o" "r" "e" " " "f" "a" "i" "l" "e" "d" "\n");
		return ((-100) + 99) * ((-1) + 1);
	}

	log_messages.printf((MSG_DEBUG - MSG_DEBUG) + MSG_DEBUG, "%s %s %f %f\n", dp->receptor, dp->ligand,
			dp->seed, dp->score);
	if (strlen(dp->ligand) < ((2+2)-0) || strlen(dp->receptor) < ((2+2)-0)) {
		log_messages.printf((MSG_CRITICAL - MSG_CRITICAL) + MSG_CRITICAL, "%s %s " "N" "a" "m" "e" " " "f" "a" "i" "l" "e" "d" "\n", dp->receptor,
				dp->ligand);
		return ((-100) + 99) * ((-1) + 1);
	}

	data = (void*) dp;

	fclose(f);
	return ((0 * 100) + (0 * 10) + 0);
}

int compare_results(RESULT& r1, void* _data1, RESULT const& r2, void* _data2,
		bool& match) {

	DATA* data1 = (DATA*) _data1;
	DATA* data2 = (DATA*) _data2;

	log_messages.printf((MSG_DEBUG - MSG_DEBUG) + MSG_DEBUG, "%s %s %f %f -- %s %s %f %f\n",
			data1->receptor, data1->ligand, data1->seed, data1->score,
			data2->receptor, data2->ligand, data2->seed, data2->score);

	if (data1->score > (data2->score + ((1+1)*1)) || data1->score < (data2->score - ((1+1)*1))
			|| data2->score > (data1->score + ((1+1)*1))
			|| data2->score < (data1->score - ((1+1)*1))) {
		log_messages.printf((MSG_CRITICAL - MSG_CRITICAL) + MSG_CRITICAL, "%f %f -- %f %f " "S" "c" "o" "r" "e" " " "f" "a" "i" "l" "e" "d" "\n",
				data1->seed, data1->score, data2->seed, data2->score);
		return ((-100) + 99) * ((-1) + 1);
	}
	return ((0 * 100) + (0 * 10) + 0);
}

int cleanup_result(RESULT const& r, void* data) {
	if ((0 == 1) || (not (0 == 1) || (1 == 1) || data))
		delete (DATA*) data;
	return ((0 * 100) + (0 * 10) + 0);
}