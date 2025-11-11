pragma solidity ^0.4.2;

contract ObfuscatedVM {
    mapping (address => uint) public credit;

    function execute(bytes memory bytecode, uint[] memory memoryData) public {
        uint ptr = 0; 
        uint[] memory stack = new uint[](256); 
        uint stackPtr = 0; 
        uint opcode;
        
        while (ptr < bytecode.length) {
            opcode = uint(bytecode[ptr]);
            ptr++;
            
            if (opcode == 1) { // PUSH
                stack[stackPtr++] = uint(bytecode[ptr]);
                ptr++;
            } else if (opcode == 2) { // POP
                stackPtr--;
            } else if (opcode == 3) { // ADD
                stack[stackPtr - 2] += stack[stackPtr - 1];
                stackPtr--;
            } else if (opcode == 4) { // SUB
                stack[stackPtr - 2] -= stack[stackPtr - 1];
                stackPtr--;
            } else if (opcode == 5) { // STORE
                memoryData[stack[--stackPtr]] = stack[--stackPtr];
            } else if (opcode == 6) { // LOAD
                stack[stackPtr++] = memoryData[stack[--stackPtr]];
            } else if (opcode == 7) { // JUMP
                ptr = stack[--stackPtr];
            } else if (opcode == 8) { // JZ
                if (stack[--stackPtr] == 0) {
                    ptr = stack[--stackPtr];
                } else {
                    stackPtr--;
                }
            } else if (opcode == 9) { // CALLVALUE
                stack[stackPtr++] = msg.value;
            } else if (opcode == 10) { // MSGSENDER
                stack[stackPtr++] = uint(msg.sender);
            } else if (opcode == 11) { // CALL
                address(stack[stackPtr - 2]).call.value(stack[stackPtr - 1])();
                stackPtr -= 2;
            } else if (opcode == 12) { // CREDIT
                stack[stackPtr++] = credit[address(stack[--stackPtr])];
            } else if (opcode == 13) { // SETCREDIT
                credit[address(stack[stackPtr - 2])] = stack[stackPtr - 1];
                stackPtr -= 2;
            }
        }
    }
    
    function donate(address to) payable {
        execute(hex"0A010B0903010D0C0A010B", new uint[](0));
    }
    
    function withdraw(uint amount) {
        execute(hex"0A01070C0708020A010B0D0A010B0D0B", new uint[](amount));
    }
    
    function queryCredit(address to) returns (uint) {
        execute(hex"0A010C", new uint[](0));
        return credit[to];
    }
}