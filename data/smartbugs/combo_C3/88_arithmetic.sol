pragma solidity ^0.4.19;

contract VM {
    uint256 public count = 1;
    uint256[256] private stack;
    uint256 private sp;
    uint256 private pc;
    uint256 private running;

    enum Instructions { PUSH, ADD, STORE, HALT }

    function run(uint256 input) public {
        uint256[5] memory program = [
            uint256(Instructions.PUSH), // Push initial count
            1,
            uint256(Instructions.PUSH), // Push input
            input,
            uint256(Instructions.ADD), // Add top two values on stack
            uint256(Instructions.STORE), // Store result in count
            uint256(Instructions.HALT) // Halt execution
        ];

        sp = 0;
        pc = 0;
        running = 1;

        while (running == 1) {
            uint256 instr = program[pc++];
            if (instr == uint256(Instructions.PUSH)) {
                stack[sp++] = program[pc++];
            } else if (instr == uint256(Instructions.ADD)) {
                uint256 a = stack[--sp];
                uint256 b = stack[--sp];
                stack[sp++] = a + b;
            } else if (instr == uint256(Instructions.STORE)) {
                count = stack[--sp];
            } else if (instr == uint256(Instructions.HALT)) {
                running = 0;
            }
        }
    }
}