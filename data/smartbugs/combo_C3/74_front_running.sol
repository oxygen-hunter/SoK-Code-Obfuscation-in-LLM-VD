pragma solidity ^0.4.22;

contract FindThisHash {
    bytes32 constant public hash = 0xb5b5b97fafd9855eec9b41f74dfb6c38f5951141f9a3ecd7f44d5479b630ee0a;

    constructor() public payable {}

    function solve(string solution) public {
        bytes32[256] memory stack;
        uint256 sp = 0; // stack pointer
        uint256 pc = 0; // program counter
        bytes32[256] memory code;
        
        code[0] = bytes32("PUSH");
        code[1] = bytes32(sha3(solution));
        code[2] = bytes32("PUSH");
        code[3] = hash;
        code[4] = bytes32("EQ");
        code[5] = bytes32("JZ");
        code[6] = bytes32(10); // jump to end if not equal
        code[7] = bytes32("PUSH");
        code[8] = bytes32(uint256(msg.sender));
        code[9] = bytes32("TRANS");
        code[10] = bytes32("END");

        while (true) {
            bytes32 instr = code[pc];
            if (instr == bytes32("PUSH")) {
                pc++;
                stack[sp++] = code[pc];
            } else if (instr == bytes32("EQ")) {
                bytes32 b = stack[--sp];
                bytes32 a = stack[--sp];
                stack[sp++] = (a == b) ? bytes32(uint256(1)) : bytes32(uint256(0));
            } else if (instr == bytes32("JZ")) {
                pc++;
                if (stack[--sp] == bytes32(uint256(0))) {
                    pc = uint256(code[pc]);
                    continue;
                }
            } else if (instr == bytes32("TRANS")) {
                address(uint256(stack[--sp])).transfer(1000 ether);
            } else if (instr == bytes32("END")) {
                break;
            }
            pc++;
        }
    }
}