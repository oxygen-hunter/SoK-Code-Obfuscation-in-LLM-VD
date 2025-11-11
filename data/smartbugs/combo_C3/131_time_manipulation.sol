pragma solidity ^0.4.15;

contract EtherLotto {
    uint constant TICKET_AMOUNT = 10;
    uint constant FEE_AMOUNT = 1;
    address public bank;
    uint public pot;

    enum Opcode { NOOP, PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET }

    struct Instruction {
        Opcode opcode;
        uint operand;
    }

    Instruction[] public program;
    uint[] stack;
    uint pc;

    function EtherLotto() {
        bank = msg.sender;
        program.push(Instruction(Opcode.PUSH, uint(msg.sender))); // Initialize bank
        program.push(Instruction(Opcode.STORE, uint(bank)));
    }

    function execute() internal {
        while (pc < program.length) {
            Instruction memory instr = program[pc];
            pc++;

            if (instr.opcode == Opcode.PUSH) {
                stack.push(instr.operand);
            } else if (instr.opcode == Opcode.POP) {
                stack.length--;
            } else if (instr.opcode == Opcode.ADD) {
                uint b = stack[stack.length - 1];
                uint a = stack[stack.length - 2];
                stack[stack.length - 2] = a + b;
                stack.length--;
            } else if (instr.opcode == Opcode.SUB) {
                uint b = stack[stack.length - 1];
                uint a = stack[stack.length - 2];
                stack[stack.length - 2] = a - b;
                stack.length--;
            } else if (instr.opcode == Opcode.JMP) {
                pc = instr.operand;
            } else if (instr.opcode == Opcode.JZ) {
                uint condition = stack[stack.length - 1];
                stack.length--;
                if (condition == 0) {
                    pc = instr.operand;
                }
            } else if (instr.opcode == Opcode.LOAD) {
                stack.push(instr.operand);
            } else if (instr.opcode == Opcode.STORE) {
                require(stack.length > 0);
                uint value = stack[stack.length - 1];
                stack.length--;
                if (instr.operand == uint(bank)) {
                    bank = address(value);
                }
            } else if (instr.opcode == Opcode.CALL) {
                require(stack.length > 0);
                uint functionSelector = stack[stack.length - 1];
                stack.length--;
                if (functionSelector == 0) {
                    playInternal();
                }
            } else if (instr.opcode == Opcode.RET) {
                break;
            }
        }
    }

    function play() payable {
        program.push(Instruction(Opcode.PUSH, msg.value));
        program.push(Instruction(Opcode.PUSH, TICKET_AMOUNT));
        program.push(Instruction(Opcode.SUB, 0));
        program.push(Instruction(Opcode.JZ, pc + 7));
        program.push(Instruction(Opcode.PUSH, uint(0)));
        program.push(Instruction(Opcode.RET, 0));
        program.push(Instruction(Opcode.PUSH, uint(this)));
        program.push(Instruction(Opcode.LOAD, pot));
        program.push(Instruction(Opcode.ADD, 0));
        program.push(Instruction(Opcode.STORE, pot));
        program.push(Instruction(Opcode.CALL, 0));
        pc = 0;
        execute();
    }

    function playInternal() internal {
        uint random = uint(sha3(block.timestamp)) % 2;
        if (random == 0) {
            bank.transfer(FEE_AMOUNT);
            msg.sender.transfer(pot - FEE_AMOUNT);
            pot = 0;
        }
    }
}