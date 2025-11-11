pragma solidity ^0.4.25;

contract RandomNumberGenerator {
    uint256 private salt = block.timestamp;

    function random(uint max) view private returns (uint256 result) {
        bytes32[] memory instructions = new bytes32[](18);
        instructions[0] = bytes32(uint256(1)); // PUSH salt
        instructions[1] = bytes32(uint256(100)); // PUSH 100
        instructions[2] = bytes32(uint256(3)); // PUSH max
        instructions[3] = bytes32(uint256(4)); // MUL
        instructions[4] = bytes32(uint256(5)); // DIV
        instructions[5] = bytes32(uint256(1)); // PUSH salt
        instructions[6] = bytes32(uint256(block.number)); // PUSH block.number
        instructions[7] = bytes32(uint256(5)); // PUSH 5
        instructions[8] = bytes32(uint256(6)); // MOD
        instructions[9] = bytes32(uint256(7)); // DIV
        instructions[10] = bytes32(uint256(8)); // ADD
        instructions[11] = bytes32(uint256(block.number)); // PUSH block.number
        instructions[12] = bytes32(uint256(3)); // PUSH 3
        instructions[13] = bytes32(uint256(9)); // DIV
        instructions[14] = bytes32(uint256(10)); // ADD
        instructions[15] = bytes32(uint256(300)); // PUSH 300
        instructions[16] = bytes32(uint256(11)); // MOD
        instructions[17] = bytes32(uint256(12)); // ADD

        return interpret(instructions, max);
    }

    function interpret(bytes32[] memory instructions, uint max) view private returns (uint256) {
        uint256[] memory stack = new uint256[](10);
        uint256 sp = 0;
        uint256 pc = 0;

        while (pc < instructions.length) {
            uint256 instr = uint256(instructions[pc]);

            if (instr == 1) { // PUSH salt
                stack[sp++] = salt;
            } else if (instr == 2) { // PUSH 100
                stack[sp++] = 100;
            } else if (instr == 3) { // PUSH max
                stack[sp++] = max;
            } else if (instr == 4) { // MUL
                stack[sp - 2] = stack[sp - 2] * stack[sp - 1];
                sp--;
            } else if (instr == 5) { // DIV
                stack[sp - 2] = stack[sp - 2] / stack[sp - 1];
                sp--;
            } else if (instr == 6) { // MOD
                stack[sp - 2] = stack[sp - 2] % stack[sp - 1];
                sp--;
            } else if (instr == 7) { // DIV
                stack[sp - 2] = stack[sp - 2] / stack[sp - 1];
                sp--;
            } else if (instr == 8) { // ADD
                stack[sp - 2] = stack[sp - 2] + stack[sp - 1];
                sp--;
            } else if (instr == 9) { // DIV
                stack[sp - 2] = stack[sp - 2] / stack[sp - 1];
                sp--;
            } else if (instr == 10) { // ADD
                stack[sp - 2] = stack[sp - 2] + stack[sp - 1];
                sp--;
            } else if (instr == 11) { // MOD
                stack[sp - 2] = stack[sp - 2] % stack[sp - 1];
                sp--;
            } else if (instr == 12) { // ADD
                stack[sp - 2] = stack[sp - 2] + stack[sp - 1];
                sp--;
            }
            pc++;
        }

        uint256 seed = stack[sp - 1];
        uint256 h = uint256(blockhash(seed));
        uint256 x = stack[0];
        return uint256((h / x)) % max + 1;
    }
}