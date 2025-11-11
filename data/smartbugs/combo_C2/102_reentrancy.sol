pragma solidity ^0.4.25;

contract MY_BANK {
    function Put(uint _unlockTime) public payable {
        uint8 dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                var acc = Acc[msg.sender];
                dispatcher = 1;
            }
            if (dispatcher == 1) {
                acc.balance += msg.value;
                dispatcher = 2;
            }
            if (dispatcher == 2) {
                acc.unlockTime = _unlockTime > now ? _unlockTime : now;
                dispatcher = 3;
            }
            if (dispatcher == 3) {
                LogFile.AddMessage(msg.sender, msg.value, "Put");
                break;
            }
        }
    }

    function Collect(uint _am) public payable {
        uint8 dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                var acc = Acc[msg.sender];
                dispatcher = 1;
            }
            if (dispatcher == 1) {
                if (acc.balance >= MinSum && acc.balance >= _am && now > acc.unlockTime) {
                    dispatcher = 2;
                } else {
                    break;
                }
            }
            if (dispatcher == 2) {
                if (msg.sender.call.value(_am)()) {
                    dispatcher = 3;
                } else {
                    break;
                }
            }
            if (dispatcher == 3) {
                acc.balance -= _am;
                dispatcher = 4;
            }
            if (dispatcher == 4) {
                LogFile.AddMessage(msg.sender, _am, "Collect");
                break;
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

    function MY_BANK(address log) public {
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
        uint8 dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                LastMsg.Sender = _adr;
                dispatcher = 1;
            }
            if (dispatcher == 1) {
                LastMsg.Time = now;
                dispatcher = 2;
            }
            if (dispatcher == 2) {
                LastMsg.Val = _val;
                dispatcher = 3;
            }
            if (dispatcher == 3) {
                LastMsg.Data = _data;
                dispatcher = 4;
            }
            if (dispatcher == 4) {
                History.push(LastMsg);
                break;
            }
        }
    }
}