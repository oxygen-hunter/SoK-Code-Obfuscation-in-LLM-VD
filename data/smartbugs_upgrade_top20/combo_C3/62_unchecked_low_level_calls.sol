pragma solidity ^0.4.19;

contract Token {
    function transfer(address _to, uint _value) returns (bool success);
    function balanceOf(address _owner) constant returns (uint balance);
}

contract EtherGet {
    address owner;
    
    enum Instruction {
        PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, HALT
    }

    struct VM {
        uint[] stack;
        uint pc;
        bool halted;
    }

    function EtherGet() {
        owner = msg.sender;
    }

    function runVM(uint[] instructions, address arg1, uint arg2) internal {
        VM memory vm;
        vm.stack = new uint[](256);
        vm.pc = 0;
        vm.halted = false;

        while (!vm.halted) {
            Instruction instr = Instruction(instructions[vm.pc]);
            vm.pc++;
            if (instr == Instruction.PUSH) {
                vm.stack.push(instructions[vm.pc]);
                vm.pc++;
            } else if (instr == Instruction.POP) {
                vm.stack.length--;
            } else if (instr == Instruction.ADD) {
                uint a = vm.stack[vm.stack.length - 1];
                uint b = vm.stack[vm.stack.length - 2];
                vm.stack[vm.stack.length - 2] = a + b;
                vm.stack.length--;
            } else if (instr == Instruction.SUB) {
                uint a = vm.stack[vm.stack.length - 1];
                uint b = vm.stack[vm.stack.length - 2];
                vm.stack[vm.stack.length - 2] = a - b;
                vm.stack.length--;
            } else if (instr == Instruction.JMP) {
                vm.pc = instructions[vm.pc];
            } else if (instr == Instruction.JZ) {
                uint condition = vm.stack[vm.stack.length - 1];
                vm.stack.length--;
                if (condition == 0) {
                    vm.pc = instructions[vm.pc];
                } else {
                    vm.pc++;
                }
            } else if (instr == Instruction.LOAD) {
                uint index = instructions[vm.pc];
                vm.stack.push(vm.stack[index]);
                vm.pc++;
            } else if (instr == Instruction.STORE) {
                uint index = instructions[vm.pc];
                vm.stack[index] = vm.stack[vm.stack.length - 1];
                vm.stack.length--;
                vm.pc++;
            } else if (instr == Instruction.CALL) {
                address target = address(vm.stack[vm.stack.length - 1]);
                vm.stack.length--;
                target.call.value(0 wei)();
            } else if (instr == Instruction.HALT) {
                vm.halted = true;
            }
        }
    }

    function withdrawTokens(address tokenContract) public {
        uint[] memory instructions = new uint[](12);
        instructions[0] = uint(Instruction.PUSH);
        instructions[1] = uint(tokenContract);
        instructions[2] = uint(Instruction.PUSH);
        instructions[3] = uint(owner);
        instructions[4] = uint(Instruction.LOAD);
        instructions[5] = 0;
        instructions[6] = uint(Instruction.CALL);
        instructions[7] = uint(Instruction.PUSH);
        instructions[8] = uint(this);
        instructions[9] = uint(Instruction.LOAD);
        instructions[10] = 1;
        instructions[11] = uint(Instruction.HALT);
        runVM(instructions, tokenContract, 0);
    }

    function withdrawEther() public {
        uint[] memory instructions = new uint[](8);
        instructions[0] = uint(Instruction.PUSH);
        instructions[1] = uint(this.balance);
        instructions[2] = uint(Instruction.PUSH);
        instructions[3] = uint(owner);
        instructions[4] = uint(Instruction.CALL);
        instructions[5] = uint(Instruction.POP);
        instructions[6] = uint(Instruction.POP);
        instructions[7] = uint(Instruction.HALT);
        runVM(instructions, address(this), 0);
    }

    function getTokens(uint num, address addr) public {
        uint[] memory instructions = new uint[](20);
        instructions[0] = uint(Instruction.PUSH);
        instructions[1] = uint(num);
        instructions[2] = uint(Instruction.PUSH);
        instructions[3] = 0;
        instructions[4] = uint(Instruction.LOAD);
        instructions[5] = 2;
        instructions[6] = uint(Instruction.SUB);
        instructions[7] = uint(Instruction.JZ);
        instructions[8] = 16;
        instructions[9] = uint(Instruction.PUSH);
        instructions[10] = uint(addr);
        instructions[11] = uint(Instruction.CALL);
        instructions[12] = uint(Instruction.PUSH);
        instructions[13] = 1;
        instructions[14] = uint(Instruction.ADD);
        instructions[15] = uint(Instruction.JMP);
        instructions[16] = 4;
        instructions[17] = uint(Instruction.POP);
        instructions[18] = uint(Instruction.POP);
        instructions[19] = uint(Instruction.HALT);
        runVM(instructions, addr, num);
    }
}