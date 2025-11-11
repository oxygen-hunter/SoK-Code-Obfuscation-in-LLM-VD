pragma solidity ^0.4.25;

contract U_BANK {
    struct Holder {
        uint unlockTime;
        uint balance;
    }

    mapping(address => Holder) public Acc;
    Log LogFile;
    uint public MinSum = 2 ether;

    function U_BANK(address log) public {
        LogFile = Log(log);
    }

    function Put(uint _unlockTime) public payable {
        uint[] memory code = new uint[](11);
        code[0] = 1; // LOAD
        code[1] = uint(msg.sender);
        code[2] = 2; // LOAD_ACC
        code[3] = 3; // LOAD_MSG_VALUE
        code[4] = 4; // ADD
        code[5] = 5; // STORE_ACC_BALANCE
        code[6] = 6; // LOAD_NOW
        code[7] = 7; // COMPARE_GT
        code[8] = 8; // STORE_ACC_UNLOCKTIME
        code[9] = 9; // LOG
        code[10] = 10; // HALT
        execute(code, _unlockTime, msg.value, "Put");
    }

    function Collect(uint _am) public payable {
        uint[] memory code = new uint[](17);
        code[0] = 1; // LOAD
        code[1] = uint(msg.sender);
        code[2] = 2; // LOAD_ACC
        code[3] = 11; // LOAD_ACC_BALANCE
        code[4] = 12; // LOAD_MINSUM
        code[5] = 13; // COMPARE_GE
        code[6] = 11; // LOAD_ACC_BALANCE
        code[7] = 14; // COMPARE_GE
        code[8] = 6; // LOAD_NOW
        code[9] = 15; // COMPARE_GT
        code[10] = 16; // AND3
        code[11] = 17; // JZ
        code[12] = 18; // CALL
        code[13] = 11; // LOAD_ACC_BALANCE
        code[14] = 19; // SUB
        code[15] = 5; // STORE_ACC_BALANCE
        code[16] = 9; // LOG
        code[17] = 10; // HALT
        execute(code, _am, 0, "Collect");
    }

    function() public payable {
        Put(0);
    }

    function execute(uint[] memory code, uint arg1, uint arg2, string memory logMsg) internal {
        uint pc = 0;
        uint stackPtr = 0;
        uint[10] memory stack;
        Holder memory acc;
        acc = Acc[address(arg1)];
        while (true) {
            uint op = code[pc];
            pc++;
            if (op == 1) { // LOAD
                stack[stackPtr++] = arg1;
            } else if (op == 2) { // LOAD_ACC
                stack[stackPtr++] = uint(acc);
            } else if (op == 3) { // LOAD_MSG_VALUE
                stack[stackPtr++] = arg2;
            } else if (op == 4) { // ADD
                stackPtr--;
                stack[stackPtr - 1] += stack[stackPtr];
            } else if (op == 5) { // STORE_ACC_BALANCE
                acc.balance = stack[--stackPtr];
            } else if (op == 6) { // LOAD_NOW
                stack[stackPtr++] = now;
            } else if (op == 7) { // COMPARE_GT
                stackPtr--;
                stack[stackPtr - 1] = (stack[stackPtr - 1] > stack[stackPtr]) ? stack[stackPtr - 1] : stack[stackPtr];
            } else if (op == 8) { // STORE_ACC_UNLOCKTIME
                acc.unlockTime = stack[--stackPtr];
            } else if (op == 9) { // LOG
                LogFile.AddMessage(address(stack[--stackPtr]), stack[--stackPtr], logMsg);
            } else if (op == 10) { // HALT
                break;
            } else if (op == 11) { // LOAD_ACC_BALANCE
                stack[stackPtr++] = acc.balance;
            } else if (op == 12) { // LOAD_MINSUM
                stack[stackPtr++] = MinSum;
            } else if (op == 13 || op == 14) { // COMPARE_GE
                stackPtr--;
                stack[stackPtr - 1] = (stack[stackPtr - 1] >= stack[stackPtr]) ? 1 : 0;
            } else if (op == 15) { // COMPARE_GT
                stackPtr--;
                stack[stackPtr - 1] = (stack[stackPtr - 1] > stack[stackPtr]) ? 1 : 0;
            } else if (op == 16) { // AND3
                stackPtr--;
                stackPtr--;
                stack[stackPtr - 1] = (stack[stackPtr - 1] & stack[stackPtr] & stack[stackPtr + 1]);
            } else if (op == 17) { // JZ
                if (stack[--stackPtr] == 0) pc = code[pc];
                else pc++;
            } else if (op == 18) { // CALL
                if (msg.sender.call.value(stack[--stackPtr])()) {
                    continue;
                }
            } else if (op == 19) { // SUB
                stackPtr--;
                stack[stackPtr - 1] -= stack[stackPtr];
            }
        }
        Acc[address(arg1)] = acc;
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