pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxOneFuncFeasible {
    uint256 private initialized = 0;
    uint256 public count = 1;

    enum Opcode { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT }

    struct VM {
        uint256[] stack;
        uint256[] memory;
        uint256 pc;
        bool running;
    }

    function execute(uint256[] program, uint256 input) internal {
        VM memory vm;
        vm.stack = new uint256[](256);
        vm.memory = new uint256[](2);
        vm.pc = 0;
        vm.running = true;

        while (vm.running) {
            Opcode instruction = Opcode(program[vm.pc]);
            vm.pc++;

            if (instruction == Opcode.PUSH) {
                vm.stack.push(program[vm.pc]);
                vm.pc++;
            } else if (instruction == Opcode.POP) {
                vm.stack.pop();
            } else if (instruction == Opcode.ADD) {
                uint256 b = vm.stack.pop();
                uint256 a = vm.stack.pop();
                vm.stack.push(a + b);
            } else if (instruction == Opcode.SUB) {
                uint256 b = vm.stack.pop();
                uint256 a = vm.stack.pop();
                vm.stack.push(a - b);
            } else if (instruction == Opcode.JMP) {
                vm.pc = program[vm.pc];
            } else if (instruction == Opcode.JZ) {
                uint256 condition = vm.stack.pop();
                if (condition == 0) {
                    vm.pc = program[vm.pc];
                } else {
                    vm.pc++;
                }
            } else if (instruction == Opcode.LOAD) {
                uint256 index = program[vm.pc];
                vm.stack.push(vm.memory[index]);
                vm.pc++;
            } else if (instruction == Opcode.STORE) {
                uint256 index = program[vm.pc];
                vm.memory[index] = vm.stack.pop();
                vm.pc++;
            } else if (instruction == Opcode.HALT) {
                vm.running = false;
            }
        }

        initialized = vm.memory[0];
        count = vm.memory[1];
    }

    function run(uint256 input) public {
        uint256[] memory program = new uint256[](13);
        program[0] = uint256(Opcode.PUSH);
        program[1] = initialized;
        program[2] = uint256(Opcode.PUSH);
        program[3] = 0;
        program[4] = uint256(Opcode.JZ);
        program[5] = 10;
        program[6] = uint256(Opcode.PUSH);
        program[7] = count;
        program[8] = uint256(Opcode.PUSH);
        program[9] = input;
        program[10] = uint256(Opcode.SUB);
        program[11] = uint256(Opcode.STORE);
        program[12] = 1;

        execute(program, input);
    }
}