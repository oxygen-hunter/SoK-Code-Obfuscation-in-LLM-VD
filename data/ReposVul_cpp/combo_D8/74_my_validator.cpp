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

std::string getDynamicString(std::string str) {
    return str;
}

double getDynamicDouble(const char* str) {
    return strtod(str, NULL);
}

float getDynamicFloat(const char* str) {
    return atof(str);
}

int init_result(RESULT & result, void*& data) {
    FILE* f;
    std::string line;
    int retval, n;
    DATA* dp = new DATA;

    OUTPUT_FILE_INFO fi;

    log_messages.printf(MSG_DEBUG, getDynamicString("Start\n").c_str());

    retval = get_output_file_path(result, fi.path);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, getDynamicString("Unable to open file\n").c_str());
        return -1;
    }

    f = fopen(fi.path.c_str(), getDynamicString("r").c_str());

    if (f == NULL) {
        log_messages.printf(MSG_CRITICAL,
                getDynamicString("Open error: %s\n errno: %s Waiting...\n").c_str(), fi.path.c_str(),
                errno);
        usleep(1000);
        log_messages.printf(MSG_CRITICAL, getDynamicString("Try again...\n").c_str());
        f = fopen(fi.path.c_str(), getDynamicString("r").c_str());
        if (f == NULL) {
            return -1;
        }
    }
    log_messages.printf(MSG_DEBUG, getDynamicString("Check result\n").c_str());

    char buff[256];
    fgets(buff, 256, f);
    char * pch;
    pch = strtok(buff, getDynamicString(" ,").c_str());
    if (pch != NULL) {
        dp->receptor = pch;
    } else {
        log_messages.printf(MSG_CRITICAL, getDynamicString("Seek receptor failed\n").c_str());
        return -1;
    }
    pch = strtok(NULL, getDynamicString(",").c_str());
    if (pch != NULL) {
        dp->ligand = pch;
    } else {
        log_messages.printf(MSG_CRITICAL, getDynamicString("Seek ligand failed\n").c_str());
        return -1;
    }
    pch = strtok(NULL, getDynamicString(",").c_str());
    if (pch != NULL) {
        dp->seed = getDynamicDouble(pch);
    } else {
        log_messages.printf(MSG_CRITICAL, getDynamicString("Seek seed failed\n").c_str());
        return -1;
    }
    pch = strtok(NULL, getDynamicString(",").c_str());
    if (pch != NULL) {
        dp->score = getDynamicFloat(pch);
    } else {
        log_messages.printf(MSG_CRITICAL, getDynamicString("Seek score failed\n").c_str());
        return -1;
    }

    log_messages.printf(MSG_DEBUG, getDynamicString("%s %s %f %f\n").c_str(), dp->receptor, dp->ligand,
            dp->seed, dp->score);
    if (strlen(dp->ligand) < 4 || strlen(dp->receptor) < 4) {
        log_messages.printf(MSG_CRITICAL, getDynamicString("%s %s Name failed\n").c_str(), dp->receptor,
                dp->ligand);
        return -1;
    }

    data = (void*) dp;

    fclose(f);
    return 0;
}

int compare_results(RESULT& r1, void* _data1, RESULT const& r2, void* _data2,
        bool& match) {

    DATA* data1 = (DATA*) _data1;
    DATA* data2 = (DATA*) _data2;

    log_messages.printf(MSG_DEBUG, getDynamicString("%s %s %f %f -- %s %s %f %f\n").c_str(),
            data1->receptor, data1->ligand, data1->seed, data1->score,
            data2->receptor, data2->ligand, data2->seed, data2->score);

    if (data1->score > (data2->score + 2) || data1->score < (data2->score - 2)
            || data2->score > (data1->score + 2)
            || data2->score < (data1->score - 2)) {
        log_messages.printf(MSG_CRITICAL, getDynamicString("%f %f -- %f %f Score failed\n").c_str(),
                data1->seed, data1->score, data2->seed, data2->score);
        return -1;
    }
    return 0;
}

int cleanup_result(RESULT const& r, void* data) {
    if (data)
        delete (DATA*) data;
    return 0;
}