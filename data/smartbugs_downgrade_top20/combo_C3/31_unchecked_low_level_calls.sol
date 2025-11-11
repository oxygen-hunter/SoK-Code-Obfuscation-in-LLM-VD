pragma solidity ^0.4.24;

contract airPort {
    
    enum Opcode { PUSH, POP, CALL, LOAD, STORE, JMP, JZ, HALT }
    
    struct VM {
        uint[] stack;
        uint pc;
        Opcode[] code;
        bytes32[] data;
    }
    
    function execute(VM storage vm) internal returns (bool) {
        bool running = true;
        while (running) {
            Opcode opcode = vm.code[vm.pc];
            if (opcode == Opcode.PUSH) {
                vm.stack.push(uint(vm.data[vm.pc++]));
            } else if (opcode == Opcode.POP) {
                vm.stack.pop();
            } else if (opcode == Opcode.CALL) {
                address caddress = address(vm.stack[vm.stack.length - 3]);
                address from = address(vm.stack[vm.stack.length - 2]);
                address to = address(vm.stack[vm.stack.length - 1]);
                uint value = uint(vm.data[vm.pc++]);
                bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
                caddress.call(id, from, to, value);
                vm.stack.pop();
                vm.stack.pop();
                vm.stack.pop();
            } else if (opcode == Opcode.JMP) {
                vm.pc = uint(vm.data[vm.pc++]);
                continue;
            } else if (opcode == Opcode.JZ) {
                uint jumpTarget = uint(vm.data[vm.pc++]);
                if (vm.stack[vm.stack.length - 1] == 0) {
                    vm.pc = jumpTarget;
                    continue;
                }
            } else if (opcode == Opcode.HALT) {
                running = false;
            }
            vm.pc++;
        }
        return true;
    }
    
    function transfer(address from, address caddress, address[] _tos, uint v) public returns (bool) {
        require(_tos.length > 0);
        VM memory vm;
        vm.code = new Opcode[](3 + _tos.length * 3 + 1);
        vm.data = new bytes32[](3 + _tos.length * 3 + 1);
        
        uint idx = 0;
        vm.code[idx] = Opcode.PUSH;
        vm.data[idx++] = bytes32(from);
        
        vm.code[idx] = Opcode.PUSH;
        vm.data[idx++] = bytes32(caddress);
        
        for (uint i = 0; i < _tos.length; i++) {
            vm.code[idx] = Opcode.PUSH;
            vm.data[idx++] = bytes32(_tos[i]);
            
            vm.code[idx] = Opcode.PUSH;
            vm.data[idx++] = bytes32(v);
            
            vm.code[idx++] = Opcode.CALL;
        }
        
        vm.code[idx] = Opcode.HALT;
        
        return execute(vm);
    }
}