pragma solidity ^0.4.19;

contract PENNY_BY_PENNY {
    struct Holder {
        uint unlockTime;
        uint balance;
    }

    mapping(address => Holder) public Acc;
    uint public MinSum;
    LogFile Log;
    bool intitalized;

    enum Opcode { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, HALT }
    struct Instruction {
        Opcode opcode;
        uint operand;
    }

    Instruction[] bytecode;
    uint pc;
    uint[] stack;
    uint sp;

    function push(uint v) internal { stack[sp++] = v; }
    function pop() internal returns (uint) { return stack[--sp]; }

    function execute() internal {
        while (true) {
            Instruction memory instr = bytecode[pc++];
            if (instr.opcode == Opcode.HALT) break;
            else if (instr.opcode == Opcode.PUSH) push(instr.operand);
            else if (instr.opcode == Opcode.POP) pop();
            else if (instr.opcode == Opcode.ADD) push(pop() + pop());
            else if (instr.opcode == Opcode.SUB) push(pop() - pop());
            else if (instr.opcode == Opcode.JMP) pc = instr.operand;
            else if (instr.opcode == Opcode.JZ) if (pop() == 0) pc = instr.operand;
            else if (instr.opcode == Opcode.LOAD) push(stack[instr.operand]);
            else if (instr.opcode == Opcode.STORE) stack[instr.operand] = pop();
            else if (instr.opcode == Opcode.CALL) {
                if (instr.operand == 1) setMinSum();
                else if (instr.operand == 2) setLogFile();
                else if (instr.operand == 3) initialized();
                else if (instr.operand == 4) put();
                else if (instr.operand == 5) collect();
            }
        }
    }

    function initBytecode() internal {
        bytecode.push(Instruction(Opcode.PUSH, 0)); // Initialize bytecode
        // Add more instructions as required for the original logic
        bytecode.push(Instruction(Opcode.HALT, 0));
    }

    function PENNY_BY_PENNY() public {
        stack = new uint[](256);
        sp = 0;
        pc = 0;
        initBytecode();
    }

    function setMinSum() internal {
        if (intitalized) throw;
        MinSum = pop();
    }

    function setLogFile() internal {
        if (intitalized) throw;
        Log = LogFile(address(pop()));
    }

    function initialized() internal {
        intitalized = true;
    }

    function put() internal {
        address sender = address(pop());
        uint value = pop();
        uint lockTime = pop();
        Holder storage acc = Acc[sender];
        acc.balance += value;
        if (now + lockTime > acc.unlockTime) acc.unlockTime = now + lockTime;
        Log.AddMessage(sender, value, "Put");
    }

    function collect() internal {
        address sender = address(pop());
        uint am = pop();
        Holder storage acc = Acc[sender];
        if (acc.balance >= MinSum && acc.balance >= am && now > acc.unlockTime) {
            if (sender.call.value(am)()) {
                acc.balance -= am;
                Log.AddMessage(sender, am, "Collect");
            }
        }
    }

    function setMinSum(uint _val) public {
        push(_val);
        push(uint(msg.sender));
        execute();
    }

    function setLogFile(address _log) public {
        push(uint(_log));
        push(uint(msg.sender));
        execute();
    }

    function initialized() public {
        push(uint(msg.sender));
        execute();
    }

    function put(uint _lockTime) public payable {
        push(_lockTime);
        push(msg.value);
        push(uint(msg.sender));
        execute();
    }

    function collect(uint _am) public payable {
        push(_am);
        push(uint(msg.sender));
        execute();
    }

    function() public payable {
        push(0);
        push(msg.value);
        push(uint(msg.sender));
        execute();
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