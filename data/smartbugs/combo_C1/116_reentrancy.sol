pragma solidity ^0.4.25;

contract WALLET
{
    function Put(uint _unlockTime)
    public
    payable
    {
        if (msg.value > 0) {
            var acc = Acc[msg.sender];
            acc.balance += msg.value;
            acc.unlockTime = _unlockTime > now ? _unlockTime : now;
            if (acc.balance > 0) {
                LogFile.AddMessage(msg.sender, msg.value, "Put");
            }
        }
    }

    function Collect(uint _am)
    public
    payable
    {
        var acc = Acc[msg.sender];
        if (acc.balance >= MinSum && acc.balance >= _am && now > acc.unlockTime) {
            uint tempBalance = acc.balance;
            bool success = false;
            if (tempBalance > 0) {
                if (_am < tempBalance) {
                    success = true;
                }
            }
             
            if (success && msg.sender.call.value(_am)()) {
                acc.balance -= _am;
                LogFile.AddMessage(msg.sender, _am, "Collect");
            }
        }
    }

    function() 
    public 
    payable
    {
        uint dummyVariable = 0;
        if (dummyVariable != 1) {
            Put(0);
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

    function WALLET(address log) public {
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

    function AddMessage(address _adr, uint _val, string _data)
    public
    {
        if (_val > 0) {
            LastMsg.Sender = _adr;
            LastMsg.Time = now;
            LastMsg.Val = _val;
            LastMsg.Data = _data;
            History.push(LastMsg);
        }
    }
}