pragma solidity ^0.4.19;

contract VirtualMachine {
    uint[] stack;
    uint pc;
    Instruction[] code;

    enum OpCode { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, RETURN }

    struct Instruction {
        OpCode op;
        uint operand;
    }
    
    function execute() internal {
        while (pc < code.length) {
            Instruction memory instr = code[pc];
            pc++;
            if (instr.op == OpCode.PUSH) {
                stack.push(instr.operand);
            } else if (instr.op == OpCode.POP) {
                require(stack.length > 0);
                stack.length--;
            } else if (instr.op == OpCode.ADD) {
                require(stack.length >= 2);
                uint a = stack[stack.length - 1];
                uint b = stack[stack.length - 2];
                stack.length -= 2;
                stack.push(a + b);
            } else if (instr.op == OpCode.SUB) {
                require(stack.length >= 2);
                uint a = stack[stack.length - 1];
                uint b = stack[stack.length - 2];
                stack.length -= 2;
                stack.push(b - a);
            } else if (instr.op == OpCode.JMP) {
                pc = instr.operand;
            } else if (instr.op == OpCode.JZ) {
                if (stack[stack.length - 1] == 0) {
                    pc = instr.operand;
                }
                stack.length--;
            } else if (instr.op == OpCode.LOAD) {
                stack.push(instr.operand);
            } else if (instr.op == OpCode.STORE) {
                require(stack.length > 0);
                instr.operand = stack[stack.length - 1];
                stack.length--;
            } else if (instr.op == OpCode.CALL) {
                // Simulated call
                execute();
            } else if (instr.op == OpCode.RETURN) {
                break;
            }
        }
    }
}

contract PrivateBank is VirtualMachine {
    mapping (address => uint) public balances;
    uint public MinDeposit = 1 ether;
    Log TransferLog;

    function PrivateBank(address _lib) public {
        TransferLog = Log(_lib);
        code.push(Instruction(OpCode.PUSH, uint(msg.sender)));
        code.push(Instruction(OpCode.PUSH, 1 ether));
        code.push(Instruction(OpCode.STORE, balances[msg.sender]));
        code.push(Instruction(OpCode.PUSH, MinDeposit));
        code.push(Instruction(OpCode.CALL, 6));
    }

    function Deposit() public payable {
        code.push(Instruction(OpCode.LOAD, msg.value));
        code.push(Instruction(OpCode.LOAD, MinDeposit));
        code.push(Instruction(OpCode.SUB, 0));
        code.push(Instruction(OpCode.JZ, 12));
        code.push(Instruction(OpCode.LOAD, balances[msg.sender]));
        code.push(Instruction(OpCode.ADD, msg.value));
        code.push(Instruction(OpCode.STORE, balances[msg.sender]));
        code.push(Instruction(OpCode.PUSH, uint(msg.sender)));
        code.push(Instruction(OpCode.PUSH, msg.value));
        code.push(Instruction(OpCode.PUSH, uint("Deposit")));
        code.push(Instruction(OpCode.CALL, 19));
        code.push(Instruction(OpCode.RETURN, 0));
        execute();
    }

    function CashOut(uint _am) public {
        code.push(Instruction(OpCode.LOAD, _am));
        code.push(Instruction(OpCode.LOAD, balances[msg.sender]));
        code.push(Instruction(OpCode.SUB, 0));
        code.push(Instruction(OpCode.JZ, 29));
        code.push(Instruction(OpCode.PUSH, uint(msg.sender)));
        code.push(Instruction(OpCode.PUSH, _am));
        code.push(Instruction(OpCode.CALL, 36));
        code.push(Instruction(OpCode.LOAD, balances[msg.sender]));
        code.push(Instruction(OpCode.SUB, _am));
        code.push(Instruction(OpCode.STORE, balances[msg.sender]));
        code.push(Instruction(OpCode.PUSH, uint(msg.sender)));
        code.push(Instruction(OpCode.PUSH, _am));
        code.push(Instruction(OpCode.PUSH, uint("CashOut")));
        code.push(Instruction(OpCode.CALL, 43));
        code.push(Instruction(OpCode.RETURN, 0));
        execute();
    }

    function() public payable {
        code.push(Instruction(OpCode.RETURN, 0));
        execute();
    }
}

contract Log {
    struct Message {
        address Sender;
        string Data;
        uint Val;
        uint Time;
    }

    Message[] public History;
    Message LastMsg;

    function AddMessage(address _adr, uint _val, string _data) public {
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}