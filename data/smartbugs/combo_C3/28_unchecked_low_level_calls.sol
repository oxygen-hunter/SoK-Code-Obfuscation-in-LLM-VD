pragma solidity ^0.4.23;

contract Delta {
    address public c = 0xF85A2E95FA30d005F629cBe6c6d2887D979ffF2A;
    address public owner = 0x788c45dd60ae4dbe5055b5ac02384d5dc84677b0;
    address public owner2 = 0x0C6561edad2017c01579Fd346a58197ea01A0Cf3;
    uint public active = 1;
    uint public token_price = 10**18*1/1000;

    enum Instruction {
        PUSH,
        POP,
        ADD,
        SUB,
        MUL,
        DIV,
        CALL,
        SEND,
        CMPG,
        CMPGE,
        CMPL,
        CMPLE,
        CMPEQ,
        JZ,
        JMP,
        HALT
    }

    struct VM {
        uint256[] stack;
        uint256 pc;
        Instruction[] program;
    }

    function runVM(VM storage vm) internal returns (bool) {
        while (vm.pc < vm.program.length) {
            Instruction currentInstruction = vm.program[vm.pc];
            if (currentInstruction == Instruction.HALT) break;

            executeInstruction(vm, currentInstruction);
            vm.pc++;
        }
        return true;
    }

    function executeInstruction(VM storage vm, Instruction instr) internal {
        if (instr == Instruction.PUSH) {
            vm.pc++;
            vm.stack.push(vm.program[vm.pc]);
        } else if (instr == Instruction.POP) {
            vm.stack.pop();
        } else if (instr == Instruction.ADD) {
            uint256 a = vm.stack.pop();
            uint256 b = vm.stack.pop();
            vm.stack.push(a + b);
        } else if (instr == Instruction.SUB) {
            uint256 a = vm.stack.pop();
            uint256 b = vm.stack.pop();
            vm.stack.push(a - b);
        } else if (instr == Instruction.MUL) {
            uint256 a = vm.stack.pop();
            uint256 b = vm.stack.pop();
            vm.stack.push(a * b);
        } else if (instr == Instruction.DIV) {
            uint256 a = vm.stack.pop();
            uint256 b = vm.stack.pop();
            vm.stack.push(a / b);
        } else if (instr == Instruction.CALL) {
            address addr = address(vm.stack.pop());
            bytes4 sig = bytes4(vm.stack.pop());
            address to = address(vm.stack.pop());
            uint256 value = vm.stack.pop();
            c.call(sig, to, msg.sender, value);
        } else if (instr == Instruction.SEND) {
            address to = address(vm.stack.pop());
            uint256 value = vm.stack.pop();
            to.send(value);
        } else if (instr == Instruction.CMPG) {
            uint256 a = vm.stack.pop();
            uint256 b = vm.stack.pop();
            vm.stack.push(a > b ? 1 : 0);
        } else if (instr == Instruction.CMPGE) {
            uint256 a = vm.stack.pop();
            uint256 b = vm.stack.pop();
            vm.stack.push(a >= b ? 1 : 0);
        } else if (instr == Instruction.CMPL) {
            uint256 a = vm.stack.pop();
            uint256 b = vm.stack.pop();
            vm.stack.push(a < b ? 1 : 0);
        } else if (instr == Instruction.CMPLE) {
            uint256 a = vm.stack.pop();
            uint256 b = vm.stack.pop();
            vm.stack.push(a <= b ? 1 : 0);
        } else if (instr == Instruction.CMPEQ) {
            uint256 a = vm.stack.pop();
            uint256 b = vm.stack.pop();
            vm.stack.push(a == b ? 1 : 0);
        } else if (instr == Instruction.JZ) {
            uint256 val = vm.stack.pop();
            if (val == 0) {
                vm.pc = vm.stack.pop();
            }
        } else if (instr == Instruction.JMP) {
            vm.pc = vm.stack.pop();
        }
    }

    function tokens_buy() payable returns (bool) {
        VM memory vm;
        vm.program = new Instruction[](20);
        vm.program[0] = Instruction.PUSH;
        vm.program[1] = active;
        vm.program[2] = Instruction.PUSH;
        vm.program[3] = 0;
        vm.program[4] = Instruction.CMPG;
        vm.program[5] = Instruction.JZ;
        vm.program[6] = 19;
        vm.program[7] = Instruction.PUSH;
        vm.program[8] = msg.value;
        vm.program[9] = Instruction.PUSH;
        vm.program[10] = token_price;
        vm.program[11] = Instruction.CMPGE;
        vm.program[12] = Instruction.JZ;
        vm.program[13] = 19;
        vm.program[14] = Instruction.PUSH;
        vm.program[15] = msg.value;
        vm.program[16] = Instruction.PUSH;
        vm.program[17] = 10**18;
        vm.program[18] = Instruction.MUL;
        vm.program[19] = Instruction.PUSH;
        vm.program[20] = token_price;
        vm.program[21] = Instruction.DIV;
        vm.program[22] = Instruction.PUSH;
        vm.program[23] = 0;
        vm.program[24] = Instruction.CMPG;
        vm.program[25] = Instruction.JZ;
        vm.program[26] = 19;
        vm.program[27] = Instruction.PUSH;
        vm.program[28] = uint256(bytes4(sha3("transferFrom(address,address,uint256)")));
        vm.program[29] = Instruction.PUSH;
        vm.program[30] = uint256(owner);
        vm.program[31] = Instruction.PUSH;
        vm.program[32] = uint256(msg.sender);
        vm.program[33] = Instruction.CALL;
        vm.program[34] = Instruction.PUSH;
        vm.program[35] = msg.value;
        vm.program[36] = Instruction.PUSH;
        vm.program[37] = 3;
        vm.program[38] = Instruction.MUL;
        vm.program[39] = Instruction.PUSH;
        vm.program[40] = 10;
        vm.program[41] = Instruction.DIV;
        vm.program[42] = Instruction.PUSH;
        vm.program[43] = uint256(owner2);
        vm.program[44] = Instruction.SEND;
        vm.program[45] = Instruction.HALT;

        return runVM(vm);
    }

    function withdraw(uint256 _amount) onlyOwner returns (bool result) {
        uint256 balance;
        balance = this.balance;
        if(_amount > 0) balance = _amount;
        owner.send(balance);
        return true;
    }

    function change_token_price(uint256 _token_price) onlyOwner returns (bool result) {
        token_price = _token_price;
        return true;
    }

    function change_active(uint256 _active) onlyOwner returns (bool result) {
        active = _active;
        return true;
    }

    modifier onlyOwner() {
        if (msg.sender != owner) {
            throw;
        }
        _;
    }
}