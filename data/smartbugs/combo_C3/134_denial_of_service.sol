pragma solidity ^0.4.25;

contract VM {
    enum Opcode { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT }
    struct Instruction { Opcode opcode; uint operand; }
    uint[] stack;
    uint[] memory;
    uint pc;
    Instruction[] program;
    uint numElements;
    uint[] array;

    constructor() public {
        stack = new uint[](0);
        memory = new uint[](0);
        pc = 0;
        program = new Instruction[](0);
        numElements = 0;
        array = new uint[](0);
    }

    function run() internal {
        while (pc < program.length) {
            Instruction memory instr = program[pc];
            execute(instr);
            if (instr.opcode != Opcode.JMP && instr.opcode != Opcode.JZ)
                pc++;
        }
    }

    function execute(Instruction memory instr) internal {
        if (instr.opcode == Opcode.PUSH) {
            stack.push(instr.operand);
        } else if (instr.opcode == Opcode.POP) {
            stack.length--;
        } else if (instr.opcode == Opcode.ADD) {
            uint a = stack[stack.length - 1];
            uint b = stack[stack.length - 2];
            stack[stack.length - 2] = a + b;
            stack.length--;
        } else if (instr.opcode == Opcode.SUB) {
            uint a = stack[stack.length - 1];
            uint b = stack[stack.length - 2];
            stack[stack.length - 2] = a - b;
            stack.length--;
        } else if (instr.opcode == Opcode.JMP) {
            pc = instr.operand - 1;
        } else if (instr.opcode == Opcode.JZ) {
            if (stack[stack.length - 1] == 0) {
                pc = instr.operand - 1;
            }
            stack.length--;
        } else if (instr.opcode == Opcode.LOAD) {
            stack.push(memory[instr.operand]);
        } else if (instr.opcode == Opcode.STORE) {
            memory[instr.operand] = stack[stack.length - 1];
            stack.length--;
        } else if (instr.opcode == Opcode.HALT) {
            pc = program.length;
        }
    }

    function insertNnumbers(uint value, uint numbers) public {
        program.push(Instruction(Opcode.PUSH, 0));
        program.push(Instruction(Opcode.STORE, 0));
        program.push(Instruction(Opcode.LOAD, 1));
        program.push(Instruction(Opcode.PUSH, numbers));
        program.push(Instruction(Opcode.SUB, 0));
        program.push(Instruction(Opcode.JZ, 11));
        program.push(Instruction(Opcode.LOAD, 0));
        program.push(Instruction(Opcode.PUSH, array.length));
        program.push(Instruction(Opcode.SUB, 0));
        program.push(Instruction(Opcode.JZ, 10));
        program.push(Instruction(Opcode.PUSH, 1));
        program.push(Instruction(Opcode.LOAD, 1));
        program.push(Instruction(Opcode.ADD, 0));
        program.push(Instruction(Opcode.JMP, 2));
        program.push(Instruction(Opcode.LOAD, 0));
        program.push(Instruction(Opcode.LOAD, 1));
        program.push(Instruction(Opcode.PUSH, value));
        program.push(Instruction(Opcode.STORE, 0));
        program.push(Instruction(Opcode.LOAD, 1));
        program.push(Instruction(Opcode.ADD, 0));
        program.push(Instruction(Opcode.STORE, 1));
        program.push(Instruction(Opcode.JMP, 2));
        program.push(Instruction(Opcode.HALT, 0));
        run();
    }

    function clear() public {
        require(numElements > 1500);
        numElements = 0;
    }

    function clearDOS() public {
        require(numElements > 1500);
        array = new uint[](0);
        numElements = 0;
    }

    function getLengthArray() public view returns(uint) {
        return numElements;
    }

    function getRealLengthArray() public view returns(uint) {
        return array.length;
    }
}