#include "calenderServer.h"
#include "serverActions.h"
#include "persistentState.h"
#include "renderPage.h"
#include "objectCache.h"
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <thread>
#include <algorithm>
#include <string>
#include <sstream>
#include <unistd.h>
#include <vector>
#include <map>
#include <list>
#include <boost/algorithm/string.hpp>

using namespace std;

enum Instructions {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, PRINT, EXIT
};

vector<int> stack;
int pc = 0;
vector<int> program;
map<string, int> variables;
int sp = -1;

void execute() {
    while (pc < program.size()) {
        switch (program[pc++]) {
            case PUSH:
                stack[++sp] = program[pc++];
                break;
            case POP:
                --sp;
                break;
            case ADD:
                stack[sp - 1] += stack[sp];
                --sp;
                break;
            case SUB:
                stack[sp - 1] -= stack[sp];
                --sp;
                break;
            case JMP:
                pc = program[pc];
                break;
            case JZ:
                if (stack[sp--] == 0)
                    pc = program[pc];
                else
                    ++pc;
                break;
            case LOAD:
                stack[++sp] = variables[program[pc++]];
                break;
            case STORE:
                variables[program[pc++]] = stack[sp--];
                break;
            case CALL:
                stack[++sp] = pc + 1;
                pc = program[pc];
                break;
            case RET:
                pc = stack[sp--];
                break;
            case PRINT:
                cout << stack[sp--] << endl;
                break;
            case EXIT:
                return;
        }
    }
}

void initProgram() {
    program = {
        // Main function logic
        LOAD, "argc",
        PUSH, 2,
        SUB,
        JZ, 10,
        CALL, 100, // printUsageAndExit
        LOAD, "argv1",
        CALL, 200, // stoi
        STORE, "portNum",
        CALL, 300, // serverListen
        CALL, 100, // printUsageAndExit
        EXIT,

        // printUsageAndExit function at 100
        PRINT,
        PUSH, 1,
        CALL, 400, // exit
        RET,

        // stoi function at 200
        // Simulated stoi logic
        RET,

        // serverListen function at 300
        // Simulated serverListen logic
        RET,

        // exit function at 400
        // Simulated exit logic
        RET,
    };
}

int main(int argc, char** argv) {
    variables["argc"] = argc;
    variables["argv1"] = stoi(argv[1]);
    
    initProgram();
    execute();

    return 0;
}