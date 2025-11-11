pragma solidity ^0.5.0;

contract Reentrancy_insecure {

    mapping (address => uint) private userBalances;

    function executeVM(bytes memory bytecode) public {
        uint[] memory stack = new uint[](256);
        uint sp = 0;
        uint pc = 0;
        uint instr;
        uint operand;
        address caller = msg.sender;

        while (pc < bytecode.length) {
            instr = uint8(bytecode[pc]);
            pc++;
            if (instr == 0x01) { // PUSH
                operand = uint8(bytecode[pc]);
                pc++;
                stack[sp] = operand;
                sp++;
            } else if (instr == 0x02) { // POP
                require(sp > 0, "Stack underflow");
                sp--;
            } else if (instr == 0x03) { // ADD
                require(sp > 1, "Stack underflow");
                sp--;
                stack[sp - 1] = stack[sp - 1] + stack[sp];
            } else if (instr == 0x04) { // SUB
                require(sp > 1, "Stack underflow");
                sp--;
                stack[sp - 1] = stack[sp - 1] - stack[sp];
            } else if (instr == 0x05) { // JMP
                operand = uint8(bytecode[pc]);
                pc = operand;
            } else if (instr == 0x06) { // JZ
                operand = uint8(bytecode[pc]);
                pc++;
                if (stack[sp - 1] == 0) {
                    pc = operand;
                }
                sp--;
            } else if (instr == 0x07) { // LOAD
                operand = uint8(bytecode[pc]);
                pc++;
                stack[sp] = operand == 0 ? userBalances[caller] : 0;
                sp++;
            } else if (instr == 0x08) { // STORE
                operand = uint8(bytecode[pc]);
                pc++;
                if (operand == 0) {
                    userBalances[caller] = stack[sp - 1];
                }
                sp--;
            } else if (instr == 0x09) { // CALL
                require(sp > 0, "Stack underflow");
                uint amount = stack[sp - 1];
                sp--;
                (bool success, ) = caller.call.value(amount)("");
                require(success);
            } else {
                revert("Invalid instruction");
            }
        }
    }

    function withdrawBalance() public {
        bytes memory bytecode = new bytes(12);
        
        bytecode[0] = byte(0x07); // LOAD userBalances[msg.sender]
        bytecode[1] = byte(0x00); // operand for LOAD (address 0)
        bytecode[2] = byte(0x09); // CALL
        bytecode[3] = byte(0x02); // POP
        bytecode[4] = byte(0x01); // PUSH
        bytecode[5] = byte(0x00); // operand for PUSH (0)
        bytecode[6] = byte(0x08); // STORE userBalances[msg.sender]
        bytecode[7] = byte(0x00); // operand for STORE (address 0)

        executeVM(bytecode);
    }
}