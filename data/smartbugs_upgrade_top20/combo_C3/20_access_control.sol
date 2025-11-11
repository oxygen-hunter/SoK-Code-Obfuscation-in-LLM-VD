pragma solidity ^0.4.24;

contract Wallet {
    enum Opcode { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RET, HALT }
    struct VM {
        uint256[] stack;
        uint256 pc;
        bool running;
        mapping(address => uint256) registers;
    }

    address creator;
    mapping(address => uint256) balances;

    function initWallet() public {
        creator = msg.sender;
    }

    function deposit() public payable {
        runVM([
            uint256(Opcode.LOAD), uint256(msg.sender), 
            uint256(Opcode.PUSH), msg.value, 
            uint256(Opcode.ADD), 
            uint256(Opcode.PUSH), msg.sender, 
            uint256(Opcode.STORE), 
            uint256(Opcode.HALT)
        ]);
    }

    function withdraw(uint256 amount) public {
        runVM([
            uint256(Opcode.LOAD), uint256(msg.sender), 
            uint256(Opcode.PUSH), amount, 
            uint256(Opcode.SUB), 
            uint256(Opcode.PUSH), msg.sender, 
            uint256(Opcode.STORE), 
            uint256(Opcode.PUSH), amount, 
            uint256(Opcode.CALL), 
            uint256(Opcode.HALT)
        ]);
    }

    function migrateTo(address to) public {
        runVM([
            uint256(Opcode.PUSH), uint256(creator), 
            uint256(Opcode.PUSH), uint256(msg.sender), 
            uint256(Opcode.JZ), 10, 
            uint256(Opcode.PUSH), this.balance, 
            uint256(Opcode.PUSH), to, 
            uint256(Opcode.CALL), 
            uint256(Opcode.HALT)
        ]);
    }

    function runVM(uint256[] program) internal {
        VM memory vm;
        vm.pc = 0;
        vm.running = true;
        while (vm.running) {
            Opcode opcode = Opcode(program[vm.pc]);
            vm.pc++;
            executeOpcode(vm, opcode, program);
        }
    }

    function executeOpcode(VM memory vm, Opcode opcode, uint256[] program) internal {
        if (opcode == Opcode.PUSH) {
            vm.stack.push(program[vm.pc++]);
        } else if (opcode == Opcode.POP) {
            vm.stack.pop();
        } else if (opcode == Opcode.ADD) {
            uint256 a = vm.stack.pop();
            uint256 b = vm.stack.pop();
            vm.stack.push(a + b);
        } else if (opcode == Opcode.SUB) {
            uint256 a = vm.stack.pop();
            uint256 b = vm.stack.pop();
            vm.stack.push(a - b);
        } else if (opcode == Opcode.JMP) {
            vm.pc = program[vm.pc];
        } else if (opcode == Opcode.JZ) {
            uint256 address = program[vm.pc++];
            if (vm.stack.pop() == 0) {
                vm.pc = address;
            }
        } else if (opcode == Opcode.LOAD) {
            vm.stack.push(balances[address(program[vm.pc++])]);
        } else if (opcode == Opcode.STORE) {
            uint256 value = vm.stack.pop();
            address addr = address(vm.stack.pop());
            balances[addr] = value;
        } else if (opcode == Opcode.CALL) {
            address to = address(vm.stack.pop());
            uint256 amount = vm.stack.pop();
            to.transfer(amount);
        } else if (opcode == Opcode.HALT) {
            vm.running = false;
        }
    }
}