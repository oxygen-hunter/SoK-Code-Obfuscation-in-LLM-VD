pragma solidity ^0.4.24;

contract Map {
    address public owner;
    uint256[] map;

    enum Instruction { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL }

    struct VM {
        uint256[] stack;
        uint256 pc;
        Instruction[] instructions;
        uint256[] memory;
    }

    function initializeVM() internal pure returns (VM memory) {
        return VM(new uint256[](0), 0, new Instruction[](0), new uint256[](2));
    }

    function execute(VM memory vm) internal {
        while (vm.pc < vm.instructions.length) {
            Instruction instr = vm.instructions[vm.pc];
            if (instr == Instruction.PUSH) {
                vm.stack.push(vm.memory[vm.pc + 1]);
                vm.pc += 2;
            } else if (instr == Instruction.POP) {
                vm.stack.pop();
                vm.pc++;
            } else if (instr == Instruction.ADD) {
                uint256 a = vm.stack[vm.stack.length - 1];
                uint256 b = vm.stack[vm.stack.length - 2];
                vm.stack[vm.stack.length - 2] = a + b;
                vm.stack.pop();
                vm.pc++;
            } else if (instr == Instruction.SUB) {
                uint256 a = vm.stack[vm.stack.length - 1];
                uint256 b = vm.stack[vm.stack.length - 2];
                vm.stack[vm.stack.length - 2] = b - a;
                vm.stack.pop();
                vm.pc++;
            } else if (instr == Instruction.JMP) {
                vm.pc = vm.memory[vm.pc + 1];
            } else if (instr == Instruction.JZ) {
                if (vm.stack[vm.stack.length - 1] == 0) {
                    vm.pc = vm.memory[vm.pc + 1];
                } else {
                    vm.pc += 2;
                }
                vm.stack.pop();
            } else if (instr == Instruction.LOAD) {
                vm.stack.push(vm.memory[vm.memory[vm.pc + 1]]);
                vm.pc += 2;
            } else if (instr == Instruction.STORE) {
                vm.memory[vm.memory[vm.pc + 1]] = vm.stack[vm.stack.length - 1];
                vm.stack.pop();
                vm.pc += 2;
            } else if (instr == Instruction.CALL) {
                if (vm.memory[vm.pc + 1] == 0) {
                    set(vm.stack[vm.stack.length - 2], vm.stack[vm.stack.length - 1]);
                } else if (vm.memory[vm.pc + 1] == 1) {
                    vm.stack.push(get(vm.stack[vm.stack.length - 1]));
                } else if (vm.memory[vm.pc + 1] == 2) {
                    withdraw();
                }
                vm.pc += 2;
            }
        }
    }

    function set(uint256 key, uint256 value) internal {
        if (map.length <= key) {
            map.length = key + 1;
        }
        map[key] = value;
    }

    function get(uint256 key) internal view returns (uint256) {
        return map[key];
    }

    function withdraw() internal {
        require(msg.sender == owner);
        msg.sender.transfer(address(this).balance);
    }

    function interpret(uint256[] memory program) public {
        VM memory vm = initializeVM();
        vm.instructions = new Instruction[](program.length / 2);
        for (uint256 i = 0; i < program.length; i += 2) {
            vm.instructions[i / 2] = Instruction(program[i]);
            vm.memory[i / 2] = program[i + 1];
        }
        execute(vm);
    }
}