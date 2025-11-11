pragma solidity ^0.4.24;

contract VM {
    enum Instruction { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET }
    struct Frame {
        uint[] stack;
        uint pc;
    }
    Frame frame;
    address private owner;

    modifier onlyowner {
        _execute();
        require(owner == msg.sender);
        _;
    }

    function VM() public {
        frame.stack = new uint[](256);
        owner = msg.sender;
    }

    function _execute() internal {
        Instruction[] memory program = new Instruction[](6);
        program[0] = Instruction.LOAD;
        program[1] = Instruction.PUSH;
        program[2] = Instruction.JZ;
        program[3] = Instruction.LOAD;
        program[4] = Instruction.PUSH;
        program[5] = Instruction.RET;

        uint pc = 0;
        while (pc < program.length) {
            Instruction instr = program[pc];
            if (instr == Instruction.PUSH) {
                frame.stack.push(uint(msg.sender));
                pc++;
            } else if (instr == Instruction.LOAD) {
                frame.stack.push(uint(owner));
                pc++;
            } else if (instr == Instruction.RET) {
                return;
            } else if (instr == Instruction.JZ) {
                uint a = frame.stack[frame.stack.length - 1];
                frame.stack.length--;
                if (a == 0) {
                    pc = uint(frame.stack[frame.stack.length - 1]);
                    frame.stack.length--;
                } else {
                    pc++;
                }
            } else {
                pc++;
            }
        }
    }

    function () payable {}

    function withdraw() public onlyowner {
        owner.transfer(this.balance);
    }
}