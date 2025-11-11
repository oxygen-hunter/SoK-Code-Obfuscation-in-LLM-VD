pragma solidity ^0.4.18;

contract ReentranceVM {
    mapping(address => uint) public balances;

    enum OpCode { PUSH, POP, ADD, SUB, JMP, JZ, CALL, RET, LOAD, STORE, HALT }
    struct Instruction {
        OpCode opcode;
        uint operand;
    }
    
    struct VM {
        uint[] stack;
        uint pc;
        bool running;
    }

    Instruction[] public bytecode;
    VM public vm;

    function ReentranceVM() public {
        vm = VM(new uint[](0), 0, false);
        initializeBytecode();
    }

    function initializeBytecode() internal {
        // donate(address _to)
        bytecode.push(Instruction(OpCode.PUSH, 0)); // Load _to
        bytecode.push(Instruction(OpCode.LOAD, 0)); // Load msg.value
        bytecode.push(Instruction(OpCode.ADD, 0));  // balances[_to] += msg.value
        bytecode.push(Instruction(OpCode.STORE, 0)); // Store in balances
        bytecode.push(Instruction(OpCode.RET, 0));

        // balanceOf(address _who)
        bytecode.push(Instruction(OpCode.PUSH, 0)); // Load _who
        bytecode.push(Instruction(OpCode.LOAD, 0)); // Load balances[_who]
        bytecode.push(Instruction(OpCode.RET, 0));  // Return balance

        // withdraw(uint _amount)
        bytecode.push(Instruction(OpCode.PUSH, 0)); // Load _amount
        bytecode.push(Instruction(OpCode.LOAD, 0)); // Load balances[msg.sender]
        bytecode.push(Instruction(OpCode.SUB, 0));  // Check if >= _amount
        bytecode.push(Instruction(OpCode.JZ, 0));   // Jump if not enough balance
        bytecode.push(Instruction(OpCode.CALL, 0)); // msg.sender.call.value(_amount)()
        bytecode.push(Instruction(OpCode.POP, 0));  // Discard result
        bytecode.push(Instruction(OpCode.LOAD, 0)); // Load balances[msg.sender]
        bytecode.push(Instruction(OpCode.SUB, 0));  // Subtract _amount
        bytecode.push(Instruction(OpCode.STORE, 0)); // Store new balance
        bytecode.push(Instruction(OpCode.RET, 0));

        // fallback function
        bytecode.push(Instruction(OpCode.RET, 0));
    }

    function execute() internal {
        vm.running = true;
        while(vm.running) {
            Instruction memory instr = bytecode[vm.pc];
            vm.pc++;
            executeInstruction(instr);
        }
    }

    function executeInstruction(Instruction memory instr) internal {
        if (instr.opcode == OpCode.PUSH) {
            vm.stack.push(instr.operand);
        } else if (instr.opcode == OpCode.POP) {
            require(vm.stack.length > 0);
            vm.stack.length--;
        } else if (instr.opcode == OpCode.ADD) {
            require(vm.stack.length >= 2);
            uint b = vm.stack[vm.stack.length - 1];
            uint a = vm.stack[vm.stack.length - 2];
            vm.stack[vm.stack.length - 2] = a + b;
            vm.stack.length--;
        } else if (instr.opcode == OpCode.SUB) {
            require(vm.stack.length >= 2);
            uint b = vm.stack[vm.stack.length - 1];
            uint a = vm.stack[vm.stack.length - 2];
            vm.stack[vm.stack.length - 2] = a - b;
            vm.stack.length--;
        } else if (instr.opcode == OpCode.JMP) {
            vm.pc = instr.operand;
        } else if (instr.opcode == OpCode.JZ) {
            require(vm.stack.length > 0);
            if (vm.stack[vm.stack.length - 1] == 0) {
                vm.pc = instr.operand;
            }
            vm.stack.length--;
        } else if (instr.opcode == OpCode.CALL) {
            if (!msg.sender.call.value(vm.stack[vm.stack.length - 1])()) {
                revert();
            }
            vm.stack.length--;
        } else if (instr.opcode == OpCode.LOAD) {
            vm.stack.push(balances[msg.sender]);
        } else if (instr.opcode == OpCode.STORE) {
            balances[msg.sender] = vm.stack[vm.stack.length - 1];
            vm.stack.length--;
        } else if (instr.opcode == OpCode.RET) {
            vm.running = false;
        } else if (instr.opcode == OpCode.HALT) {
            vm.running = false;
        }
    }

    function donate(address _to) public payable {
        vm.stack.push(uint(_to));
        vm.stack.push(msg.value);
        vm.pc = 0;
        execute();
    }

    function balanceOf(address _who) public view returns (uint balance) {
        vm.stack.push(uint(_who));
        vm.pc = 4;
        execute();
        return vm.stack[0];
    }

    function withdraw(uint _amount) public {
        vm.stack.push(_amount);
        vm.pc = 8;
        execute();
    }

    function() public payable {
        vm.pc = 18;
        execute();
    }
}