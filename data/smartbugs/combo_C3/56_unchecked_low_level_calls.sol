pragma solidity ^0.4.16;

contract RealOldFuckMaker {
    address fuck = 0xc63e7b1DEcE63A77eD7E4Aeef5efb3b05C81438D;

    enum Opcode { PUSH, POP, ADD, SUB, JMP, JZ, CALL, HALT }
    
    struct VM {
        uint32[] stack;
        uint32 pc;
        bool running;
    }

    function execute(uint32[] program, uint32 number) internal {
        VM memory vm;
        vm.running = true;
        while (vm.running) {
            uint32 instruction = program[vm.pc++];
            Opcode opcode = Opcode(instruction);
            if (opcode == Opcode.PUSH) {
                vm.stack.push(program[vm.pc++]);
            } else if (opcode == Opcode.POP) {
                vm.stack.length--;
            } else if (opcode == Opcode.ADD) {
                uint32 a = vm.stack[vm.stack.length - 1];
                vm.stack.length--;
                uint32 b = vm.stack[vm.stack.length - 1];
                vm.stack[vm.stack.length - 1] = a + b;
            } else if (opcode == Opcode.SUB) {
                uint32 a = vm.stack[vm.stack.length - 1];
                vm.stack.length--;
                uint32 b = vm.stack[vm.stack.length - 1];
                vm.stack[vm.stack.length - 1] = b - a;
            } else if (opcode == Opcode.JMP) {
                vm.pc = program[vm.pc];
            } else if (opcode == Opcode.JZ) {
                uint32 value = vm.stack[vm.stack.length - 1];
                vm.stack.length--;
                if (value == 0) {
                    vm.pc = program[vm.pc];
                } else {
                    vm.pc++;
                }
            } else if (opcode == Opcode.CALL) {
                fuck.call(bytes4(sha3("giveBlockReward()")));
            } else if (opcode == Opcode.HALT) {
                vm.running = false;
            }
        }
    }

    function makeOldFucks(uint32 number) {
        uint32[] memory program = new uint32[](8);
        program[0] = uint32(Opcode.PUSH); program[1] = 0;
        program[2] = uint32(Opcode.PUSH); program[3] = number;
        program[4] = uint32(Opcode.JZ); program[5] = 7;
        program[6] = uint32(Opcode.CALL);
        program[7] = uint32(Opcode.HALT);

        execute(program, number);
    }
}