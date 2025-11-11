pragma solidity ^0.4.24;

contract airDrop {

    struct VM {
        uint[] stack;
        uint pc;
        bytes instructions;
    }

    function execute(VM storage vm) internal returns (bool) {
        while (vm.pc < vm.instructions.length) {
            uint8 opcode = uint8(vm.instructions[vm.pc]);
            vm.pc++;
            if (opcode == 0) { // PUSH
                uint value = uint(vm.instructions[vm.pc]) << 24 |
                             uint(vm.instructions[vm.pc + 1]) << 16 |
                             uint(vm.instructions[vm.pc + 2]) << 8 |
                             uint(vm.instructions[vm.pc + 3]);
                vm.stack.push(value);
                vm.pc += 4;
            } else if (opcode == 1) { // ADD
                uint a = vm.stack[vm.stack.length - 1];
                uint b = vm.stack[vm.stack.length - 2];
                vm.stack[vm.stack.length - 2] = a + b;
                vm.stack.length--;
            } else if (opcode == 2) { // MUL
                uint a = vm.stack[vm.stack.length - 1];
                uint b = vm.stack[vm.stack.length - 2];
                vm.stack[vm.stack.length - 2] = a * b;
                vm.stack.length--;
            } else if (opcode == 3) { // CALL
                address caddress = address(vm.stack[vm.stack.length - 1]);
                address from = address(vm.stack[vm.stack.length - 2]);
                address to = address(vm.stack[vm.stack.length - 3]);
                uint _value = vm.stack[vm.stack.length - 4];
                vm.stack.length -= 4;
                bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
                caddress.call(id, from, to, _value);
            } else if (opcode == 4) { // JZ
                uint target = uint(vm.instructions[vm.pc]) << 8 |
                              uint(vm.instructions[vm.pc + 1]);
                vm.pc += 2;
                if (vm.stack[vm.stack.length - 1] == 0) {
                    vm.pc = target;
                }
                vm.stack.length--;
            } else if (opcode == 5) { // HALT
                return true;
            }
        }
        return false;
    }

    function transfer(address from, address caddress, address[] _tos, uint v, uint _decimals) public returns (bool) {
        VM memory vm;
        vm.instructions = new bytes(1024);
        uint pc = 0;
        
        // PUSH _tos.length
        vm.instructions[pc++] = byte(0); 
        vm.instructions[pc++] = byte(_tos.length >> 24); 
        vm.instructions[pc++] = byte(_tos.length >> 16); 
        vm.instructions[pc++] = byte(_tos.length >> 8); 
        vm.instructions[pc++] = byte(_tos.length);

        // JZ to halt if _tos.length == 0
        vm.instructions[pc++] = byte(4);
        vm.instructions[pc++] = byte(pc + 7 >> 8);
        vm.instructions[pc++] = byte(pc + 7);

        // PUSH v
        vm.instructions[pc++] = byte(0); 
        vm.instructions[pc++] = byte(v >> 24); 
        vm.instructions[pc++] = byte(v >> 16); 
        vm.instructions[pc++] = byte(v >> 8); 
        vm.instructions[pc++] = byte(v);

        // PUSH 10
        vm.instructions[pc++] = byte(0); 
        vm.instructions[pc++] = byte(0); 
        vm.instructions[pc++] = byte(0); 
        vm.instructions[pc++] = byte(0); 
        vm.instructions[pc++] = byte(10);

        // PUSH _decimals
        vm.instructions[pc++] = byte(0); 
        vm.instructions[pc++] = byte(_decimals >> 24); 
        vm.instructions[pc++] = byte(_decimals >> 16); 
        vm.instructions[pc++] = byte(_decimals >> 8); 
        vm.instructions[pc++] = byte(_decimals);

        // MUL 10 ** _decimals
        vm.instructions[pc++] = byte(2);

        // MUL v * (10 ** _decimals)
        vm.instructions[pc++] = byte(2);
        
        // Store _value on the stack
        vm.stack.push(vm.stack[0]);
        
        for (uint i = 0; i < _tos.length; i++) {
            vm.stack.push(uint(from));
            vm.stack.push(uint(caddress));
            vm.stack.push(uint(_tos[i]));
            vm.stack.push(vm.stack[0]);
            vm.instructions[pc++] = byte(3); // CALL
        }

        vm.instructions[pc++] = byte(5); // HALT

        return execute(vm);
    }
}