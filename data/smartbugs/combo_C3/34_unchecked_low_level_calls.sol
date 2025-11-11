pragma solidity ^0.4.18;

contract AirDropContract {

    function AirDropContract() public {
    }

    // Define VM instruction set
    enum Instruction { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL }

    struct VM {
        uint[] stack;
        uint pc;
        Instruction[] code;
        bool running;
    }

    modifier validAddress(address addr) {
        require(addr != address(0x0));
        require(addr != address(this));
        _;
    }

    function transfer(address contract_address, address[] tos, uint[] vs)
        public
        validAddress(contract_address)
        returns (bool)
    {
        VM memory vm;
        vm.running = true;
        vm.pc = 0;
        vm.code = new Instruction[](12 + tos.length * 5);

        // Load data into stack
        vm.code[0] = Instruction.PUSH; // tos length
        vm.code[1] = Instruction.PUSH; // vs length
        vm.code[2] = Instruction.ADD;  // add tos and vs lengths
        vm.code[3] = Instruction.JZ;   // jump if zero
        vm.code[4] = Instruction.PUSH; // tos length
        vm.code[5] = Instruction.PUSH; // vs length
        vm.code[6] = Instruction.SUB;  // compare lengths
        vm.code[7] = Instruction.JZ;   // jump if not equal

        // Iterate over tos and vs
        for (uint i = 0; i < tos.length; i++) {
            vm.code[8 + i * 5] = Instruction.PUSH; // tos[i]
            vm.code[9 + i * 5] = Instruction.PUSH; // vs[i]
            vm.code[10 + i * 5] = Instruction.PUSH; // contract_address
            vm.code[11 + i * 5] = Instruction.CALL; // call transferFrom
        }

        vm.code[8 + tos.length * 5] = Instruction.POP; // return true
        vm.code[9 + tos.length * 5] = Instruction.JMP; // end

        // Execute the VM
        while (vm.running) {
            execute(vm, contract_address, tos, vs);
        }

        return true;
    }

    function execute(VM memory vm, address contract_address, address[] tos, uint[] vs) internal {
        Instruction instr = vm.code[vm.pc];
        if (instr == Instruction.PUSH) {
            vm.stack.push(vm.pc);
        } else if (instr == Instruction.POP) {
            vm.stack.pop();
        } else if (instr == Instruction.ADD) {
            uint a = vm.stack.pop();
            uint b = vm.stack.pop();
            vm.stack.push(a + b);
        } else if (instr == Instruction.SUB) {
            uint a = vm.stack.pop();
            uint b = vm.stack.pop();
            vm.stack.push(a - b);
        } else if (instr == Instruction.JMP) {
            vm.pc = vm.stack.pop();
        } else if (instr == Instruction.JZ) {
            if (vm.stack[vm.stack.length - 1] == 0) {
                vm.pc = vm.stack.pop();
            }
        } else if (instr == Instruction.CALL) {
            bytes4 id = bytes4(keccak256("transferFrom(address,address,uint256)"));
            contract_address.call(id, msg.sender, tos[vm.stack.pop()], vs[vm.stack.pop()]);
        }
        vm.pc++;
        if (vm.pc >= vm.code.length) {
            vm.running = false;
        }
    }
}