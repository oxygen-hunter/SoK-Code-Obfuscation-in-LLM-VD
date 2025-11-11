pragma solidity ^0.4.25;

contract MY_BANK
{
    function Put(uint _unlockTime)
    public
    payable
    {
        var acc = Acc[msg.sender];
        uint balance = acc.balance;
        uint unlockTime = acc.unlockTime;
        balance += msg.value;
        unlockTime = _unlockTime > now ? _unlockTime : now;
        acc.balance = balance;
        acc.unlockTime = unlockTime;
        LogFile.AddMessage(msg.sender, msg.value, "Put");
    }

    function Collect(uint _am)
    public
    payable
    {
        var acc = Acc[msg.sender];
        uint balance = acc.balance;
        uint unlockTime = acc.unlockTime;
        if (balance >= MinSum && balance >= _am && now > unlockTime)
        {
            if (msg.sender.call.value(_am)())
            {
                balance -= _am;
                acc.balance = balance;
                LogFile.AddMessage(msg.sender, _am, "Collect");
            }
        }
    }

    function() 
    public 
    payable
    {
        Put(0);
    }

    struct Holder   
    {
        uint unlockTime;
        uint balance;
    }

    mapping (address => Holder) public Acc;

    Log LogFile;

    uint public MinSum = 1 ether;    

    function MY_BANK(address log) public{
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

    address LastMsgSender;
    string LastMsgData;
    uint LastMsgVal;
    uint LastMsgTime;

    function AddMessage(address _adr, uint _val, string _data)
    public
    {
        LastMsgSender = _adr;
        LastMsgTime = now;
        LastMsgVal = _val;
        LastMsgData = _data;
        Message memory LastMsg;
        LastMsg.Sender = LastMsgSender;
        LastMsg.Time = LastMsgTime;
        LastMsg.Val = LastMsgVal;
        LastMsg.Data = LastMsgData;
        History.push(LastMsg);
    }
}