pragma solidity ^0.4.19;

contract IntegerOverflowVM {
    uint public count = 2;

    enum Opcode { PUSH, MUL, LOAD, STORE, HALT }

    struct Instruction {
        Opcode opcode;
        uint operand;
    }

    Instruction[] public program;
    uint[] public stack;
    uint public programCounter;

    function IntegerOverflowVM() public {
        program.push(Instruction(Opcode.LOAD, 0)); 
        program.push(Instruction(Opcode.PUSH, 2)); 
        program.push(Instruction(Opcode.MUL, 0)); 
        program.push(Instruction(Opcode.STORE, 0)); 
        program.push(Instruction(Opcode.HALT, 0));
    }
    
    function execute(uint256 input) public {
        stack.push(input);
        programCounter = 0;
        while (true) {
            Instruction memory instr = program[programCounter];

            if (instr.opcode == Opcode.PUSH) {
                stack.push(instr.operand);
            } else if (instr.opcode == Opcode.MUL) {
                uint b = stack[stack.length - 1];
                stack.length--;
                uint a = stack[stack.length - 1];
                stack.length--;
                stack.push(a * b);
            } else if (instr.opcode == Opcode.LOAD) {
                stack.push(count);
            } else if (instr.opcode == Opcode.STORE) {
                count = stack[stack.length - 1];
                stack.length--;
            } else if (instr.opcode == Opcode.HALT) {
                break;
            } 
            programCounter++;
        }
    }

    function run(uint256 input) public {
        execute(input);
    }
}