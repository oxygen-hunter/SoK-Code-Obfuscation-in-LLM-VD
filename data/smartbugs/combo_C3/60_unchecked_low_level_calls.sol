pragma solidity ^0.4.10;

contract VM {
    enum Opcode { PUSH, CALL, HALT }
    struct Instruction {
        Opcode opcode;
        address operand;
    }

    address public target;
    uint public pc = 0;
    Instruction[] public program;

    function VM(address a) {
        target = a;
        program.push(Instruction(Opcode.PUSH, a));
        program.push(Instruction(Opcode.CALL, address(0)));
        program.push(Instruction(Opcode.HALT, address(0)));
    }

    function execute() public {
        while (pc < program.length) {
            Instruction storage instr = program[pc];
            if (instr.opcode == Opcode.PUSH) {
                pc++;
            } else if (instr.opcode == Opcode.CALL) {
                InstrCall(instr.operand);
                pc++;
            } else if (instr.opcode == Opcode.HALT) {
                break;
            }
        }
    }

    function InstrCall(address _target) internal {
        _target.call();
    }
}