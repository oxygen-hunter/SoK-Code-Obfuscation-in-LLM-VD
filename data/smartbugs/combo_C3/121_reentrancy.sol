pragma solidity ^0.4.19;

contract PRIVATE_ETH_CELL {
    mapping(address => uint256) public balances;
    uint public MinSum;
    LogFile Log;
    bool intitalized;
    
    enum OpCode { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, HALT }
    
    struct Instruction {
        OpCode opcode;
        uint256 operand;
    }
    
    Instruction[] public program;
    uint256[] public stack;
    uint256 public pc;
    
    function runVM() internal {
        while (pc < program.length) {
            Instruction memory inst = program[pc];
            pc++;
            execute(inst);
        }
    }
    
    function execute(Instruction memory inst) internal {
        if (inst.opcode == OpCode.PUSH) {
            stack.push(inst.operand);
        } else if (inst.opcode == OpCode.POP) {
            stack.pop();
        } else if (inst.opcode == OpCode.ADD) {
            uint256 a = stack.pop();
            uint256 b = stack.pop();
            stack.push(a + b);
        } else if (inst.opcode == OpCode.SUB) {
            uint256 a = stack.pop();
            uint256 b = stack.pop();
            stack.push(a - b);
        } else if (inst.opcode == OpCode.JMP) {
            pc = inst.operand;
        } else if (inst.opcode == OpCode.JZ) {
            uint256 value = stack.pop();
            if (value == 0) {
                pc = inst.operand;
            }
        } else if (inst.opcode == OpCode.LOAD) {
            stack.push(inst.operand);
            stack.push(balances[msg.sender]);
        } else if (inst.opcode == OpCode.STORE) {
            balances[msg.sender] = stack.pop();
        } else if (inst.opcode == OpCode.CALL) {
            if (stack.pop() == 1) {
                msg.sender.call.value(stack.pop())();
            }
        } else if (inst.opcode == OpCode.HALT) {
            return;
        }
    }

    function SetMinSum(uint _val) public {
        program.push(Instruction(OpCode.PUSH, uint256(intitalized)));
        program.push(Instruction(OpCode.JZ, 4));
        program.push(Instruction(OpCode.HALT, 0));
        program.push(Instruction(OpCode.PUSH, _val));
        program.push(Instruction(OpCode.STORE, uint256(MinSum)));
        runVM();
    }

    function SetLogFile(address _log) public {
        program.push(Instruction(OpCode.PUSH, uint256(intitalized)));
        program.push(Instruction(OpCode.JZ, 4));
        program.push(Instruction(OpCode.HALT, 0));
        program.push(Instruction(OpCode.PUSH, uint256(_log)));
        program.push(Instruction(OpCode.STORE, uint256(Log)));
        runVM();
    }

    function Initialized() public {
        program.push(Instruction(OpCode.PUSH, 1));
        program.push(Instruction(OpCode.STORE, uint256(intitalized)));
        runVM();
    }

    function Deposit() public payable {
        program.push(Instruction(OpCode.LOAD, 0));
        program.push(Instruction(OpCode.PUSH, msg.value));
        program.push(Instruction(OpCode.ADD, 0));
        program.push(Instruction(OpCode.STORE, 0));
        program.push(Instruction(OpCode.PUSH, msg.value));
        program.push(Instruction(OpCode.CALL, 0));
        runVM();
        Log.AddMessage(msg.sender, msg.value, "Put");
    }

    function Collect(uint _am) public payable {
        program.push(Instruction(OpCode.LOAD, 0));
        program.push(Instruction(OpCode.PUSH, uint256(MinSum)));
        program.push(Instruction(OpCode.SUB, 0));
        program.push(Instruction(OpCode.JZ, 6));
        program.push(Instruction(OpCode.LOAD, 0));
        program.push(Instruction(OpCode.PUSH, _am));
        program.push(Instruction(OpCode.SUB, 0));
        program.push(Instruction(OpCode.JZ, 10));
        program.push(Instruction(OpCode.PUSH, 1));
        program.push(Instruction(OpCode.PUSH, _am));
        program.push(Instruction(OpCode.CALL, 0));
        program.push(Instruction(OpCode.LOAD, 0));
        program.push(Instruction(OpCode.PUSH, _am));
        program.push(Instruction(OpCode.SUB, 0));
        program.push(Instruction(OpCode.STORE, 0));
        program.push(Instruction(OpCode.PUSH, _am));
        program.push(Instruction(OpCode.CALL, 0));
        runVM();
        Log.AddMessage(msg.sender, _am, "Collect");
    }

    function() public payable {
        Deposit();
    }
}

contract LogFile {
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