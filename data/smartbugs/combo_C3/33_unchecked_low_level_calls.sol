pragma solidity 0.4.25;

contract VirtualMachine {
    enum Opcode { PUSH, CALL, REQUIRE, HALT }
    uint256[] stack;
    uint256 pc;
    bytes instructions;

    function execute(bytes _instructions) internal {
        instructions = _instructions;
        pc = 0;
        while (pc < instructions.length) {
            Opcode opcode = Opcode(instructions[pc]);
            if (opcode == Opcode.PUSH) {
                pc++;
                uint256 value = uint256(instructions[pc]);
                stack.push(value);
            } else if (opcode == Opcode.CALL) {
                address callee = address(stack[stack.length - 1]);
                callee.call();
                stack.pop();
            } else if (opcode == Opcode.REQUIRE) {
                bool condition = stack[stack.length - 1] != 0;
                require(condition);
                stack.pop();
            } else if (opcode == Opcode.HALT) {
                break;
            }
            pc++;
        }
    }
}

contract ReturnValue is VirtualMachine {

    function callchecked(address callee) public {
        bytes memory program = new bytes(3);
        program[0] = byte(Opcode.PUSH);
        program[1] = byte(callee);
        program[2] = byte(Opcode.CALL);
        execute(program);
    }

    function callnotchecked(address callee) public {
        bytes memory program = new bytes(4);
        program[0] = byte(Opcode.PUSH);
        program[1] = byte(callee);
        program[2] = byte(Opcode.CALL);
        program[3] = byte(Opcode.HALT);
        execute(program);
    }
}