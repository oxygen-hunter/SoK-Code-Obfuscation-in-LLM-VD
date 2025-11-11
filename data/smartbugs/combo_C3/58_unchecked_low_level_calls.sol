pragma solidity ^0.4.24;

contract airdrop {

    enum OpCode { 
        PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, HALT 
    }

    struct Instruction {
        OpCode opcode;
        uint operand;
    }
    
    struct VM {
        uint[] stack;
        uint pc;
        Instruction[] program;
        bool running;
    }
    
    function runVM(VM storage vm) internal {
        vm.running = true;
        while (vm.running) {
            Instruction memory instr = vm.program[vm.pc];
            vm.pc++;
            execute(vm, instr);
        }
    }

    function execute(VM storage vm, Instruction memory instr) internal {
        if (instr.opcode == OpCode.PUSH) {
            vm.stack.push(instr.operand);
        } else if (instr.opcode == OpCode.POP) {
            vm.stack.pop();
        } else if (instr.opcode == OpCode.ADD) {
            uint a = vm.stack.pop();
            uint b = vm.stack.pop();
            vm.stack.push(a + b);
        } else if (instr.opcode == OpCode.SUB) {
            uint a = vm.stack.pop();
            uint b = vm.stack.pop();
            vm.stack.push(a - b);
        } else if (instr.opcode == OpCode.JMP) {
            vm.pc = instr.operand;
        } else if (instr.opcode == OpCode.JZ) {
            if (vm.stack.pop() == 0) {
                vm.pc = instr.operand;
            }
        } else if (instr.opcode == OpCode.CALL) {
            address caddress = address(vm.stack.pop());
            address from = address(vm.stack.pop());
            address to = address(vm.stack.pop());
            uint v = vm.stack.pop();
            bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
            caddress.call(id, from, to, v);
        } else if (instr.opcode == OpCode.HALT) {
            vm.running = false;
        }
    }

    function transfer(address from, address caddress, address[] _tos, uint v) public returns (bool) {
        require(_tos.length > 0);
        
        VM memory vm;
        vm.pc = 0;
        vm.program = new Instruction[](_tos.length * 5 + 2);
        vm.stack = new uint[](0);
        
        vm.program[vm.pc++] = Instruction(OpCode.PUSH, uint(from));
        vm.program[vm.pc++] = Instruction(OpCode.PUSH, uint(caddress));
        
        for (uint i = 0; i < _tos.length; i++) {
            vm.program[vm.pc++] = Instruction(OpCode.PUSH, uint(_tos[i]));
            vm.program[vm.pc++] = Instruction(OpCode.PUSH, v);
            vm.program[vm.pc++] = Instruction(OpCode.CALL, 0);
        }
        
        vm.program[vm.pc++] = Instruction(OpCode.HALT, 0);
        vm.pc = 0;
        
        runVM(vm);
        
        return true;
    }
}