pragma solidity ^0.4.24;

contract ObfuscatedWallet {
    address _0x1;
    mapping(address => uint256) _0x2;

    enum Instructions { 
        PUSH, POP, ADD, SUB, LOAD, STORE, TRANSFER, ASSERT, REQUIRE, CALL, JUMP, JZ 
    }

    struct VM {
        uint256[] stack;
        uint256 pc;
    }

    constructor() public {
        _0x1 = msg.sender;
    }

    function _execute(uint256[] memory bytecode) internal {
        VM memory vm;
        vm.pc = 0;
        while (vm.pc < bytecode.length) {
            Instructions instruction = Instructions(bytecode[vm.pc]);
            vm.pc++;
            if (instruction == Instructions.PUSH) {
                vm.stack.push(bytecode[vm.pc]);
                vm.pc++;
            } else if (instruction == Instructions.POP) {
                vm.stack.pop();
            } else if (instruction == Instructions.ADD) {
                uint256 b = vm.stack[vm.stack.length - 1]; vm.stack.pop();
                uint256 a = vm.stack[vm.stack.length - 1]; vm.stack.pop();
                vm.stack.push(a + b);
            } else if (instruction == Instructions.SUB) {
                uint256 b = vm.stack[vm.stack.length - 1]; vm.stack.pop();
                uint256 a = vm.stack[vm.stack.length - 1]; vm.stack.pop();
                vm.stack.push(a - b);
            } else if (instruction == Instructions.LOAD) {
                address addr = address(vm.stack[vm.stack.length - 1]); vm.stack.pop();
                vm.stack.push(_0x2[addr]);
            } else if (instruction == Instructions.STORE) {
                address addr = address(vm.stack[vm.stack.length - 1]); vm.stack.pop();
                uint256 value = vm.stack[vm.stack.length - 1]; vm.stack.pop();
                _0x2[addr] = value;
            } else if (instruction == Instructions.TRANSFER) {
                uint256 amount = vm.stack[vm.stack.length - 1]; vm.stack.pop();
                address(addr).transfer(amount);
            } else if (instruction == Instructions.ASSERT) {
                uint256 b = vm.stack[vm.stack.length - 1]; vm.stack.pop();
                uint256 a = vm.stack[vm.stack.length - 1]; vm.stack.pop();
                require(a > b);
            } else if (instruction == Instructions.REQUIRE) {
                uint256 b = vm.stack[vm.stack.length - 1]; vm.stack.pop();
                uint256 a = vm.stack[vm.stack.length - 1]; vm.stack.pop();
                require(a <= b);
            } else if (instruction == Instructions.CALL) {
                address addr = address(vm.stack[vm.stack.length - 1]); vm.stack.pop();
                addr.transfer(vm.stack[vm.stack.length - 1]); vm.stack.pop();
            } else if (instruction == Instructions.JUMP) {
                vm.pc = vm.stack[vm.stack.length - 1]; vm.stack.pop();
            } else if (instruction == Instructions.JZ) {
                uint256 addr = vm.stack[vm.stack.length - 1]; vm.stack.pop();
                if (vm.stack[vm.stack.length - 1] == 0) {
                    vm.pc = addr;
                }
                vm.stack.pop();
            }
        }
    }

    function deposit() public payable {
        uint256[] memory bytecode = new uint256[](10);
        bytecode[0] = uint256(Instructions.PUSH);
        bytecode[1] = uint256(uint160(msg.sender));
        bytecode[2] = uint256(Instructions.LOAD);
        bytecode[3] = uint256(Instructions.PUSH);
        bytecode[4] = msg.value;
        bytecode[5] = uint256(Instructions.ADD);
        bytecode[6] = uint256(Instructions.PUSH);
        bytecode[7] = uint256(uint160(msg.sender));
        bytecode[8] = uint256(Instructions.STORE);
        bytecode[9] = uint256(Instructions.ASSERT);
        _execute(bytecode);
    }

    function withdraw(uint256 amount) public {
        uint256[] memory bytecode = new uint256[](13);
        bytecode[0] = uint256(Instructions.PUSH);
        bytecode[1] = amount;
        bytecode[2] = uint256(Instructions.PUSH);
        bytecode[3] = uint256(uint160(msg.sender));
        bytecode[4] = uint256(Instructions.LOAD);
        bytecode[5] = uint256(Instructions.REQUIRE);
        bytecode[6] = uint256(Instructions.PUSH);
        bytecode[7] = amount;
        bytecode[8] = uint256(Instructions.PUSH);
        bytecode[9] = uint256(uint160(msg.sender));
        bytecode[10] = uint256(Instructions.CALL);
        bytecode[11] = uint256(Instructions.PUSH);
        bytecode[12] = uint256(uint160(msg.sender));
        _execute(bytecode);
    }

    function refund() public {
        uint256[] memory bytecode = new uint256[](5);
        bytecode[0] = uint256(Instructions.PUSH);
        bytecode[1] = uint256(uint160(msg.sender));
        bytecode[2] = uint256(Instructions.LOAD);
        bytecode[3] = uint256(Instructions.PUSH);
        bytecode[4] = uint256(uint160(msg.sender));
        _execute(bytecode);
    }

    function migrateTo(address to) public {
        uint256[] memory bytecode = new uint256[](5);
        bytecode[0] = uint256(Instructions.PUSH);
        bytecode[1] = uint256(uint160(_0x1));
        bytecode[2] = uint256(Instructions.PUSH);
        bytecode[3] = uint256(uint160(msg.sender));
        bytecode[4] = uint256(Instructions.REQUIRE);
        _execute(bytecode);
        to.transfer(address(this).balance);
    }
}