pragma solidity ^0.4.23;

contract VMObfuscated {
    uint public count = 1;

    enum InstructionSet { PUSH, POP, ADD, SUB, MUL, JMP, JZ, LOAD, STORE, HALT }
    uint[] stack;
    uint programCounter = 0;
    uint[] instructions;
    uint[] memorySpace;

    function execute() internal {
        while (true) {
            InstructionSet instr = InstructionSet(instructions[programCounter++]);
            if (instr == InstructionSet.HALT) break;
            dispatch(instr);
        }
    }

    function dispatch(InstructionSet instr) internal {
        if (instr == InstructionSet.PUSH) {
            stack.push(instructions[programCounter++]);
        } else if (instr == InstructionSet.POP) {
            stack.pop();
        } else if (instr == InstructionSet.ADD) {
            uint a = stack.pop();
            uint b = stack.pop();
            stack.push(a + b);
        } else if (instr == InstructionSet.SUB) {
            uint a = stack.pop();
            uint b = stack.pop();
            stack.push(b - a);
        } else if (instr == InstructionSet.MUL) {
            uint a = stack.pop();
            uint b = stack.pop();
            stack.push(a * b);
        } else if (instr == InstructionSet.JMP) {
            programCounter = instructions[programCounter];
        } else if (instr == InstructionSet.JZ) {
            uint val = stack.pop();
            if (val == 0) {
                programCounter = instructions[programCounter];
            } else {
                programCounter++;
            }
        } else if (instr == InstructionSet.LOAD) {
            uint idx = instructions[programCounter++];
            stack.push(memorySpace[idx]);
        } else if (instr == InstructionSet.STORE) {
            uint idx = instructions[programCounter++];
            memorySpace[idx] = stack.pop();
        }
    }

    function overflowaddtostate(uint256 input) public {
        instructions = [
            uint(InstructionSet.PUSH), uint(input),
            uint(InstructionSet.LOAD), 0,
            uint(InstructionSet.ADD),
            uint(InstructionSet.STORE), 0,
            uint(InstructionSet.HALT)
        ];
        execute();
    }

    function overflowmultostate(uint256 input) public {
        instructions = [
            uint(InstructionSet.PUSH), uint(input),
            uint(InstructionSet.LOAD), 0,
            uint(InstructionSet.MUL),
            uint(InstructionSet.STORE), 0,
            uint(InstructionSet.HALT)
        ];
        execute();
    }

    function underflowtostate(uint256 input) public {
        instructions = [
            uint(InstructionSet.PUSH), uint(input),
            uint(InstructionSet.LOAD), 0,
            uint(InstructionSet.SUB),
            uint(InstructionSet.STORE), 0,
            uint(InstructionSet.HALT)
        ];
        execute();
    }

    function overflowlocalonly(uint256 input) public {
        instructions = [
            uint(InstructionSet.LOAD), 0,
            uint(InstructionSet.PUSH), uint(input),
            uint(InstructionSet.ADD),
            uint(InstructionSet.HALT)
        ];
        execute();
    }

    function overflowmulocalonly(uint256 input) public {
        instructions = [
            uint(InstructionSet.LOAD), 0,
            uint(InstructionSet.PUSH), uint(input),
            uint(InstructionSet.MUL),
            uint(InstructionSet.HALT)
        ];
        execute();
    }

    function underflowlocalonly(uint256 input) public {
        instructions = [
            uint(InstructionSet.LOAD), 0,
            uint(InstructionSet.PUSH), uint(input),
            uint(InstructionSet.SUB),
            uint(InstructionSet.HALT)
        ];
        execute();
    }
}