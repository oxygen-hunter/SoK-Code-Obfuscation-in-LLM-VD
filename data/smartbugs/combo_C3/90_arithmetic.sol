pragma solidity ^0.4.10;

contract IntegerOverflowAdd {
    mapping (address => uint256) public balanceOf;

    enum Opcode {
        LOAD, STORE, PUSH, POP, ADD, SUB, JMP, JZ, HALT
    }

    struct VM {
        uint256[] stack;
        uint256 pc;
        bool running;
        uint256[] memory;
    }

    function execute(uint256[] memory code, address _to, uint256 _value) internal {
        VM memory vm;
        vm.stack = new uint256[](256);
        vm.memory = new uint256[](256);
        vm.pc = 0;
        vm.running = true;

        while (vm.running) {
            Opcode opcode = Opcode(code[vm.pc]);
            vm.pc++;
            if (opcode == Opcode.LOAD) {
                uint256 idx = code[vm.pc++];
                vm.stack.push(vm.memory[idx]);
            } else if (opcode == Opcode.STORE) {
                uint256 idx = code[vm.pc++];
                vm.memory[idx] = vm.stack.pop();
            } else if (opcode == Opcode.PUSH) {
                uint256 val = code[vm.pc++];
                vm.stack.push(val);
            } else if (opcode == Opcode.POP) {
                vm.stack.pop();
            } else if (opcode == Opcode.ADD) {
                uint256 a = vm.stack.pop();
                uint256 b = vm.stack.pop();
                vm.stack.push(a + b);
            } else if (opcode == Opcode.SUB) {
                uint256 a = vm.stack.pop();
                uint256 b = vm.stack.pop();
                vm.stack.push(b - a);
            } else if (opcode == Opcode.JMP) {
                vm.pc = code[vm.pc];
            } else if (opcode == Opcode.JZ) {
                uint256 condition = vm.stack.pop();
                if (condition == 0) {
                    vm.pc = code[vm.pc];
                } else {
                    vm.pc++;
                }
            } else if (opcode == Opcode.HALT) {
                vm.running = false;
            }
        }
    }

    function transfer(address _to, uint256 _value) public {
        uint256[] memory code = new uint256[](15);
        code[0] = uint256(Opcode.PUSH);
        code[1] = uint256(msg.sender);
        code[2] = uint256(Opcode.LOAD);
        code[3] = 0;
        code[4] = uint256(Opcode.PUSH);
        code[5] = _value;
        code[6] = uint256(Opcode.SUB);
        code[7] = uint256(Opcode.JZ);
        code[8] = 14;
        code[9] = uint256(Opcode.STORE);
        code[10] = 0;
        code[11] = uint256(Opcode.ADD);
        code[12] = uint256(Opcode.HALT);

        execute(code, _to, _value);
    }
}