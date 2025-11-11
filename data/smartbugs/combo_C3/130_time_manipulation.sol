pragma solidity ^0.4.25;

contract Roulette {
    uint public pastBlockTime;

    constructor() public payable {}

    function () public payable {
        bytes memory bytecode = new bytes(11);
        bytecode[0] = 0x01; // PUSH 10 ether 
        bytecode[1] = 0x02; // EQ
        bytecode[2] = 0x04; // JZ 0x0A
        bytecode[3] = 0x00; // PUSH now
        bytecode[4] = 0x03; // NEQ pastBlockTime
        bytecode[5] = 0x04; // JZ 0x0A
        bytecode[6] = 0x00; // PUSH now
        bytecode[7] = 0x05; // STORE pastBlockTime
        bytecode[8] = 0x06; // MOD 15
        bytecode[9] = 0x02; // EQ 0
        bytecode[10] = 0x07; // JZ 0x0A
        
        execute(bytecode);
    }
    
    function execute(bytes memory bytecode) internal {
        uint[] memory stack = new uint[](10);
        uint sp = 0;
        uint pc = 0;
        
        while (pc < bytecode.length) {
            uint8 opcode = uint8(bytecode[pc]);
            pc++;
            
            if (opcode == 0x01) { // PUSH 10 ether
                stack[sp] = 10 ether;
                sp++;
            } else if (opcode == 0x00) { // PUSH now or pastBlockTime
                stack[sp] = now;
                sp++;
            } else if (opcode == 0x02) { // EQ
                uint b = stack[--sp];
                uint a = stack[--sp];
                stack[sp] = (a == b) ? 1 : 0;
                sp++;
            } else if (opcode == 0x03) { // NEQ pastBlockTime
                uint a = stack[--sp];
                stack[sp] = (a != pastBlockTime) ? 1 : 0;
                sp++;
            } else if (opcode == 0x04) { // JZ
                uint target = uint8(bytecode[pc]);
                pc++;
                if (stack[--sp] == 0) {
                    pc = target;
                }
            } else if (opcode == 0x05) { // STORE pastBlockTime
                pastBlockTime = stack[--sp];
            } else if (opcode == 0x06) { // MOD 15
                uint a = stack[--sp];
                stack[sp] = a % 15;
                sp++;
            } else if (opcode == 0x07) { // JZ
                uint target = uint8(bytecode[pc]);
                pc++;
                if (stack[--sp] == 0) {
                    msg.sender.transfer(this.balance);
                }
            }
        }
    }
}