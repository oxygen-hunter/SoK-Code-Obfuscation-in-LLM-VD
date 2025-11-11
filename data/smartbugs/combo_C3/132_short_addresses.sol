pragma solidity ^0.4.11;

contract MyToken {
    mapping (address => uint) balances;
    event Transfer(address indexed _from, address indexed _to, uint256 _value);

    struct VM {
        uint[] stack;
        uint pc;
    }

    enum Opcode { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL }

    function execute(uint[] program) internal returns(uint) {
        VM memory vm;
        vm.stack = new uint[](256);
        vm.pc = 0;

        while (vm.pc < program.length) {
            Opcode opcode = Opcode(program[vm.pc]);
            uint op1;
            uint op2;
            uint res;
            vm.pc++;

            if (opcode == Opcode.PUSH) {
                vm.stack.push(program[vm.pc++]);
            } else if (opcode == Opcode.POP) {
                vm.stack.pop();
            } else if (opcode == Opcode.ADD) {
                op2 = vm.stack.pop();
                op1 = vm.stack.pop();
                res = op1 + op2;
                vm.stack.push(res);
            } else if (opcode == Opcode.SUB) {
                op2 = vm.stack.pop();
                op1 = vm.stack.pop();
                res = op1 - op2;
                vm.stack.push(res);
            } else if (opcode == Opcode.JMP) {
                vm.pc = program[vm.pc];
            } else if (opcode == Opcode.JZ) {
                if (vm.stack.pop() == 0) {
                    vm.pc = program[vm.pc];
                } else {
                    vm.pc++;
                }
            } else if (opcode == Opcode.LOAD) {
                vm.stack.push(balances[address(vm.stack.pop())]);
            } else if (opcode == Opcode.STORE) {
                balances[address(vm.stack.pop())] = vm.stack.pop();
            } else if (opcode == Opcode.CALL) {
                return vm.stack.pop();
            }
        }
        return 0;
    }

    function MyToken() {
        uint[] memory program = new uint[](6);
        program[0] = uint(Opcode.PUSH);
        program[1] = uint(tx.origin);
        program[2] = uint(Opcode.PUSH);
        program[3] = 10000;
        program[4] = uint(Opcode.STORE);
        execute(program);
    }

    function sendCoin(address to, uint amount) returns(bool sufficient) {
        uint[] memory program = new uint[](21);
        program[0] = uint(Opcode.PUSH);
        program[1] = uint(msg.sender);
        program[2] = uint(Opcode.LOAD);
        program[3] = uint(Opcode.PUSH);
        program[4] = amount;
        program[5] = uint(Opcode.SUB);
        program[6] = uint(Opcode.PUSH);
        program[7] = 0;
        program[8] = uint(Opcode.SUB);
        program[9] = uint(Opcode.JZ);
        program[10] = 20;
        program[11] = uint(Opcode.PUSH);
        program[12] = uint(msg.sender);
        program[13] = uint(Opcode.LOAD);
        program[14] = uint(Opcode.PUSH);
        program[15] = amount;
        program[16] = uint(Opcode.SUB);
        program[17] = uint(Opcode.STORE);
        program[18] = uint(Opcode.PUSH);
        program[19] = uint(to);
        program[20] = uint(Opcode.LOAD);
        program[21] = uint(Opcode.PUSH);
        program[22] = amount;
        program[23] = uint(Opcode.ADD);
        program[24] = uint(Opcode.STORE);
        program[25] = uint(Opcode.CALL);
        Transfer(msg.sender, to, amount);
        return execute(program) != 0;
    }

    function getBalance(address addr) constant returns(uint) {
        uint[] memory program = new uint[](4);
        program[0] = uint(Opcode.PUSH);
        program[1] = uint(addr);
        program[2] = uint(Opcode.LOAD);
        program[3] = uint(Opcode.CALL);
        return execute(program);
    }
}