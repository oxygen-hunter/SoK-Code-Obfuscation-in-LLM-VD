pragma solidity ^0.4.15;

contract VirtualMachine {
    uint[] stack;
    uint pc;
    uint[] instructions;
    mapping (uint => uint) memoryStorage;

    enum Opcode { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALLVAL, THROW, HALT }

    function execute(uint[] code) internal {
        instructions = code;
        pc = 0;
        while (pc < instructions.length) {
            Opcode op = Opcode(instructions[pc++]);
            if (op == Opcode.PUSH) {
                stack.push(instructions[pc++]);
            } else if (op == Opcode.POP) {
                stack.pop();
            } else if (op == Opcode.ADD) {
                uint b = stack.pop();
                uint a = stack.pop();
                stack.push(a + b);
            } else if (op == Opcode.SUB) {
                uint b = stack.pop();
                uint a = stack.pop();
                stack.push(a - b);
            } else if (op == Opcode.JMP) {
                pc = instructions[pc];
            } else if (op == Opcode.JZ) {
                uint a = stack.pop();
                if (a == 0) {
                    pc = instructions[pc];
                } else {
                    pc++;
                }
            } else if (op == Opcode.LOAD) {
                uint addr = instructions[pc++];
                stack.push(memoryStorage[addr]);
            } else if (op == Opcode.STORE) {
                uint addr = instructions[pc++];
                memoryStorage[addr] = stack.pop();
            } else if (op == Opcode.CALLVAL) {
                uint val = stack.pop();
                if (!(msg.sender.call.value(val)())) {
                    stack.push(1); 
                } else {
                    stack.push(0);
                }
            } else if (op == Opcode.THROW) {
                revert();
            } else if (op == Opcode.HALT) {
                break;
            }
        }
    }
}

contract Reentrance is VirtualMachine {
    mapping (address => uint) userBalance;

    function getBalance(address u) constant returns(uint) {
        return userBalance[u];
    }

    function addToBalance() payable {
        userBalance[msg.sender] += msg.value;
    }

    function withdrawBalance() {
        uint[] memory program = new uint[](13);
        program[0] = uint(Opcode.PUSH);
        program[1] = uint(msg.sender);
        program[2] = uint(Opcode.LOAD);
        program[3] = uint(Opcode.PUSH);
        program[4] = uint(this);
        program[5] = uint(Opcode.CALLVAL);
        program[6] = uint(Opcode.JZ);
        program[7] = 11;
        program[8] = uint(Opcode.THROW);
        program[9] = uint(Opcode.PUSH);
        program[10] = uint(msg.sender);
        program[11] = uint(Opcode.STORE);
        program[12] = uint(Opcode.HALT);

        memoryStorage[uint(msg.sender)] = userBalance[msg.sender];
        execute(program);
        userBalance[msg.sender] = 0;
    }
}