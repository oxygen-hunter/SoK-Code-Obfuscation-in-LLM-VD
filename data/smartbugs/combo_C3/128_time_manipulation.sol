pragma solidity ^0.4.0;
contract lottopollo {
    address leader;
    uint timestamp;

    enum Opcode { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET }
    uint[] stack;
    uint programCounter;
    uint randomNumber;

    function execute(uint[] memory bytecode, uint seed) internal {
        programCounter = 0;
        randomNumber = block.timestamp;
        
        while (programCounter < bytecode.length) {
            Opcode instruction = Opcode(bytecode[programCounter]);
            programCounter++;

            if (instruction == Opcode.PUSH) {
                stack.push(bytecode[programCounter]);
                programCounter++;
            } else if (instruction == Opcode.POP) {
                stack.pop();
            } else if (instruction == Opcode.ADD) {
                uint b = stack.pop();
                uint a = stack.pop();
                stack.push(a + b);
            } else if (instruction == Opcode.SUB) {
                uint b = stack.pop();
                uint a = stack.pop();
                stack.push(a - b);
            } else if (instruction == Opcode.JMP) {
                programCounter = bytecode[programCounter];
            } else if (instruction == Opcode.JZ) {
                uint value = stack.pop();
                if (value == 0) {
                    programCounter = bytecode[programCounter];
                } else {
                    programCounter++;
                }
            } else if (instruction == Opcode.LOAD) {
                stack.push(randomNumber);
            } else if (instruction == Opcode.STORE) {
                randomNumber = stack.pop();
            } else if (instruction == Opcode.CALL) {
                uint target = bytecode[programCounter];
                programCounter++;
                execute(bytecode, target);
            } else if (instruction == Opcode.RET) {
                return;
            }
        }
    }

    function draw(uint seed) {
        uint[] memory bytecode = new uint[](16);
        bytecode[0] = uint(Opcode.PUSH); bytecode[1] = uint(0);
        bytecode[2] = uint(Opcode.LOAD);
        bytecode[3] = uint(Opcode.PUSH); bytecode[4] = uint(24 hours);
        bytecode[5] = uint(Opcode.SUB);
        bytecode[6] = uint(Opcode.PUSH); bytecode[7] = uint(1 ether);
        bytecode[8] = uint(Opcode.ADD);
        bytecode[9] = uint(Opcode.JZ); bytecode[10] = uint(14);
        bytecode[11] = uint(Opcode.CALL); bytecode[12] = uint(Opcode.RET);
        bytecode[13] = uint(Opcode.RET);
        bytecode[14] = uint(Opcode.RET);
        
        execute(bytecode, seed);
    }
}