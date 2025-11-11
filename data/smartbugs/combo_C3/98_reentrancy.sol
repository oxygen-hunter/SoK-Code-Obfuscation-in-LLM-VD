pragma solidity ^0.4.10;

contract EtherStore {
    uint256 public withdrawalLimit = 1 ether;
    mapping(address => uint256) public lastWithdrawTime;
    mapping(address => uint256) public balances;

    enum Instruction { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL }
    
    struct VM {
        uint256[] stack;
        uint256 pc;
    }

    function executeInstruction(VM storage vm, Instruction[] memory program) internal returns (bool) {
        while (vm.pc < program.length) {
            Instruction inst = program[vm.pc];
            if (inst == Instruction.PUSH) {
                vm.stack.push(program[++vm.pc]);
            } else if (inst == Instruction.POP) {
                vm.stack.pop();
            } else if (inst == Instruction.ADD) {
                uint256 a = vm.stack.pop();
                uint256 b = vm.stack.pop();
                vm.stack.push(a + b);
            } else if (inst == Instruction.SUB) {
                uint256 a = vm.stack.pop();
                uint256 b = vm.stack.pop();
                vm.stack.push(a - b);
            } else if (inst == Instruction.JMP) {
                vm.pc = program[++vm.pc];
                continue;
            } else if (inst == Instruction.JZ) {
                uint256 a = vm.stack.pop();
                uint256 target = program[++vm.pc];
                if (a == 0) {
                    vm.pc = target;
                    continue;
                }
            } else if (inst == Instruction.LOAD) {
                uint256 idx = program[++vm.pc];
                if (idx == 0) {
                    vm.stack.push(balances[msg.sender]);
                } else if (idx == 1) {
                    vm.stack.push(withdrawalLimit);
                } else if (idx == 2) {
                    vm.stack.push(lastWithdrawTime[msg.sender]);
                }
            } else if (inst == Instruction.STORE) {
                uint256 idx = program[++vm.pc];
                uint256 value = vm.stack.pop();
                if (idx == 0) {
                    balances[msg.sender] = value;
                } else if (idx == 2) {
                    lastWithdrawTime[msg.sender] = value;
                }
            } else if (inst == Instruction.CALL) {
                uint256 value = vm.stack.pop();
                require(msg.sender.call.value(value)());
            }
            vm.pc++;
        }
        return true;
    }

    function depositFunds() public payable {
        balances[msg.sender] += msg.value;
    }

    function withdrawFunds(uint256 _weiToWithdraw) public {
        VM memory vm;
        Instruction[] memory program = new Instruction[](30);
        program[0] = Instruction.PUSH; program[1] = _weiToWithdraw;
        program[2] = Instruction.LOAD; program[3] = 0; // balances[msg.sender]
        program[4] = Instruction.SUB;
        program[5] = Instruction.JZ; program[6] = 29;
        
        program[7] = Instruction.PUSH; program[8] = _weiToWithdraw;
        program[9] = Instruction.LOAD; program[10] = 1; // withdrawalLimit
        program[11] = Instruction.SUB;
        program[12] = Instruction.JZ; program[13] = 29;

        program[14] = Instruction.LOAD; program[15] = 2; // lastWithdrawTime[msg.sender]
        program[16] = Instruction.PUSH; program[17] = 1 weeks;
        program[18] = Instruction.ADD;
        program[19] = Instruction.PUSH; program[20] = now;
        program[21] = Instruction.SUB;
        program[22] = Instruction.JZ; program[23] = 29;

        program[24] = Instruction.PUSH; program[25] = _weiToWithdraw;
        program[26] = Instruction.CALL;

        program[27] = Instruction.PUSH; program[28] = _weiToWithdraw;
        program[29] = Instruction.LOAD; program[30] = 0; // balances[msg.sender]
        program[31] = Instruction.SUB;
        program[32] = Instruction.STORE; program[33] = 0;

        program[34] = Instruction.PUSH; program[35] = now;
        program[36] = Instruction.STORE; program[37] = 2;

        require(executeInstruction(vm, program));
    }
}