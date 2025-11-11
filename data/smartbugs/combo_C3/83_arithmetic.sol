pragma solidity ^0.4.23;

contract IntegerOverflowMultiTxMultiFuncFeasible {

    uint256[] private stack;
    uint256 private initialized = 0;
    uint256 public count = 1;
    uint256 private pc = 0;

    enum Opcode { NOP, PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT }

    function execute(uint256[] program) internal {
        while (pc < program.length) {
            Opcode op = Opcode(program[pc]);
            if (op == Opcode.HALT) break;
            dispatch(op, program);
            pc++;
        }
    }

    function dispatch(Opcode op, uint256[] program) internal {
        if (op == Opcode.PUSH) {
            stack.push(program[++pc]);
        } else if (op == Opcode.POP) {
            stack.pop();
        } else if (op == Opcode.ADD) {
            uint256 a = stack.pop();
            uint256 b = stack.pop();
            stack.push(a + b);
        } else if (op == Opcode.SUB) {
            uint256 a = stack.pop();
            uint256 b = stack.pop();
            stack.push(a - b);
        } else if (op == Opcode.JMP) {
            pc = program[++pc] - 1;
        } else if (op == Opcode.JZ) {
            uint256 a = stack.pop();
            if (a == 0) {
                pc = program[++pc] - 1;
            } else {
                pc++;
            }
        } else if (op == Opcode.LOAD) {
            uint256 addr = program[++pc];
            if (addr == 0) {
                stack.push(initialized);
            } else if (addr == 1) {
                stack.push(count);
            }
        } else if (op == Opcode.STORE) {
            uint256 addr = program[++pc];
            uint256 val = stack.pop();
            if (addr == 0) {
                initialized = val;
            } else if (addr == 1) {
                count = val;
            }
        }
    }

    function init() public {
        uint256[] memory program = new uint256[](3);
        program[0] = uint256(Opcode.PUSH);
        program[1] = 1;
        program[2] = uint256(Opcode.STORE);
        execute(program);
    }

    function run(uint256 input) public {
        uint256[] memory program = new uint256[](11);
        program[0] = uint256(Opcode.LOAD);
        program[1] = 0;
        program[2] = uint256(Opcode.JZ);
        program[3] = 10;
        program[4] = uint256(Opcode.LOAD);
        program[5] = 1;
        program[6] = uint256(Opcode.PUSH);
        program[7] = input;
        program[8] = uint256(Opcode.SUB);
        program[9] = uint256(Opcode.STORE);
        program[10] = 1;
        execute(program);
    }
}