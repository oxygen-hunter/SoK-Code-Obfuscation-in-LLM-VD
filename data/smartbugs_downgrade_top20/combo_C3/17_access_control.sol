pragma solidity ^0.4.24;

contract Proxy {
    address owner;

    enum Instruction { PUSH, POP, CALL, HALT }
    uint256[] stack;
    uint256 pc;
    
    constructor() public {
        owner = msg.sender;
    }

    function interpret(uint256[] program) internal {
        pc = 0;
        while (pc < program.length) {
            Instruction instr = Instruction(program[pc++]);
            if (instr == Instruction.PUSH) {
                uint256 value = program[pc++];
                stack.push(value);
            } else if (instr == Instruction.POP) {
                stack.pop();
            } else if (instr == Instruction.CALL) {
                address callee = address(stack[stack.length - 2]);
                bytes memory _data = new bytes(stack[stack.length - 1]);
                for (uint i = 0; i < _data.length; i++) {
                    _data[i] = byte(stack[stack.length - 2 - _data.length + i]);
                }
                require(callee.delegatecall(_data));
                for (uint i = 0; i < _data.length + 2; i++) {
                    stack.pop();
                }
            } else if (instr == Instruction.HALT) {
                break;
            }
        }
    }

    function forward(address callee, bytes _data) public {
        uint256[] memory program = new uint256[](3 + _data.length);
        program[0] = uint256(Instruction.PUSH);
        program[1] = uint256(callee);
        program[2] = uint256(_data.length);
        for (uint i = 0; i < _data.length; i++) {
            program[3 + i] = uint256(_data[i]);
        }
        program[3 + _data.length] = uint256(Instruction.CALL);
        program[4 + _data.length] = uint256(Instruction.HALT);
        interpret(program);
    }
}