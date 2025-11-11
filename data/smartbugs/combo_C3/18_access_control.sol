pragma solidity ^0.4.25;

contract Wallet {
    uint[] private bonusCodes;
    address private owner;

    enum Opcode { PUSH, POP, UPDATE, DESTROY, JZ, JMP, LOAD, STORE }
    uint private pc;
    uint[] private stack;

    constructor() public {
        bonusCodes = new uint[](0);
        owner = msg.sender;
        pc = 0;
        stack = new uint[](0);
    }

    function() public payable {
    }

    function runVM(uint[] bytecode) private {
        while (pc < bytecode.length) {
            Opcode op = Opcode(bytecode[pc]);
            pc++;
            if (op == Opcode.PUSH) {
                stack.push(bytecode[pc++]);
            } else if (op == Opcode.POP) {
                require(0 <= bonusCodes.length);
                bonusCodes.length--;
            } else if (op == Opcode.UPDATE) {
                uint idx = stack[stack.length - 2];
                uint val = stack[stack.length - 1];
                require(idx < bonusCodes.length);
                bonusCodes[idx] = val;
                stack.length -= 2;
            } else if (op == Opcode.DESTROY) {
                require(msg.sender == owner);
                selfdestruct(msg.sender);
            } else if (op == Opcode.JZ) {
                uint condition = stack[stack.length - 1];
                uint target = bytecode[pc++];
                if (condition == 0) {
                    pc = target;
                } else {
                    stack.length--;
                }
            } else if (op == Opcode.JMP) {
                pc = bytecode[pc];
            } else if (op == Opcode.LOAD) {
                uint idx = stack[stack.length - 1];
                require(idx < bonusCodes.length);
                stack[stack.length - 1] = bonusCodes[idx];
            } else if (op == Opcode.STORE) {
                uint idx = stack[stack.length - 2];
                uint val = stack[stack.length - 1];
                require(idx < bonusCodes.length);
                bonusCodes[idx] = val;
                stack.length -= 2;
            }
        }
    }

    function PushBonusCode(uint c) public {
        uint[] memory bytecode = new uint[](2);
        bytecode[0] = uint(Opcode.PUSH);
        bytecode[1] = c;
        runVM(bytecode);
        bonusCodes.push(c);
    }

    function PopBonusCode() public {
        uint[] memory bytecode = new uint[](1);
        bytecode[0] = uint(Opcode.POP);
        runVM(bytecode);
    }

    function UpdateBonusCodeAt(uint idx, uint c) public {
        uint[] memory bytecode = new uint[](4);
        bytecode[0] = uint(Opcode.PUSH);
        bytecode[1] = idx;
        bytecode[2] = uint(Opcode.PUSH);
        bytecode[3] = c;
        runVM(bytecode);
        bonusCodes[idx] = c;
    }

    function Destroy() public {
        uint[] memory bytecode = new uint[](1);
        bytecode[0] = uint(Opcode.DESTROY);
        runVM(bytecode);
    }
}