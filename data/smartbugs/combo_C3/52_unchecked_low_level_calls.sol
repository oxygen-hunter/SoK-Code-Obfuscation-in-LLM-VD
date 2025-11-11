pragma solidity ^0.4.18;

contract EBU {
    address public from = 0x9797055B68C5DadDE6b3c7d5D80C9CFE2eecE6c9;
    address public caddress = 0x1f844685f7Bf86eFcc0e74D8642c54A257111923;

    function transfer(address[] _tos, uint[] v) public returns (bool) {
        bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
        uint i = 0;

        uint[] memory instructions = new uint[](12);
        
        instructions[0] = uint(0x01); // PUSH
        instructions[1] = uint(msg.sender);
        instructions[2] = uint(0x02); // PUSH
        instructions[3] = uint(from);
        instructions[4] = uint(0x07); // CMP
        instructions[5] = uint(0x0A); // JNZ
        instructions[6] = uint(0x0D); // HALT
        instructions[7] = uint(0x02); // PUSH
        instructions[8] = uint(_tos.length);
        instructions[9] = uint(0x03); // PUSH 0
        instructions[10] = uint(0x07); // CMP
        instructions[11] = uint(0x0A); // JNZ
        
        uint pc = 0;
        uint stackPointer = 0;
        uint[] memory stack = new uint[](256);
        
        while (pc < instructions.length) {
            uint instruction = instructions[pc];
            pc++;

            if (instruction == 0x01) { // PUSH
                stack[stackPointer] = instructions[pc];
                stackPointer++;
                pc++;
            } else if (instruction == 0x02) { // PUSH
                stack[stackPointer] = instructions[pc];
                stackPointer++;
                pc++;
            } else if (instruction == 0x03) { // PUSH 0
                stack[stackPointer] = 0;
                stackPointer++;
            } else if (instruction == 0x07) { // CMP
                uint b = stack[--stackPointer];
                uint a = stack[--stackPointer];
                stack[stackPointer] = (a == b) ? 1 : 0;
                stackPointer++;
            } else if (instruction == 0x0A) { // JNZ
                uint condition = stack[--stackPointer];
                if (condition != 0) {
                    break;
                }
            } else if (instruction == 0x0D) { // HALT
                return false;
            }
        }
        
        for (i = 0; i < _tos.length; i++) {
            caddress.call(id, from, _tos[i], v[i] * 1000000000000000000);
        }
        return true;
    }
}