pragma solidity ^0.4.15;

contract Overflow {
    uint private sellerBalance = 0;

    function executeVM(uint[] instructions, uint[] data) private returns (bool) {
        uint stackPointer = 0;
        uint[256] memory stack;
        uint pc = 0;

        while (pc < instructions.length) {
            uint opcode = instructions[pc];
            
            if (opcode == 1) {  // PUSH
                stack[stackPointer++] = data[instructions[++pc]];
            } else if (opcode == 2) {  // ADD
                stack[stackPointer - 2] = stack[stackPointer - 2] + stack[stackPointer - 1];
                stackPointer--;
            } else if (opcode == 3) {  // LOAD
                stack[stackPointer++] = sellerBalance;
            } else if (opcode == 4) {  // STORE
                sellerBalance = stack[--stackPointer];
            } else if (opcode == 5) {  // JUMP
                pc = instructions[++pc] - 1;
            } else if (opcode == 6) {  // JUMPIFZERO
                if (stack[--stackPointer] == 0) {
                    pc = instructions[++pc] - 1;
                } else {
                    pc++;
                }
            } else if (opcode == 0) {  // HALT
                break;
            }
            pc++;
        }
        return true;
    }

    function add(uint value) returns (bool) {
        uint[] memory instructions = new uint[](6);
        uint[] memory data = new uint[](1);

        instructions[0] = 1;  // PUSH
        instructions[1] = 0;  // Data index 0
        instructions[2] = 3;  // LOAD
        instructions[3] = 2;  // ADD
        instructions[4] = 4;  // STORE
        instructions[5] = 0;  // HALT

        data[0] = value;

        return executeVM(instructions, data);
    }
}