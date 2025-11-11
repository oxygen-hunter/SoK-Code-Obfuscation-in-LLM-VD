pragma solidity ^0.4.24;

contract SimpleWallet {
    address public owner = msg.sender;
    uint public depositsCount;

    enum Instruction { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, TRANSFER }
    
    struct VM {
        uint[] stack;
        uint pc;
        uint[] memory;
    }
    
    modifier onlyOwner {
        execute(new uint[](0));
        _;
    }
    
    function() public payable {
        uint[] memory program = new uint[](2);
        program[0] = uint(Instruction.LOAD);
        program[1] = 0;
        execute(program);
    }
    
    function withdrawAll() public onlyOwner {
        uint[] memory program = new uint[](3);
        program[0] = uint(Instruction.PUSH);
        program[1] = address(this).balance;
        program[2] = uint(Instruction.CALL);
        execute(program);
    }
    
    function withdraw(uint _value) public onlyOwner {
        uint[] memory program = new uint[](3);
        program[0] = uint(Instruction.PUSH);
        program[1] = _value;
        program[2] = uint(Instruction.TRANSFER);
        execute(program);
    }
    
    function sendMoney(address _target, uint _value) public onlyOwner {
        uint[] memory program = new uint[](5);
        program[0] = uint(Instruction.PUSH);
        program[1] = uint(_target);
        program[2] = uint(Instruction.PUSH);
        program[3] = _value;
        program[4] = uint(Instruction.CALL);
        execute(program);
    }
    
    function execute(uint[] memory program) internal {
        VM memory vm;
        vm.stack = new uint[](256);
        vm.memory = new uint[](256);
        vm.pc = 0;
        
        while (vm.pc < program.length) {
            Instruction instr = Instruction(program[vm.pc]);
            if (instr == Instruction.PUSH) {
                vm.stack[vm.stack.length++] = program[++vm.pc];
            } else if (instr == Instruction.POP) {
                vm.stack.length--;
            } else if (instr == Instruction.ADD) {
                uint a = vm.stack[--vm.stack.length];
                uint b = vm.stack[--vm.stack.length];
                vm.stack[vm.stack.length++] = a + b;
            } else if (instr == Instruction.SUB) {
                uint a = vm.stack[--vm.stack.length];
                uint b = vm.stack[--vm.stack.length];
                vm.stack[vm.stack.length++] = a - b;
            } else if (instr == Instruction.JMP) {
                vm.pc = vm.stack[--vm.stack.length] - 1;
            } else if (instr == Instruction.JZ) {
                if (vm.stack[--vm.stack.length] == 0) {
                    vm.pc = vm.stack[--vm.stack.length] - 1;
                }
            } else if (instr == Instruction.LOAD) {
                depositsCount++;
            } else if (instr == Instruction.STORE) {
                vm.memory[vm.stack[--vm.stack.length]] = vm.stack[--vm.stack.length];
            } else if (instr == Instruction.CALL) {
                address(uint160(vm.stack[--vm.stack.length])).call.value(vm.stack[--vm.stack.length])();
            } else if (instr == Instruction.TRANSFER) {
                msg.sender.transfer(vm.stack[--vm.stack.length]);
            }
            vm.pc++;
        }
    }
}