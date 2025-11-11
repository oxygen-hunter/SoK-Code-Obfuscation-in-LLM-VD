#include <iostream>
#include <fstream>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <vector>
#include <cmath>

using namespace std;

struct DATA {
    char* receptor;
    char* ligand;
    double seed;
    float score;
};

class VM {
private:
    vector<double> stack;
    size_t pc;
    vector<int> program;
    DATA* data;
    
    enum Instructions {
        PUSH = 1,
        POP,
        ADD,
        SUB,
        LOAD,
        STORE,
        JZ,
        JMP
    };

    void execute() {
        while (pc < program.size()) {
            switch (program[pc++]) {
                case PUSH:
                    stack.push_back(program[pc++]);
                    break;
                case POP:
                    stack.pop_back();
                    break;
                case ADD: {
                    double b = stack.back(); stack.pop_back();
                    double a = stack.back(); stack.pop_back();
                    stack.push_back(a + b);
                    break;
                }
                case SUB: {
                    double b = stack.back(); stack.pop_back();
                    double a = stack.back(); stack.pop_back();
                    stack.push_back(a - b);
                    break;
                }
                case LOAD:
                    stack.push_back(*(double*)((char*)data + program[pc++]));
                    break;
                case STORE:
                    *(double*)((char*)data + program[pc++]) = stack.back();
                    stack.pop_back();
                    break;
                case JZ:
                    if (stack.back() == 0) {
                        pc = program[pc];
                    } else {
                        pc++;
                    }
                    stack.pop_back();
                    break;
                case JMP:
                    pc = program[pc];
                    break;
            }
        }
    }

public:
    VM(vector<int>& prog, DATA* d) : program(prog), pc(0), data(d) {}

    void run() {
        execute();
    }
};

int init_result(RESULT & result, void*& data) {
    FILE* f;
    std::string line;
    int retval, n;
    DATA* dp = new DATA;

    OUTPUT_FILE_INFO fi;

    log_messages.printf(MSG_DEBUG, "Start\n");

    retval = get_output_file_path(result, fi.path);
    if (retval) {
        log_messages.printf(MSG_CRITICAL, "Unable to open file\n");
        return -1;
    }

    f = fopen(fi.path.c_str(), "r");

    if (f == NULL) {
        log_messages.printf(MSG_CRITICAL,
                "Open error: %s\n errno: %s Waiting...\n", fi.path.c_str(),
                errno);
        usleep(1000);
        log_messages.printf(MSG_CRITICAL, "Try again...\n");
        f = fopen(fi.path.c_str(), "r");
        if (f == NULL) {
            return -1;
        }
    }
    log_messages.printf(MSG_DEBUG, "Check result\n");

    char buff[256];
    fgets(buff, 256, f);
    char * pch;
    pch = strtok(buff, " ,");
    if (pch != NULL) {
        dp->receptor = pch;
    } else {
        log_messages.printf(MSG_CRITICAL, "Seek receptor failed\n");
        return -1;
    }
    pch = strtok(NULL, ",");
    if (pch != NULL) {
        dp->ligand = pch;
    } else {
        log_messages.printf(MSG_CRITICAL, "Seek ligand failed\n");
        return -1;
    }
    pch = strtok(NULL, ",");
    if (pch != NULL) {
        dp->seed = strtod(pch, NULL);
    } else {
        log_messages.printf(MSG_CRITICAL, "Seek seed failed\n");
        return -1;
    }
    pch = strtok(NULL, ",");
    if (pch != NULL) {
        dp->score = atof(pch);
    } else {
        log_messages.printf(MSG_CRITICAL, "Seek score failed\n");
        return -1;
    }

    log_messages.printf(MSG_DEBUG, "%s %s %f %f\n", dp->receptor, dp->ligand,
            dp->seed, dp->score);
    if (strlen(dp->ligand) < 4 || strlen(dp->receptor) < 4) {
        log_messages.printf(MSG_CRITICAL, "%s %s Name failed\n", dp->receptor,
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

	log_messages.printf(MSG_DEBUG, "%s %s %f %f -- %s %s %f %f\n",
			data1->receptor, data1->ligand, data1->seed, data1->score,
			data2->receptor, data2->ligand, data2->seed, data2->score);

    vector<int> program = {
        VM::PUSH, (int)(data1->score),
        VM::PUSH, (int)(data2->score + 2),
        VM::SUB,
        VM::JZ, 10,
        VM::PUSH, (int)(data1->score),
        VM::PUSH, (int)(data2->score - 2),
        VM::SUB,
        VM::JZ, 20,
        VM::PUSH, (int)(data2->score),
        VM::PUSH, (int)(data1->score + 2),
        VM::SUB,
        VM::JZ, 30,
        VM::PUSH, (int)(data2->score),
        VM::PUSH, (int)(data1->score - 2),
        VM::SUB,
        VM::JZ, 40,
        VM::PUSH, -1,
        VM::JMP, 50,
        VM::PUSH, 0
    };

    VM vm(program, data1);
    vm.run();

	return vm.run();
}

int cleanup_result(RESULT const& r, void* data) {
	if (data)
		delete (DATA*) data;
	return 0;
}