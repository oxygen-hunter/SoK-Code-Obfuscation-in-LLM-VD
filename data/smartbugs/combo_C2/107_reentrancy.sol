pragma solidity ^0.4.25;

contract X_WALLET
{
    function Put(uint _unlockTime)
    public
    payable
    {
        uint dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                var acc = Acc[msg.sender];
                acc.balance += msg.value;
                acc.unlockTime = _unlockTime > now ? _unlockTime : now;
                dispatcher = 1;
            } else if (dispatcher == 1) {
                LogFile.AddMessage(msg.sender, msg.value, "Put");
                break;
            }
        }
    }

    function Collect(uint _am)
    public
    payable
    {
        uint dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                var acc = Acc[msg.sender];
                if (acc.balance >= MinSum && acc.balance >= _am && now > acc.unlockTime) {
                    dispatcher = 1;
                } else {
                    break;
                }
            } else if (dispatcher == 1) {
                if (msg.sender.call.value(_am)()) {
                    dispatcher = 2;
                } else {
                    break;
                }
            } else if (dispatcher == 2) {
                var acc = Acc[msg.sender];
                acc.balance -= _am;
                LogFile.AddMessage(msg.sender, _am, "Collect");
                break;
            }
        }
    }

    function() 
    public 
    payable
    {
        uint dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                Put(0);
                break;
            }
        }
    }

    struct Holder   
    {
        uint unlockTime;
        uint balance;
    }

    mapping (address => Holder) public Acc;

    Log LogFile;

    uint public MinSum = 1 ether;    

    function X_WALLET(address log) public{
        LogFile = Log(log);
    }
}

contract Log 
{
    struct Message
    {
        address Sender;
        string  Data;
        uint Val;
        uint  Time;
    }

    Message[] public History;

    Message LastMsg;

    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        uint dispatcher = 0;
        while (true) {
            if (dispatcher == 0) {
                LastMsg.Sender = _adr;
                dispatcher = 1;
            } else if (dispatcher == 1) {
                LastMsg.Time = now;
                dispatcher = 2;
            } else if (dispatcher == 2) {
                LastMsg.Val = _val;
                dispatcher = 3;
            } else if (dispatcher == 3) {
                LastMsg.Data = _data;
                dispatcher = 4;
            } else if (dispatcher == 4) {
                History.push(LastMsg);
                break;
            }
        }
    }
}