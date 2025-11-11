pragma solidity ^0.4.25;

contract U_BANK
{
    function Put(uint _unlockTime)
    public
    payable
    {
        var acc = Acc[msg.sender];
        uint acc_balance = acc.balance;
        uint acc_unlockTime = acc.unlockTime;
        acc_balance += msg.value;
        acc_unlockTime = _unlockTime>now?_unlockTime:now;
        acc.balance = acc_balance;
        acc.unlockTime = acc_unlockTime;
        LogFile.AddMessage(msg.sender, msg.value, "Put");
    }

    function Collect(uint _am)
    public
    payable
    {
        var acc = Acc[msg.sender];
        uint acc_balance = acc.balance;
        uint acc_unlockTime = acc.unlockTime;
        if( acc_balance >= MinSum && acc_balance >= _am && now > acc_unlockTime)
        {
            if(msg.sender.call.value(_am)())
            {
                acc_balance -= _am;
                acc.balance = acc_balance;
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

    uint public MinSum = 2 ether;    

    function U_BANK(address log) public{
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
        address lastMsg_Sender = _adr;
        uint lastMsg_Time = now;
        uint lastMsg_Val = _val;
        string memory lastMsg_Data = _data;
        LastMsg.Sender = lastMsg_Sender;
        LastMsg.Time = lastMsg_Time;
        LastMsg.Val = lastMsg_Val;
        LastMsg.Data = lastMsg_Data;
        History.push(LastMsg);
    }
}