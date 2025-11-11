#include <iostream>
#include <vector>
#include <unordered_map>
#include <functional>

class VM {
public:
    using Instruction = std::function<void(VM&)>;
    
    enum Opcode {
        PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT
    };

    VM() : pc(0), running(true) {}

    void Run() {
        while (running) {
            auto instr = program[pc++];
            instr(*this);
        }
    }

    void LoadProgram(const std::vector<Instruction>& prog) {
        program = prog;
    }

    void Push(int value) {
        stack.push_back(value);
    }

    int Pop() {
        if (stack.empty()) throw std::runtime_error("Stack underflow");
        int value = stack.back();
        stack.pop_back();
        return value;
    }

    void Add() {
        int b = Pop();
        int a = Pop();
        Push(a + b);
    }

    void Sub() {
        int b = Pop();
        int a = Pop();
        Push(a - b);
    }

    void Jmp(unsigned int address) {
        pc = address;
    }

    void Jz(unsigned int address) {
        if (Pop() == 0) pc = address;
    }

    void Load(unsigned int reg) {
        Push(registers[reg]);
    }

    void Store(unsigned int reg) {
        registers[reg] = Pop();
    }

    void Halt() {
        running = false;
    }

private:
    unsigned int pc;
    bool running;
    std::vector<int> stack;
    std::unordered_map<unsigned int, int> registers;
    std::vector<Instruction> program;
};

int main() {
    VM vm;

    std::vector<VM::Instruction> program = {
        [](VM& vm) { vm.Push(10); },
        [](VM& vm) { vm.Push(20); },
        [](VM& vm) { vm.Add(); },
        [](VM& vm) { vm.Store(1); },
        [](VM& vm) { vm.Load(1); },
        [](VM& vm) { vm.Push(5); },
        [](VM& vm) { vm.Sub(); },
        [](VM& vm) { vm.Halt(); }
    };

    vm.LoadProgram(program);
    vm.Run();

    return 0;
}