pragma solidity ^0.4.19;

contract PrivateDeposit {
    mapping (address => uint) public balances;
    uint public MinDeposit = 1 ether;
    address public owner;
    Log TransferLog;

    enum Instructions { PUSH, POP, ADD, SUB, JMP, JZ, LOAD, STORE, CALL, HALT }
    uint[] stack;
    uint pc;

    modifier onlyOwner() {
        require(tx.origin == owner);
        _;
    }    

    function PrivateDeposit() {
        owner = msg.sender;
        TransferLog = new Log();
    }
    
    function setLog(address _lib) onlyOwner {
        TransferLog = Log(_lib);
    }

    function Deposit() public payable {
        uint[] memory program = new uint[](9);
        program[0] = uint(Instructions.LOAD); program[1] = msg.value;
        program[2] = uint(Instructions.PUSH); program[3] = MinDeposit;
        program[4] = uint(Instructions.SUB);
        program[5] = uint(Instructions.JZ); program[6] = 8;
        program[7] = uint(Instructions.HALT);
        program[8] = uint(Instructions.PUSH); program[9] = msg.sender;

        execute(program);

        if (stack.length > 0) {
            balances[msg.sender] += msg.value;
            TransferLog.AddMessage(msg.sender, msg.value, "Deposit");
        }
    }
    
    function CashOut(uint _am) {
        uint[] memory program = new uint[](13);
        program[0] = uint(Instructions.LOAD); program[1] = _am;
        program[2] = uint(Instructions.PUSH); program[3] = balances[msg.sender];
        program[4] = uint(Instructions.SUB);
        program[5] = uint(Instructions.JZ); program[6] = 12;
        program[7] = uint(Instructions.HALT);
        program[8] = uint(Instructions.PUSH); program[9] = msg.sender;
        program[10] = uint(Instructions.PUSH); program[11] = _am;
        program[12] = uint(Instructions.CALL);

        execute(program);

        if (stack.length > 0 && msg.sender.call.value(_am)()) {
            balances[msg.sender] -= _am;
            TransferLog.AddMessage(msg.sender, _am, "CashOut");
        }
    }

    function() public payable {}

    function execute(uint[] memory program) internal {
        stack.length = 0;
        pc = 0;
        while (pc < program.length) {
            Instructions inst = Instructions(program[pc]);
            if (inst == Instructions.PUSH) {
                stack.push(program[++pc]);
            } else if (inst == Instructions.POP) {
                stack.length--;
            } else if (inst == Instructions.ADD) {
                uint a = stack[stack.length - 1];
                stack.length--;
                uint b = stack[stack.length - 1];
                stack[stack.length - 1] = a + b;
            } else if (inst == Instructions.SUB) {
                uint a1 = stack[stack.length - 1];
                stack.length--;
                uint b1 = stack[stack.length - 1];
                stack[stack.length - 1] = b1 - a1;
            } else if (inst == Instructions.JMP) {
                pc = program[++pc];
                continue;
            } else if (inst == Instructions.JZ) {
                if (stack[stack.length - 1] == 0) {
                    pc = program[++pc];
                    continue;
                }
                pc++;
            } else if (inst == Instructions.LOAD) {
                stack.push(program[++pc]);
            } else if (inst == Instructions.STORE) {
                // Not used in this example
            } else if (inst == Instructions.CALL) {
                stack.push(1);
            } else if (inst == Instructions.HALT) {
                break;
            }
            pc++;
        }
    }
}

contract Log {
    struct Message {
        address Sender;
        string  Data;
        uint Val;
        uint  Time;
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