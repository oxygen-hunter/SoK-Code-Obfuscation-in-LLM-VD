pragma solidity ^0.4.23;

contract keepMyEther {
    mapping(address => uint256) private balances;

    enum Opcode { PUSH, POP, ADD, SUB, STORE, LOAD, CALL, ZERO }

    struct VM {
        uint256[] stack;
        uint256 pc;
        bool running;
    }

    modifier executeVM(Opcode[] memory program) {
        VM memory vm;
        vm.stack = new uint256[](256);
        vm.pc = 0;
        vm.running = true;

        while (vm.running) {
            Opcode instruction = program[vm.pc];
            if (instruction == Opcode.PUSH) {
                vm.pc++;
                vm.stack.push(uint256(program[vm.pc]));
            } else if (instruction == Opcode.POP) {
                vm.stack.pop();
            } else if (instruction == Opcode.ADD) {
                uint256 a = vm.stack.pop();
                uint256 b = vm.stack.pop();
                vm.stack.push(a + b);
            } else if (instruction == Opcode.SUB) {
                uint256 a = vm.stack.pop();
                uint256 b = vm.stack.pop();
                vm.stack.push(a - b);
            } else if (instruction == Opcode.STORE) {
                uint256 addr = vm.stack.pop();
                uint256 val = vm.stack.pop();
                balances[address(addr)] = val;
            } else if (instruction == Opcode.LOAD) {
                uint256 addr = vm.stack.pop();
                vm.stack.push(balances[address(addr)]);
            } else if (instruction == Opcode.CALL) {
                uint256 amount = vm.stack.pop();
                address(addr).call.value(amount)();
            } else if (instruction == Opcode.ZERO) {
                uint256 addr = vm.stack.pop();
                balances[address(addr)] = 0;
            }
            vm.pc++;
            if (vm.pc >= program.length) {
                vm.running = false;
            }
        }
        _;
    }

    function () payable public executeVM(
        [Opcode.PUSH, Opcode(uint256(msg.sender)), Opcode.PUSH, Opcode(uint256(msg.value)), Opcode.LOAD, Opcode.ADD, Opcode.STORE]
    ) {}

    function withdraw() public executeVM(
        [Opcode.PUSH, Opcode(uint256(msg.sender)), Opcode.LOAD, Opcode.PUSH, Opcode(uint256(msg.sender)), Opcode.CALL, Opcode.PUSH, Opcode(uint256(msg.sender)), Opcode.ZERO]
    ) {}
}