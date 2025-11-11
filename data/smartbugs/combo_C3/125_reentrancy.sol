pragma solidity ^0.4.19;

contract BANK_SAFE {
    mapping (address => uint256) public balances;
    uint public MinSum;
    LogFile Log;
    bool intitalized;

    enum OpCode { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, SET, INIT, DEPOSIT, COLLECT }
    uint[] stack;
    uint pc;

    function execute(uint[] program) internal {
        pc = 0;
        while (pc < program.length) {
            OpCode op = OpCode(program[pc]);
            if (op == OpCode.PUSH) {
                stack.push(program[++pc]);
            } else if (op == OpCode.POP) {
                stack.length--;
            } else if (op == OpCode.ADD) {
                uint b = stack[stack.length - 1];
                uint a = stack[stack.length - 2];
                stack[stack.length - 2] = a + b;
                stack.length--;
            } else if (op == OpCode.SUB) {
                uint b = stack[stack.length - 1];
                uint a = stack[stack.length - 2];
                stack[stack.length - 2] = a - b;
                stack.length--;
            } else if (op == OpCode.JMP) {
                pc = program[++pc] - 1;
            } else if (op == OpCode.JZ) {
                if (stack[stack.length - 1] == 0) {
                    pc = program[++pc] - 1;
                } else {
                    pc++;
                }
                stack.length--;
            } else if (op == OpCode.LOAD) {
                uint addr = stack[stack.length - 1];
                stack[stack.length - 1] = balances[address(addr)];
            } else if (op == OpCode.STORE) {
                uint addr = stack[stack.length - 1];
                balances[address(addr)] = stack[stack.length - 2];
                stack.length -= 2;
            } else if (op == OpCode.CALL) {
                uint addr = stack[stack.length - 1];
                uint amount = stack[stack.length - 2];
                if (address(addr).call.value(amount)()) {
                    balances[address(addr)] -= amount;
                    Log.AddMessage(address(addr), amount, "Collect");
                }
                stack.length -= 2;
            } else if (op == OpCode.SET) {
                MinSum = stack[stack.length - 1];
                stack.length--;
            } else if (op == OpCode.INIT) {
                intitalized = true;
            }
            pc++;
        }
    }

    function SetMinSum(uint _val) public {
        execute([uint(OpCode.PUSH), uint(intitalized), uint(OpCode.JZ), 6, uint(OpCode.PUSH), _val, uint(OpCode.SET)]);
    }

    function SetLogFile(address _log) public {
        execute([uint(OpCode.PUSH), uint(intitalized), uint(OpCode.JZ), 6, uint(OpCode.PUSH), uint(_log), uint(OpCode.PUSH), uint(1), uint(OpCode.STORE)]);
    }

    function Initialized() public {
        execute([uint(OpCode.INIT)]);
    }

    function Deposit() public payable {
        execute([uint(OpCode.PUSH), uint(msg.sender), uint(OpCode.LOAD), uint(OpCode.PUSH), msg.value, uint(OpCode.ADD), uint(OpCode.PUSH), uint(msg.sender), uint(OpCode.STORE), uint(OpCode.PUSH), uint(msg.sender), uint(OpCode.PUSH), msg.value, uint(OpCode.PUSH), uint(2), uint(OpCode.CALL)]);
    }

    function Collect(uint _am) public payable {
        execute([
            uint(OpCode.PUSH), uint(msg.sender), uint(OpCode.LOAD), 
            uint(OpCode.PUSH), MinSum, uint(OpCode.SUB), uint(OpCode.JZ), 14,
            uint(OpCode.PUSH), uint(msg.sender), uint(OpCode.LOAD),
            uint(OpCode.PUSH), _am, uint(OpCode.SUB), uint(OpCode.JZ), 14,
            uint(OpCode.PUSH), uint(msg.sender), uint(OpCode.PUSH), _am, uint(OpCode.CALL)
        ]);
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