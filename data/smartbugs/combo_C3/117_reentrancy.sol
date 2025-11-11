pragma solidity ^0.4.24;

contract VM {
    uint[] private stack;
    uint private pc;
    bytes private program;
    mapping (uint => uint) private memoryStore;

    enum Opcode { PUSH, POP, ADD, SUB, LOAD, STORE, JUMP, JZ, CALL, SETBALANCE, GETBALANCE }

    function execute(bytes _program, uint[] _input) public {
        program = _program;
        stack = _input;
        pc = 0;
        
        while (pc < program.length) {
            Opcode opcode = Opcode(program[pc]);
            pc++;
            executeOpcode(opcode);
        }
    }

    function executeOpcode(Opcode opcode) private {
        if (opcode == Opcode.PUSH) {
            uint value = uint(program[pc]);
            pc++;
            stack.push(value);
        } else if (opcode == Opcode.POP) {
            stack.pop();
        } else if (opcode == Opcode.ADD) {
            uint b = stack.pop();
            uint a = stack.pop();
            stack.push(a + b);
        } else if (opcode == Opcode.SUB) {
            uint b = stack.pop();
            uint a = stack.pop();
            stack.push(a - b);
        } else if (opcode == Opcode.LOAD) {
            uint addr = stack.pop();
            stack.push(memoryStore[addr]);
        } else if (opcode == Opcode.STORE) {
            uint addr = stack.pop();
            uint value = stack.pop();
            memoryStore[addr] = value;
        } else if (opcode == Opcode.JUMP) {
            pc = uint(program[pc]);
        } else if (opcode == Opcode.JZ) {
            uint addr = uint(program[pc]);
            pc++;
            if(stack.pop() == 0) {
                pc = addr;
            }
        } else if (opcode == Opcode.CALL) {
            address to = address(stack.pop());
            uint amount = stack.pop();
            to.call.value(amount)("");
        } else if (opcode == Opcode.SETBALANCE) {
            address addr = address(stack.pop());
            uint amount = stack.pop();
            memoryStore[uint(addr)] = amount;
        } else if (opcode == Opcode.GETBALANCE) {
            address addr = address(stack.pop());
            stack.push(memoryStore[uint(addr)]);
        }
    }
}

contract Reentrancy_cross_function {
    VM private vm;

    constructor() public {
        vm = new VM();
    }

    function transfer(address to, uint amount) public {
        bytes memory program = hex"0A000B0C020C050A00040C01";  
        uint[] memory input = new uint[](3);
        input[0] = uint(msg.sender);
        input[1] = uint(to);
        input[2] = amount;
        vm.execute(program, input);
    }

    function withdrawBalance() public {
        bytes memory program = hex"0D000A00030E000F"; 
        uint[] memory input = new uint[](1);
        input[0] = uint(msg.sender);
        vm.execute(program, input);
    }
}