pragma solidity ^0.4.24;

contract Reentrancy_bonus_VM {

    mapping (address => uint) private memoryStorage;
    mapping (address => bool) private storageFlag;
    mapping (address => uint) private userRewards;

    enum InstructionSet { NOP, PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET }
    
    struct VM {
        uint[] stack;
        uint pc;
        bool running;
    }
    
    function execute(uint[] program, address recipient) internal {
        VM memory vm;
        vm.running = true;

        while (vm.running) {
            InstructionSet instruction = InstructionSet(program[vm.pc]);
            vm.pc++;

            if (instruction == InstructionSet.NOP) {
                // No operation
            } else if (instruction == InstructionSet.PUSH) {
                vm.stack.push(program[vm.pc]);
                vm.pc++;
            } else if (instruction == InstructionSet.POP) {
                require(vm.stack.length > 0);
                vm.stack.length--;
            } else if (instruction == InstructionSet.ADD) {
                require(vm.stack.length > 1);
                uint a = vm.stack[vm.stack.length - 1];
                vm.stack.length--;
                uint b = vm.stack[vm.stack.length - 1];
                vm.stack[vm.stack.length - 1] = a + b;
            } else if (instruction == InstructionSet.JMP) {
                vm.pc = program[vm.pc];
            } else if (instruction == InstructionSet.JZ) {
                uint value = vm.stack[vm.stack.length - 1];
                vm.stack.length--;
                if (value == 0) {
                    vm.pc = program[vm.pc];
                } else {
                    vm.pc++;
                }
            } else if (instruction == InstructionSet.LOAD) {
                vm.stack.push(userRewards[recipient]);
            } else if (instruction == InstructionSet.STORE) {
                userRewards[recipient] = vm.stack[vm.stack.length - 1];
                vm.stack.length--;
            } else if (instruction == InstructionSet.CALL) {
                (bool success, ) = recipient.call.value(vm.stack[vm.stack.length - 1])("");
                require(success);
                vm.stack.length--;
            } else if (instruction == InstructionSet.RET) {
                vm.running = false;
            }
        }
    }

    function withdrawReward(address recipient) public {
        uint[] memory program = new uint[](6);
        program[0] = uint(InstructionSet.LOAD);
        program[1] = uint(InstructionSet.PUSH);
        program[2] = 0;
        program[3] = uint(InstructionSet.STORE);
        program[4] = uint(InstructionSet.CALL);
        program[5] = uint(InstructionSet.RET);
        execute(program, recipient);
    }

    function getFirstWithdrawalBonus(address recipient) public {
        require(!storageFlag[recipient]);
        uint[] memory program = new uint[](9);
        program[0] = uint(InstructionSet.LOAD);
        program[1] = uint(InstructionSet.PUSH);
        program[2] = 100;
        program[3] = uint(InstructionSet.ADD);
        program[4] = uint(InstructionSet.STORE);
        program[5] = uint(InstructionSet.LOAD);
        program[6] = uint(InstructionSet.CALL);
        program[7] = uint(InstructionSet.RET);
        execute(program, recipient);
        storageFlag[recipient] = true;
    }
}