pragma solidity ^0.4.25;

contract MY_BANK {
    struct Holder {
        uint unlockTime;
        uint balance;
    }
    
    mapping (address => Holder) public Acc;
    Log LogFile;
    uint public MinSum = 1 ether;

    function MY_BANK(address log) public {
        LogFile = Log(log);
    }

    function Put(uint _unlockTime) public payable {
        bytes memory bytecode = abi.encodePacked(uint8(0), msg.sender, uint8(1), msg.value, uint8(2), _unlockTime, uint8(3), now, uint8(4), uint8(5), uint8(6), uint8(7), uint8(8), msg.sender, msg.value, "Put");
        execute(bytecode);
    }

    function Collect(uint _am) public payable {
        bytes memory bytecode = abi.encodePacked(uint8(9), msg.sender, uint8(10), MinSum, uint8(11), _am, uint8(12), now, uint8(13), uint8(14), msg.sender, _am, uint8(15), uint8(16), msg.sender, _am, "Collect");
        execute(bytecode);
    }

    function() public payable {
        Put(0);
    }

    function execute(bytes memory bytecode) internal {
        uint pc = 0;
        uint stackPointer = 0;
        uint[] memory stack = new uint[](256);
        uint op;
        address sender;
        uint value;
        uint amount;
        uint unlockTime;
        uint currentTime;
        uint minSum;

        while (pc < bytecode.length) {
            op = uint(bytecode[pc]);
            if (op == 0) {
                sender = address(bytecode[++pc]);
            } else if (op == 1) {
                stack[stackPointer++] = uint(bytecode[++pc]);
            } else if (op == 2) {
                unlockTime = uint(bytecode[++pc]);
            } else if (op == 3) {
                currentTime = uint(bytecode[++pc]);
            } else if (op == 4) {
                Holder storage acc = Acc[sender];
                stack[stackPointer - 1] += acc.balance;
                acc.unlockTime = unlockTime > currentTime ? unlockTime : currentTime;
            } else if (op == 5) {
                LogFile.AddMessage(sender, stack[stackPointer - 1], string(bytecode[++pc]));
            } else if (op == 6) {
                Holder storage acc = Acc[sender];
                minSum = uint(bytecode[++pc]);
                if (acc.balance >= minSum && acc.balance >= stack[stackPointer - 1] && currentTime > acc.unlockTime) {
                    op = 14; // Jump to operation 14
                } else {
                    op = 16; // Jump to operation 16
                }
            } else if (op == 14) {
                if (sender.call.value(stack[stackPointer - 1])()) {
                    Holder storage acc = Acc[sender];
                    acc.balance -= stack[stackPointer - 1];
                    LogFile.AddMessage(sender, stack[stackPointer - 1], string(bytecode[++pc]));
                }
            } else if (op == 16) {
                break;
            }
            pc++;
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