pragma solidity ^0.4.0;
contract EtherBank {
    mapping (address => uint) userBalances;

    enum Opcode { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, THROW }
    struct Instruction {
        Opcode opcode;
        uint operand;
    }

    function execute(Instruction[] instructions) internal {
        uint[] memory stack = new uint[](256);
        uint sp = 0;
        uint pc = 0;

        while (pc < instructions.length) {
            Instruction memory ins = instructions[pc];
            pc++;
            if (ins.opcode == Opcode.PUSH) {
                stack[sp++] = ins.operand;
            } else if (ins.opcode == Opcode.POP) {
                sp--;
            } else if (ins.opcode == Opcode.ADD) {
                stack[sp - 2] += stack[sp - 1];
                sp--;
            } else if (ins.opcode == Opcode.SUB) {
                stack[sp - 2] -= stack[sp - 1];
                sp--;
            } else if (ins.opcode == Opcode.JMP) {
                pc = ins.operand;
            } else if (ins.opcode == Opcode.JZ) {
                if (stack[--sp] == 0) {
                    pc = ins.operand;
                }
            } else if (ins.opcode == Opcode.LOAD) {
                stack[sp++] = userBalances[address(stack[--sp])];
            } else if (ins.opcode == Opcode.STORE) {
                userBalances[address(stack[sp - 2])] = stack[sp - 1];
                sp -= 2;
            } else if (ins.opcode == Opcode.CALL) {
                if (!(address(stack[sp - 2]).call.value(stack[sp - 1])())) {
                    pc = ins.operand;
                }
                sp -= 2;
            } else if (ins.opcode == Opcode.THROW) {
                throw;
            }
        }
    }

    function getBalance(address user) constant returns(uint) {
        Instruction[] memory instructions = new Instruction[](2);
        instructions[0] = Instruction(Opcode.PUSH, uint(user));
        instructions[1] = Instruction(Opcode.LOAD, 0);
        execute(instructions);
        return instructions[instructions.length - 1].operand;
    }

    function addToBalance() {
        Instruction[] memory instructions = new Instruction[](4);
        instructions[0] = Instruction(Opcode.PUSH, uint(msg.sender));
        instructions[1] = Instruction(Opcode.LOAD, 0);
        instructions[2] = Instruction(Opcode.PUSH, msg.value);
        instructions[3] = Instruction(Opcode.ADD, 0);
        instructions[4] = Instruction(Opcode.PUSH, uint(msg.sender));
        instructions[5] = Instruction(Opcode.STORE, 0);
        execute(instructions);
    }

    function withdrawBalance() {
        Instruction[] memory instructions = new Instruction[](9);
        instructions[0] = Instruction(Opcode.PUSH, uint(msg.sender));
        instructions[1] = Instruction(Opcode.LOAD, 0);
        instructions[2] = Instruction(Opcode.PUSH, uint(msg.sender));
        instructions[3] = Instruction(Opcode.PUSH, instructions.length); // error location
        instructions[4] = Instruction(Opcode.CALL, 0);
        instructions[5] = Instruction(Opcode.JZ, 6);
        instructions[6] = Instruction(Opcode.THROW, 0);
        instructions[7] = Instruction(Opcode.PUSH, uint(msg.sender));
        instructions[8] = Instruction(Opcode.PUSH, 0);
        instructions[9] = Instruction(Opcode.STORE, 0);
        execute(instructions);
    }
}