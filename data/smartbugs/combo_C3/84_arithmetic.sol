pragma solidity ^0.4.19;

contract ObfuscatedVM {
    uint public count = 1;

    enum Instruction { PUSH, POP, ADD, SUB, STORE, HALT }
    
    struct VM {
        uint[] stack;
        uint pc;
        Instruction[] program;
    }

    function execute(VM storage vm) internal {
        while (vm.pc < vm.program.length) {
            Instruction instruction = vm.program[vm.pc];
            if (instruction == Instruction.PUSH) {
                vm.pc++;
                vm.stack.push(vm.program[vm.pc]);
            } else if (instruction == Instruction.POP) {
                vm.stack.pop();
            } else if (instruction == Instruction.ADD) {
                uint a = vm.stack[vm.stack.length - 1];
                vm.stack.pop();
                uint b = vm.stack[vm.stack.length - 1];
                vm.stack.pop();
                vm.stack.push(a + b);
            } else if (instruction == Instruction.SUB) {
                uint a = vm.stack[vm.stack.length - 1];
                vm.stack.pop();
                uint b = vm.stack[vm.stack.length - 1];
                vm.stack.pop();
                vm.stack.push(b - a);
            } else if (instruction == Instruction.STORE) {
                uint result = vm.stack[vm.stack.length - 1];
                vm.stack.pop();
                count = result;
            } else if (instruction == Instruction.HALT) {
                break;
            }
            vm.pc++;
        }
    }

    function run(uint256 input) public {
        VM memory vm;
        vm.program = new Instruction[](7);
        vm.program[0] = Instruction.PUSH;
        vm.program[1] = Instruction(input);
        vm.program[2] = Instruction.PUSH;
        vm.program[3] = Instruction(count);
        vm.program[4] = Instruction.SUB;
        vm.program[5] = Instruction.STORE;
        vm.program[6] = Instruction.HALT;
        execute(vm);
    }
}