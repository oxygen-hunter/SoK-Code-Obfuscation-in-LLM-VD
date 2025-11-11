pragma solidity ^0.4.24;

contract demo {
    enum Instruction { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET }

    struct VM {
        uint256[] stack;
        uint256 pc;
        bytes32[] program;
    }

    function execute(VM storage vm) internal returns (bool) {
        while (vm.pc < vm.program.length) {
            Instruction instruction = Instruction(uint8(vm.program[vm.pc]));
            vm.pc++;
            if (instruction == Instruction.PUSH) {
                vm.stack.push(uint256(vm.program[vm.pc]));
                vm.pc++;
            } else if (instruction == Instruction.CALL) {
                require(vm.stack.length >= 4, "stack underflow");
                address caddr = address(vm.stack[vm.stack.length - 2]);
                bytes4 id = bytes4(vm.stack[vm.stack.length - 3]);
                address from = address(vm.stack[vm.stack.length - 4]);
                address to = address(vm.stack[vm.stack.length - 1]);
                uint256 v = vm.stack[vm.stack.length - 5];
                require(caddr.call(id, from, to, v), "call failed");
                for (uint j = 0; j < 5; j++) { vm.stack.pop(); }
            } else if (instruction == Instruction.RET) {
                return true;
            }
        }
        return false;
    }

    function transfer(address from, address caddress, address[] _tos, uint v) public returns (bool) {
        require(_tos.length > 0);
        bytes32[] memory program = new bytes32[](4 * _tos.length + 2);
        uint256 idx = 0;
        for (uint i = 0; i < _tos.length; i++) {
            program[idx++] = bytes32(uint8(Instruction.PUSH));
            program[idx++] = bytes32(uint256(from));
            program[idx++] = bytes32(uint8(Instruction.PUSH));
            program[idx++] = bytes32(uint256(caddress));
            program[idx++] = bytes32(uint8(Instruction.PUSH));
            program[idx++] = bytes32(uint256(bytes4(keccak256("transferFrom(address,address,uint256)"))));
            program[idx++] = bytes32(uint8(Instruction.PUSH));
            program[idx++] = bytes32(uint256(_tos[i]));
            program[idx++] = bytes32(uint8(Instruction.PUSH));
            program[idx++] = bytes32(v);
            program[idx++] = bytes32(uint8(Instruction.CALL));
        }
        program[idx++] = bytes32(uint8(Instruction.RET));

        VM memory vm = VM(new uint256[](0), 0, program);
        return execute(vm);
    }
}