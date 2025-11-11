pragma solidity ^0.4.25;

contract ObfuscatedVM {

    enum OpCode { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CMPGT, HALT }

    address[] stack;
    address[] creditorAddresses;
    bool win = false;
    uint counter = 0;

    function execute(uint[] program) internal {
        uint pc = 0;
        while (pc < program.length) {
            OpCode op = OpCode(program[pc]);
            pc++;
            if (op == OpCode.PUSH) {
                stack.push(address(program[pc]));
                pc++;
            } else if (op == OpCode.POP) {
                stack.length--;
            } else if (op == OpCode.ADD) {
                uint a = uint(stack[stack.length - 1]);
                stack.length--;
                uint b = uint(stack[stack.length - 1]);
                stack.length--;
                stack.push(address(a + b));
            } else if (op == OpCode.CMPGT) {
                uint a = uint(stack[stack.length - 1]);
                stack.length--;
                uint b = uint(stack[stack.length - 1]);
                stack.length--;
                stack.push(address(a > b ? 1 : 0));
            } else if (op == OpCode.JMP) {
                pc = program[pc];
            } else if (op == OpCode.JZ) {
                uint a = uint(stack[stack.length - 1]);
                stack.length--;
                if (a == 0) {
                    pc = program[pc];
                } else {
                    pc++;
                }
            } else if (op == OpCode.LOAD) {
                stack.push(creditorAddresses[program[pc]]);
                pc++;
            } else if (op == OpCode.STORE) {
                address value = stack[stack.length - 1];
                stack.length--;
                creditorAddresses[program[pc]] = value;
                pc++;
            } else if (op == OpCode.HALT) {
                break;
            }
        }
    }

    function emptyCreditors() public {
        uint[] memory program = new uint[](10);
        program[0] = uint(OpCode.PUSH);
        program[1] = 1500;
        program[2] = uint(OpCode.PUSH);
        program[3] = creditorAddresses.length;
        program[4] = uint(OpCode.CMPGT);
        program[5] = uint(OpCode.JZ);
        program[6] = 9;
        program[7] = uint(OpCode.STORE);
        program[8] = 0;
        program[9] = uint(OpCode.HALT);
        execute(program);

        if (uint(stack[stack.length - 1]) == 1) {
            creditorAddresses = new address[](0);
            win = true;
        }
    }

    function addCreditors() public returns (bool) {
        uint[] memory program = new uint[](7);
        program[0] = uint(OpCode.PUSH);
        program[1] = 350;
        program[2] = uint(OpCode.LOAD);
        program[3] = 0;
        program[4] = uint(OpCode.ADD);
        program[5] = uint(OpCode.STORE);
        program[6] = uint(OpCode.HALT);
        execute(program);

        for (uint i = 0; i < uint(stack[stack.length - 1]); i++) {
            creditorAddresses.push(msg.sender);
        }
        return true;
    }

    function iWin() public view returns (bool) {
        return win;
    }

    function numberCreditors() public view returns (uint) {
        return creditorAddresses.length;
    }
}