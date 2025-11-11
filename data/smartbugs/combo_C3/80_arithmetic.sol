pragma solidity ^0.4.19;

contract VMInterpreter {
    uint public count = 1;

    function run(uint256 input) public {
        uint256[256] memory stack;
        uint256 sp = 0;
        uint256 pc = 0;
        uint256 instruction;

        bytes32[7] memory program = [
            bytes32(uint256(0x6001)),  
            bytes32(uint256(0x6000)),  
            bytes32(uint256(0x6000)),  
            bytes32(uint256(0x6001)),  
            bytes32(uint256(0x6002)),  
            bytes32(uint256(0x6003)),  
            bytes32(uint256(0x6004))   
        ];

        while (pc < program.length) {
            instruction = uint256(program[pc++]);

            if (instruction == 0x6000) { 
                stack[sp++] = input;
            } else if (instruction == 0x6001) { 
                stack[sp++] = count;
            } else if (instruction == 0x6002) { 
                uint256 a = stack[--sp];
                uint256 b = stack[--sp];
                stack[sp++] = b - a;
            } else if (instruction == 0x6003) { 
                count = stack[--sp];
            } else if (instruction == 0x6004) { 
                break;
            }
        }
    }
}