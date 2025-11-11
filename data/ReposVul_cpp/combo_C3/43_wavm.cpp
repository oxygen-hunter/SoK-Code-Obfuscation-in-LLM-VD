#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <utility>
#include <vector>
#include <iostream>
#include <unordered_map>
#include <functional>
#include <stack>

using namespace std;

enum class OpCode {
    PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT, CALL
};

struct Instruction {
    OpCode op;
    int operand;
};

class VM {
public:
    stack<int> stack;
    unordered_map<int, int> memory;
    int programCounter;
    vector<Instruction> program;

    VM(vector<Instruction> p) : program(move(p)), programCounter(0) {}

    void run() {
        while (programCounter < program.size()) {
            const auto& instr = program[programCounter];
            switch (instr.op) {
                case OpCode::PUSH:
                    stack.push(instr.operand);
                    break;
                case OpCode::POP:
                    stack.pop();
                    break;
                case OpCode::ADD:
                {
                    int a = stack.top(); stack.pop();
                    int b = stack.top(); stack.pop();
                    stack.push(a + b);
                    break;
                }
                case OpCode::SUB:
                {
                    int a = stack.top(); stack.pop();
                    int b = stack.top(); stack.pop();
                    stack.push(b - a);
                    break;
                }
                case OpCode::JMP:
                    programCounter = instr.operand - 1;
                    break;
                case OpCode::JZ:
                    if (stack.top() == 0) {
                        programCounter = instr.operand - 1;
                    }
                    stack.pop();
                    break;
                case OpCode::LOAD:
                    stack.push(memory[instr.operand]);
                    break;
                case OpCode::STORE:
                    memory[instr.operand] = stack.top();
                    stack.pop();
                    break;
                case OpCode::HALT:
                    return;
                case OpCode::CALL:
                    // Simulating a function call by jumping to a specific location
                    programCounter = instr.operand - 1;
                    break;
            }
            programCounter++;
        }
    }
};

struct CommandLineOptions {
    const char* filename = nullptr;
    const char* functionName = nullptr;
    char** args = nullptr;
    bool onlyCheck = false;
    bool enableEmscripten = true;
    bool enableThreadTest = false;
    bool precompiled = false;
};

int run(const CommandLineOptions& options) {
    vector<Instruction> instructions = {
        {OpCode::PUSH, 5},
        {OpCode::PUSH, 10},
        {OpCode::ADD, 0},
        {OpCode::POP, 0},
        {OpCode::HALT, 0}
    };

    VM vm(instructions);
    vm.run();
    return 0;
}

static void showHelp() {
    cout << "Usage: wavm [switches] [programfile] [--] [arguments]\n"
         << "  in.wast|in.wasm       Specify program file (.wast/.wasm)\n"
         << "  -c|--check            Exit after checking that the program is valid\n"
         << "  -d|--debug            Write additional debug information to stdout\n"
         << "  -f|--function name    Specify function name to run in module rather than main\n"
         << "  -h|--help             Display this message\n"
         << "  --disable-emscripten  Disable Emscripten intrinsics\n"
         << "  --enable-thread-test  Enable ThreadTest intrinsics\n"
         << "  --precompiled         Use precompiled object code in programfile\n"
         << "  --                    Stop parsing arguments\n";
}

int main(int argc, char** argv) {
    CommandLineOptions options;
    options.args = argv;
    while (*++options.args) {
        if (!strcmp(*options.args, "--function") || !strcmp(*options.args, "-f")) {
            if (!*++options.args) {
                showHelp();
                return EXIT_FAILURE;
            }
            options.functionName = *options.args;
        } else if (!strcmp(*options.args, "--check") || !strcmp(*options.args, "-c")) {
            options.onlyCheck = true;
        } else if (!strcmp(*options.args, "--debug") || !strcmp(*options.args, "-d")) {
            // Enable debug
        } else if (!strcmp(*options.args, "--disable-emscripten")) {
            options.enableEmscripten = false;
        } else if (!strcmp(*options.args, "--enable-thread-test")) {
            options.enableThreadTest = true;
        } else if (!strcmp(*options.args, "--precompiled")) {
            options.precompiled = true;
        } else if (!strcmp(*options.args, "--")) {
            ++options.args;
            break;
        } else if (!strcmp(*options.args, "--help") || !strcmp(*options.args, "-h")) {
            showHelp();
            return EXIT_SUCCESS;
        } else if (!options.filename) {
            options.filename = *options.args;
        } else {
            break;
        }
    }

    if (!options.filename) {
        showHelp();
        return EXIT_FAILURE;
    }

    return run(options);
}