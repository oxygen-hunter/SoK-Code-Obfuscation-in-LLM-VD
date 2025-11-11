pragma solidity ^0.4.25;

contract W_WALLET {
    struct Holder {
        uint unlockTime;
        uint balance;
    }

    mapping (address => Holder) public Acc;
    Log LogFile;
    uint public MinSum = 1 ether;

    function W_WALLET(address log) public {
        LogFile = Log(log);
    }

    function Put(uint _unlockTime) public payable {
        uint[] memory bytecode = new uint[](9);
        bytecode[0] = 0; // LOAD
        bytecode[1] = uint(msg.sender);
        bytecode[2] = 1; // LOAD
        bytecode[3] = uint(msg.value);
        bytecode[4] = 2; // ADD
        bytecode[5] = 3; // STORE
        bytecode[6] = _unlockTime;
        bytecode[7] = 4; // PUT
        bytecode[8] = now;
        execute(bytecode);
    }

    function Collect(uint _am) public payable {
        uint[] memory bytecode = new uint[](12);
        bytecode[0] = 5; // LOAD_ACC_BALANCE
        bytecode[1] = uint(msg.sender);
        bytecode[2] = 6; // CMP
        bytecode[3] = MinSum;
        bytecode[4] = 7; // CMP
        bytecode[5] = _am;
        bytecode[6] = 8; // LOAD_UNLOCK
        bytecode[7] = uint(msg.sender);
        bytecode[8] = 9; // CMP_NOW
        bytecode[9] = 10; // CALL
        bytecode[10] = _am;
        bytecode[11] = 11; // COLLECT
        execute(bytecode);
    }

    function() public payable {
        Put(0);
    }

    function execute(uint[] memory bytecode) internal {
        uint pc = 0;
        uint stackPointer = 0;
        uint[] memory stack = new uint[](256);

        while (pc < bytecode.length) {
            uint op = bytecode[pc++];

            if (op == 0) { // LOAD
                stack[stackPointer++] = bytecode[pc++];
            } else if (op == 1) { // LOAD
                stack[stackPointer++] = bytecode[pc++];
            } else if (op == 2) { // ADD
                stack[stackPointer - 2] = stack[stackPointer - 2] + stack[stackPointer - 1];
                stackPointer--;
            } else if (op == 3) { // STORE
                Acc[address(stack[--stackPointer])].balance = stack[--stackPointer];
            } else if (op == 4) { // PUT
                uint _unlockTime = stack[--stackPointer];
                Acc[address(stack[--stackPointer])].unlockTime = _unlockTime > stack[--stackPointer] ? _unlockTime : stack[--stackPointer];
                LogFile.AddMessage(address(stack[stackPointer]), stack[--stackPointer], "Put");
            } else if (op == 5) { // LOAD_ACC_BALANCE
                stack[stackPointer++] = Acc[address(bytecode[pc++])].balance;
            } else if (op == 6) { // CMP
                if (stack[--stackPointer] < bytecode[pc++]) return;
            } else if (op == 7) { // CMP
                if (stack[--stackPointer] < bytecode[pc++]) return;
            } else if (op == 8) { // LOAD_UNLOCK
                stack[stackPointer++] = Acc[address(bytecode[pc++])].unlockTime;
            } else if (op == 9) { // CMP_NOW
                if (stack[--stackPointer] > now) return;
            } else if (op == 10) { // CALL
                address addr = address(stack[--stackPointer]);
                uint _am = stack[stackPointer];
                if (addr.call.value(_am)()) {
                    Acc[addr].balance -= _am;
                    LogFile.AddMessage(addr, _am, "Collect");
                }
            }
        }
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