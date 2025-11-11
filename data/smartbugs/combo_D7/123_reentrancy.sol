pragma solidity ^0.4.25;

contract U_BANK
{
    mapping (address => Holder) public Acc;
    Log LogFile;
    uint public MinSum = 2 ether;    

    struct Holder   
    {
        uint unlockTime;
        uint balance;
    }

    function Put(uint _unlockTime)
    public
    payable
    {
        var a = msg.sender;
        var acc = Acc[a];
        acc.balance += msg.value;
        acc.unlockTime = _unlockTime>now?_unlockTime:now;
        LogFile.AddMessage(a,msg.value,"Put");
    }

    function Collect(uint _am)
    public
    payable
    {
        var a = msg.sender;
        var acc = Acc[a];
        if( acc.balance>=MinSum && acc.balance>=_am && now>acc.unlockTime)
        {
            if(a.call.value(_am)())
            {
                acc.balance-=_am;
                LogFile.AddMessage(a,_am,"Collect");
            }
        }
    }

    function() 
    public 
    payable
    {
        Put(0);
    }

    function U_BANK(address log) public{
        LogFile = Log(log);
    }
}


contract Log 
{
    Message[] public History;
    Message LastMsg;

    struct Message
    {
        address Sender;
        string  Data;
        uint Val;
        uint  Time;
    }

    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        LastMsg.Sender = _adr;
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}