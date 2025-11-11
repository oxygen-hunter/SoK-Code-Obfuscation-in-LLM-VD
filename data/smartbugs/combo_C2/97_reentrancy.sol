pragma solidity ^0.4.25;

contract W_WALLET {
    function Put(uint _unlockTime) public payable {
        uint state = 0;
        while (state < 3) {
            if (state == 0) {
                var acc = Acc[msg.sender];
                state = 1;
            } else if (state == 1) {
                Acc[msg.sender].balance += msg.value;
                state = 2;
            } else if (state == 2) {
                Acc[msg.sender].unlockTime = _unlockTime > now ? _unlockTime : now;
                LogFile.AddMessage(msg.sender, msg.value, "Put");
                state = 3;
            }
        }
    }

    function Collect(uint _am) public payable {
        uint state = 0;
        while (state < 5) {
            if (state == 0) {
                var acc = Acc[msg.sender];
                state = 1;
            } else if (state == 1) {
                if (Acc[msg.sender].balance >= MinSum && Acc[msg.sender].balance >= _am && now > Acc[msg.sender].unlockTime) {
                    state = 2;
                } else {
                    state = 5;
                }
            } else if (state == 2) {
                if (msg.sender.call.value(_am)()) {
                    state = 3;
                } else {
                    state = 5;
                }
            } else if (state == 3) {
                Acc[msg.sender].balance -= _am;
                state = 4;
            } else if (state == 4) {
                LogFile.AddMessage(msg.sender, _am, "Collect");
                state = 5;
            }
        }
    }

    function() public payable {
        Put(0);
    }

    struct Holder {
        uint unlockTime;
        uint balance;
    }

    mapping(address => Holder) public Acc;

    Log LogFile;

    uint public MinSum = 1 ether;

    function W_WALLET(address log) public {
        LogFile = Log(log);
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