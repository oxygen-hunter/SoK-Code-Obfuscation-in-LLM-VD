pragma solidity ^0.4.11;

contract IntegerOverflowMappingSym1 {
    mapping(uint256 => uint256) map;

    enum Opcode { PUSH, POP, ADD, SUB, STORE, LOAD }
    
    struct VM {
        uint256[] stack;
        uint256 pc;
        uint256[] instructions;
    }

    function execute(VM memory vm) internal {
        while (vm.pc < vm.instructions.length) {
            Opcode opcode = Opcode(vm.instructions[vm.pc]);
            vm.pc++;
            if (opcode == Opcode.PUSH) {
                vm.stack.push(vm.instructions[vm.pc++]);
            } else if (opcode == Opcode.POP) {
                vm.stack.pop();
            } else if (opcode == Opcode.ADD) {
                uint256 b = vm.stack[vm.stack.length - 1];
                vm.stack.pop();
                uint256 a = vm.stack[vm.stack.length - 1];
                vm.stack[vm.stack.length - 1] = a + b;
            } else if (opcode == Opcode.SUB) {
                uint256 b = vm.stack[vm.stack.length - 1];
                vm.stack.pop();
                uint256 a = vm.stack[vm.stack.length - 1];
                vm.stack[vm.stack.length - 1] = a - b;
            } else if (opcode == Opcode.STORE) {
                uint256 key = vm.stack[vm.stack.length - 1];
                vm.stack.pop();
                uint256 value = vm.stack[vm.stack.length - 1];
                vm.stack.pop();
                map[key] = value;
            } else if (opcode == Opcode.LOAD) {
                uint256 key = vm.stack[vm.stack.length - 1];
                vm.stack.pop();
                vm.stack.push(map[key]);
            }
        }
    }

    function init(uint256 k, uint256 v) public {
        VM memory vm;
        vm.instructions = new uint256[](8);
        vm.instructions[0] = uint256(Opcode.PUSH);
        vm.instructions[1] = k;
        vm.instructions[2] = uint256(Opcode.LOAD);
        vm.instructions[3] = uint256(Opcode.PUSH);
        vm.instructions[4] = v;
        vm.instructions[5] = uint256(Opcode.SUB);
        vm.instructions[6] = uint256(Opcode.PUSH);
        vm.instructions[7] = k;
        vm.instructions[8] = uint256(Opcode.STORE);
        execute(vm);
    }
}