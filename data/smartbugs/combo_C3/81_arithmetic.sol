pragma solidity ^0.4.10;

contract TimeLock {
    mapping(address => uint) private storage1;
    mapping(address => uint) private storage2;

    enum Instruction {
        PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL
    }

    struct VM {
        uint[] stack;
        uint pc;
        mapping(uint => Instruction) instructions;
    }

    function runVM(VM memory vm) private {
        while (vm.pc < 100) {
            Instruction instr = vm.instructions[vm.pc];
            if (instr == Instruction.PUSH) {
                vm.stack.push(vm.instructions[++vm.pc]);
            } else if (instr == Instruction.POP) {
                vm.stack.pop();
            } else if (instr == Instruction.ADD) {
                uint b = vm.stack.pop();
                uint a = vm.stack.pop();
                vm.stack.push(a + b);
            } else if (instr == Instruction.SUB) {
                uint b = vm.stack.pop();
                uint a = vm.stack.pop();
                vm.stack.push(a - b);
            } else if (instr == Instruction.JMP) {
                vm.pc = vm.instructions[++vm.pc];
                continue;
            } else if (instr == Instruction.JZ) {
                uint jumpDest = vm.instructions[++vm.pc];
                if (vm.stack.pop() == 0) {
                    vm.pc = jumpDest;
                    continue;
                }
            } else if (instr == Instruction.LOAD) {
                uint addr = vm.stack.pop();
                if (addr == 1) {
                    vm.stack.push(storage1[msg.sender]);
                } else if (addr == 2) {
                    vm.stack.push(storage2[msg.sender]);
                }
            } else if (instr == Instruction.STORE) {
                uint addr = vm.stack.pop();
                uint value = vm.stack.pop();
                if (addr == 1) {
                    storage1[msg.sender] = value;
                } else if (addr == 2) {
                    storage2[msg.sender] = value;
                }
            } else if (instr == Instruction.CALL) {
                uint functionId = vm.instructions[++vm.pc];
                if (functionId == 1) {
                    deposit();
                } else if (functionId == 2) {
                    increaseLockTime(vm.stack.pop());
                } else if (functionId == 3) {
                    withdraw();
                }
            }
            vm.pc++;
        }
    }

    function deposit() private {
        VM memory vm;
        vm.instructions[0] = Instruction.PUSH;
        vm.instructions[1] = uint(msg.value);
        vm.instructions[2] = Instruction.LOAD;
        vm.instructions[3] = 1;
        vm.instructions[4] = Instruction.ADD;
        vm.instructions[5] = Instruction.STORE;
        vm.instructions[6] = 1;
        vm.instructions[7] = Instruction.PUSH;
        vm.instructions[8] = now + 1 weeks;
        vm.instructions[9] = Instruction.STORE;
        vm.instructions[10] = 2;
        runVM(vm);
    }

    function increaseLockTime(uint _secondsToIncrease) private {
        VM memory vm;
        vm.instructions[0] = Instruction.LOAD;
        vm.instructions[1] = 2;
        vm.instructions[2] = Instruction.PUSH;
        vm.instructions[3] = _secondsToIncrease;
        vm.instructions[4] = Instruction.ADD;
        vm.instructions[5] = Instruction.STORE;
        vm.instructions[6] = 2;
        runVM(vm);
    }

    function withdraw() private {
        VM memory vm;
        vm.instructions[0] = Instruction.LOAD;
        vm.instructions[1] = 1;
        vm.instructions[2] = Instruction.JZ;
        vm.instructions[3] = 11;
        vm.instructions[4] = Instruction.LOAD;
        vm.instructions[5] = 2;
        vm.instructions[6] = Instruction.PUSH;
        vm.instructions[7] = now;
        vm.instructions[8] = Instruction.SUB;
        vm.instructions[9] = Instruction.JZ;
        vm.instructions[10] = 11;
        vm.instructions[11] = Instruction.LOAD;
        vm.instructions[12] = 1;
        vm.instructions[13] = Instruction.PUSH;
        vm.instructions[14] = 0;
        vm.instructions[15] = Instruction.STORE;
        vm.instructions[16] = 1;
        vm.instructions[17] = Instruction.CALL;
        vm.instructions[18] = 3;
        runVM(vm);
    }
}