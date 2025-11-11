pragma solidity ^0.4.24;

contract Missing {
    address private owner;

    modifier onlyowner {
        require(msg.sender == owner);
        _;
    }

    function IamMissing() public {
        executeVM(createProgram());
    }

    function () payable {}

    function withdraw() public onlyowner {
        executeVM(compileWithdraw());
    }

    enum OpCode { NOP, PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RETURN, HALT }

    struct Instruction {
        OpCode opcode;
        uint operand;
    }

    uint[] stack;
    uint sp;  // Stack pointer
    uint pc;  // Program counter

    function executeVM(Instruction[] program) internal {
        sp = 0;
        pc = 0;
        stack.length = 256;  // Initialize stack size

        while (pc < program.length) {
            Instruction memory inst = program[pc];
            pc++;

            if (inst.opcode == OpCode.PUSH) {
                stack[sp++] = inst.operand;
            } else if (inst.opcode == OpCode.POP) {
                sp--;
            } else if (inst.opcode == OpCode.LOAD) {
                stack[sp++] = uint(owner);
            } else if (inst.opcode == OpCode.STORE) {
                owner = address(stack[--sp]);
            } else if (inst.opcode == OpCode.CALL) {
                address(uint160(stack[--sp])).transfer(stack[--sp]);
            } else if (inst.opcode == OpCode.HALT) {
                break;
            }
        }
    }

    function createProgram() internal pure returns (Instruction[] memory) {
        Instruction[] memory program = new Instruction[](3);
        program[0] = Instruction(OpCode.PUSH, uint(msg.sender));
        program[1] = Instruction(OpCode.STORE, 0);
        program[2] = Instruction(OpCode.HALT, 0);
        return program;
    }

    function compileWithdraw() internal view returns (Instruction[] memory) {
        Instruction[] memory program = new Instruction[](4);
        program[0] = Instruction(OpCode.LOAD, 0);
        program[1] = Instruction(OpCode.PUSH, this.balance);
        program[2] = Instruction(OpCode.CALL, 0);
        program[3] = Instruction(OpCode.HALT, 0);
        return program;
    }
}