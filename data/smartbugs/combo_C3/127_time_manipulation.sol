pragma solidity ^0.4.25;

contract TimedCrowdsale {
    
    enum Instruction { PUSH, TIMESTAMP, GE, HALT }
    
    uint256[] stack;
    uint256 pc;
    Instruction[] program;

    constructor() public {
        // Compiled instruction sequence: PUSH 1546300800, TIMESTAMP, GE, HALT
        program = [Instruction.PUSH, Instruction.TIMESTAMP, Instruction.GE, Instruction.HALT];
    }
    
    function execute() internal returns (bool) {
        while (pc < program.length) {
            Instruction instr = program[pc];
            pc++;
            
            if (instr == Instruction.PUSH) {
                stack.push(1546300800);
            } else if (instr == Instruction.TIMESTAMP) {
                stack.push(block.timestamp);
            } else if (instr == Instruction.GE) {
                uint256 b = stack[stack.length - 1];
                stack.length--;
                uint256 a = stack[stack.length - 1];
                stack.length--;
                stack.push(a >= b ? 1 : 0);
            } else if (instr == Instruction.HALT) {
                break;
            }
        }
        
        return stack[stack.length - 1] == 1;
    }
    
    function isSaleFinished() view public returns (bool) {
        pc = 0;
        stack.length = 0;
        return execute();
    }
}