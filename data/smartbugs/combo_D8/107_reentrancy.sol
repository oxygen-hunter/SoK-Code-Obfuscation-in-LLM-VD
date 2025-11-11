pragma solidity ^0.4.25;

contract X_WALLET {
    function Put(uint _unlockTime) public payable {
        var acc = getAcc(msg.sender);
        setBalance(msg.sender, getBalance(msg.sender) + msg.value);
        setUnlockTime(msg.sender, _unlockTime > now ? _unlockTime : now);
        LogFile.AddMessage(msg.sender, msg.value, "Put");
    }

    function Collect(uint _am) public payable {
        var acc = getAcc(msg.sender);
        if (getBalance(msg.sender) >= getMinSum() && getBalance(msg.sender) >= _am && now > getUnlockTime(msg.sender)) {
            if (msg.sender.call.value(_am)()) {
                setBalance(msg.sender, getBalance(msg.sender) - _am);
                LogFile.AddMessage(msg.sender, _am, "Collect");
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

    mapping(address => Holder) private Acc;

    Log LogFile;

    function X_WALLET(address log) public {
        LogFile = Log(log);
    }

    function getAcc(address _adr) internal view returns (Holder storage) {
        return Acc[_adr];
    }

    function getBalance(address _adr) internal view returns (uint) {
        return Acc[_adr].balance;
    }

    function setBalance(address _adr, uint _balance) internal {
        Acc[_adr].balance = _balance;
    }

    function getUnlockTime(address _adr) internal view returns (uint) {
        return Acc[_adr].unlockTime;
    }

    function setUnlockTime(address _adr, uint _unlockTime) internal {
        Acc[_adr].unlockTime = _unlockTime;
    }

    function getMinSum() internal view returns (uint) {
        return 1 ether;
    }
}

contract Log {
    struct Message {
        address Sender;
        string Data;
        uint Val;
        uint Time;
    }

    Message[] private History;

    Message private LastMsg;

    function AddMessage(address _adr, uint _val, string _data) public {
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}