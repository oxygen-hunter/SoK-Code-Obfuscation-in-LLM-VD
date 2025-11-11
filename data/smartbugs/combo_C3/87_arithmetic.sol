pragma solidity ^0.4.18;

contract ObfuscatedToken {
    enum InstructionSet { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, HALT }
    struct VM {
        uint[] stack;
        uint pc;
        InstructionSet[] code;
        mapping(uint => uint) memory;
    }

    mapping(address => uint) balances;
    uint public totalSupply;

    function ObfuscatedToken(uint _initialSupply) {
        balances[msg.sender] = totalSupply = _initialSupply;
    }

    function execute(VM storage vm) internal {
        while (vm.pc < vm.code.length) {
            InstructionSet ins = vm.code[vm.pc];
            if (ins == InstructionSet.PUSH) {
                vm.pc++;
                uint val = vm.code[vm.pc];
                vm.stack.push(val);
            } else if (ins == InstructionSet.POP) {
                vm.stack.pop();
            } else if (ins == InstructionSet.ADD) {
                uint a = vm.stack[vm.stack.length - 1];
                uint b = vm.stack[vm.stack.length - 2];
                vm.stack[vm.stack.length - 2] = a + b;
                vm.stack.pop();
            } else if (ins == InstructionSet.SUB) {
                uint a = vm.stack[vm.stack.length - 1];
                uint b = vm.stack[vm.stack.length - 2];
                vm.stack[vm.stack.length - 2] = a - b;
                vm.stack.pop();
            } else if (ins == InstructionSet.JMP) {
                vm.pc = vm.stack[vm.stack.length - 1];
                vm.stack.pop();
                continue;
            } else if (ins == InstructionSet.JZ) {
                uint addr = vm.stack[vm.stack.length - 1];
                uint cond = vm.stack[vm.stack.length - 2];
                vm.stack.pop(); vm.stack.pop();
                if (cond == 0) {
                    vm.pc = addr;
                    continue;
                }
            } else if (ins == InstructionSet.LOAD) {
                uint addr = vm.stack[vm.stack.length - 1];
                vm.stack[vm.stack.length - 1] = vm.memory[addr];
            } else if (ins == InstructionSet.STORE) {
                uint addr = vm.stack[vm.stack.length - 1];
                uint val = vm.stack[vm.stack.length - 2];
                vm.memory[addr] = val;
                vm.stack.pop(); vm.stack.pop();
            } else if (ins == InstructionSet.HALT) {
                break;
            }
            vm.pc++;
        }
    }

    function transfer(address _to, uint _value) public returns (bool) {
        VM memory vm;
        vm.stack = new uint[](0);
        vm.code = new InstructionSet[](0);

        // Pushing initial values and executing the transfer
        vm.code.push(InstructionSet.PUSH);
        vm.code.push(uint(msg.sender));
        vm.code.push(InstructionSet.LOAD);
        vm.code.push(InstructionSet.PUSH);
        vm.code.push(_value);
        vm.code.push(InstructionSet.SUB);
        vm.code.push(InstructionSet.PUSH);
        vm.code.push(0);
        vm.code.push(InstructionSet.JZ);
        vm.code.push(InstructionSet.HALT);

        vm.code.push(InstructionSet.PUSH);
        vm.code.push(uint(msg.sender));
        vm.code.push(InstructionSet.LOAD);
        vm.code.push(InstructionSet.PUSH);
        vm.code.push(_value);
        vm.code.push(InstructionSet.SUB);
        vm.code.push(InstructionSet.PUSH);
        vm.code.push(uint(msg.sender));
        vm.code.push(InstructionSet.STORE);

        vm.code.push(InstructionSet.PUSH);
        vm.code.push(uint(_to));
        vm.code.push(InstructionSet.LOAD);
        vm.code.push(InstructionSet.PUSH);
        vm.code.push(_value);
        vm.code.push(InstructionSet.ADD);
        vm.code.push(InstructionSet.PUSH);
        vm.code.push(uint(_to));
        vm.code.push(InstructionSet.STORE);

        vm.code.push(InstructionSet.PUSH);
        vm.code.push(1);
        vm.code.push(InstructionSet.HALT);

        execute(vm);
        return vm.stack[vm.stack.length - 1] == 1;
    }

    function balanceOf(address _owner) public constant returns (uint balance) {
        return balances[_owner];
    }
}