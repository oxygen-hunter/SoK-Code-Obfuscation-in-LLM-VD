pragma solidity 0.4.24;

contract ObfuscatedRefunder {
    address[] private addresses;
    mapping (address => uint) public balances;

    constructor() {
        addresses.push(0x79B483371E87d664cd39491b5F06250165e4b184);
        addresses.push(0x79B483371E87d664cd39491b5F06250165e4b185);
    }

    function executeVM(uint[] memory code) private {
        uint[] memory stack = new uint[](256);
        uint sp = 0;
        uint pc = 0;
        
        while (pc < code.length) {
            uint instr = code[pc];
            pc++;
            
            if (instr == 0) { // PUSH
                stack[sp] = code[pc];
                sp++;
                pc++;
            } else if (instr == 1) { // POP
                sp--;
            } else if (instr == 2) { // ADD
                sp--;
                stack[sp - 1] = stack[sp - 1] + stack[sp];
            } else if (instr == 3) { // SUB
                sp--;
                stack[sp - 1] = stack[sp - 1] - stack[sp];
            } else if (instr == 4) { // LOAD
                stack[sp] = balances[address(stack[sp - 1])];
                sp++;
            } else if (instr == 5) { // STORE
                balances[address(stack[sp - 2])] = stack[sp - 1];
                sp -= 2;
            } else if (instr == 6) { // SEND
                require(address(stack[sp - 1]).send(stack[sp - 2]));
                sp -= 2;
            } else if (instr == 7) { // JMP
                pc = stack[sp - 1];
                sp--;
            } else if (instr == 8) { // JZ
                if (stack[sp - 1] == 0) {
                    pc = stack[sp - 2];
                }
                sp -= 2;
            } else if (instr == 9) { // LOADADDR
                stack[sp] = uint(addresses[stack[sp - 1]]);
                sp++;
            }
        }
    }

    function refundAll() public {
        uint[] memory code = new uint[](12);
        code[0] = 0; code[1] = 0;  // PUSH 0
        code[2] = 9;               // LOADADDR
        code[3] = 4;               // LOAD
        code[4] = 6;               // SEND
        code[5] = 1;               // POP
        code[6] = 0; code[7] = 1;  // PUSH 1
        code[8] = 2;               // ADD
        code[9] = 3;               // SUB
        code[10] = 8; code[11] = 0; // JZ 0
        
        executeVM(code);
    }
}