pragma solidity ^0.4.18;

contract ObfuscatedLotto {

    bool public payedOut = false;
    address public winner;
    uint public winAmount;
    
    enum Instruction { 
        PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, HALT
    }
    
    struct VM {
        uint[] stack;
        uint pc;
        Instruction[] instructions;
        mapping(uint => uint) memory;
    }
    
    function execute(VM storage vm) internal {
        while (vm.pc < vm.instructions.length) {
            Instruction instr = vm.instructions[vm.pc];
            if (instr == Instruction.PUSH) {
                vm.pc++;
                vm.stack.push(vm.instructions[vm.pc]);
            } else if (instr == Instruction.POP) {
                vm.stack.pop();
            } else if (instr == Instruction.ADD) {
                uint b = vm.stack.pop();
                uint a = vm.stack.pop();
                vm.stack.push(a + b);
            } else if (instr == Instruction.SUB) {
                uint b = vm.stack.pop();
                uint a = vm.stack.pop();
                vm.stack.push(a - b);
            } else if (instr == Instruction.JMP) {
                vm.pc = vm.stack.pop();
                continue;
            } else if (instr == Instruction.JZ) {
                uint addr = vm.stack.pop();
                if (vm.stack.pop() == 0) {
                    vm.pc = addr;
                    continue;
                }
            } else if (instr == Instruction.LOAD) {
                uint addr = vm.stack.pop();
                vm.stack.push(vm.memory[addr]);
            } else if (instr == Instruction.STORE) {
                uint addr = vm.stack.pop();
                uint val = vm.stack.pop();
                vm.memory[addr] = val;
            } else if (instr == Instruction.CALL) {
                if (vm.stack.pop() == 0) {
                    winner.send(winAmount);
                    payedOut = true;
                } else {
                    msg.sender.send(this.balance);
                }
            } else if (instr == Instruction.HALT) {
                break;
            }
            vm.pc++;
        }
    }
    
    function sendToWinner() public {
        VM memory vm;
        vm.instructions = [
            Instruction.PUSH, uint(payedOut ? 1 : 0),
            Instruction.JZ, 7,
            Instruction.PUSH, 0,
            Instruction.CALL,
            Instruction.HALT
        ];
        execute(vm);
    }
    
    function withdrawLeftOver() public {
        VM memory vm;
        vm.instructions = [
            Instruction.PUSH, uint(payedOut ? 0 : 1),
            Instruction.JZ, 7,
            Instruction.PUSH, 1,
            Instruction.CALL,
            Instruction.HALT
        ];
        execute(vm);
    }
}