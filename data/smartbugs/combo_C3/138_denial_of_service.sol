pragma solidity ^0.4.15;

contract DosAuction {
    enum Instruction { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, SEND, SETFRONTRUNNER, SETBID }
    
    address currentFrontrunner;
    uint currentBid;
    
    struct VM {
        uint[] stack;
        uint pc;
        bytes instructions;
    }
    
    function execute(VM storage vm) internal {
        while (vm.pc < vm.instructions.length) {
            Instruction instr = Instruction(vm.instructions[vm.pc]);
            vm.pc++;
            if (instr == Instruction.PUSH) {
                vm.stack.push(uint(vm.instructions[vm.pc]));
                vm.pc++;
            } else if (instr == Instruction.POP) {
                vm.stack.pop();
            } else if (instr == Instruction.ADD) {
                uint b = vm.stack[vm.stack.length - 1];
                uint a = vm.stack[vm.stack.length - 2];
                vm.stack[vm.stack.length - 2] = a + b;
                vm.stack.pop();
            } else if (instr == Instruction.SUB) {
                uint b = vm.stack[vm.stack.length - 1];
                uint a = vm.stack[vm.stack.length - 2];
                vm.stack[vm.stack.length - 2] = a - b;
                vm.stack.pop();
            } else if (instr == Instruction.JMP) {
                vm.pc = vm.stack[vm.stack.length - 1];
                vm.stack.pop();
            } else if (instr == Instruction.JZ) {
                if (vm.stack[vm.stack.length - 1] == 0) {
                    vm.pc = vm.stack[vm.stack.length - 2];
                }
                vm.stack.pop();
                vm.stack.pop();
            } else if (instr == Instruction.LOAD) {
                uint index = vm.stack[vm.stack.length - 1];
                vm.stack[vm.stack.length - 1] = (index == 0) ? uint(currentFrontrunner) : currentBid;
            } else if (instr == Instruction.STORE) {
                uint index = vm.stack[vm.stack.length - 1];
                uint value = vm.stack[vm.stack.length - 2];
                if (index == 0) currentFrontrunner = address(value);
                else currentBid = value;
                vm.stack.pop();
                vm.stack.pop();
            } else if (instr == Instruction.SEND) {
                address recipient = address(vm.stack[vm.stack.length - 1]);
                uint amount = vm.stack[vm.stack.length - 2];
                require(recipient.send(amount));
                vm.stack.pop();
                vm.stack.pop();
            } else if (instr == Instruction.SETFRONTRUNNER) {
                currentFrontrunner = address(vm.stack[vm.stack.length - 1]);
                vm.stack.pop();
            } else if (instr == Instruction.SETBID) {
                currentBid = vm.stack[vm.stack.length - 1];
                vm.stack.pop();
            }
        }
    }
    
    function bid() payable {
        VM memory vm;
        vm.instructions = hex"00"; // PUSH msg.value
        vm.instructions = abi.encodePacked(vm.instructions, uint8(Instruction.PUSH), uint8(msg.value));
        vm.instructions = abi.encodePacked(vm.instructions, uint8(Instruction.LOAD), uint8(1)); // LOAD currentBid
        vm.instructions = abi.encodePacked(vm.instructions, uint8(Instruction.SUB)); // SUB
        vm.instructions = abi.encodePacked(vm.instructions, uint8(Instruction.JZ), uint8(vm.instructions.length + 2)); // JZ to end
        vm.instructions = abi.encodePacked(vm.instructions, uint8(Instruction.LOAD), uint8(0)); // LOAD currentFrontrunner
        vm.instructions = abi.encodePacked(vm.instructions, uint8(Instruction.JZ), uint8(vm.instructions.length + 6)); // JZ if currentFrontrunner is 0
        vm.instructions = abi.encodePacked(vm.instructions, uint8(Instruction.LOAD), uint8(1)); // LOAD currentBid
        vm.instructions = abi.encodePacked(vm.instructions, uint8(Instruction.SEND)); // SEND currentFrontrunner
        vm.instructions = abi.encodePacked(vm.instructions, uint8(Instruction.SETFRONTRUNNER)); // SETFRONTRUNNER msg.sender
        vm.instructions = abi.encodePacked(vm.instructions, uint8(Instruction.SETBID)); // SETBID msg.value
        
        execute(vm);
    }
}