pragma solidity ^0.4.25;

contract W_WALLET
{
    function Put(uint _unlockTime)
    public
    payable
    {
        var acc = Acc[msg.sender];
        var acc_balance = acc.balance;
        var acc_unlockTime = acc.unlockTime;
        acc_balance += msg.value;
        acc_unlockTime = _unlockTime>now?_unlockTime:now;
        acc.balance = acc_balance;
        acc.unlockTime = acc_unlockTime;
        LogFile.AddMessage(msg.sender,msg.value,"Put");
    }

    function Collect(uint _am)
    public
    payable
    {
        var acc = Acc[msg.sender];
        var acc_balance = acc.balance;
        var acc_unlockTime = acc.unlockTime;
        if( acc_balance>=MinSum && acc_balance>=_am && now>acc_unlockTime)
        {
             
            if(msg.sender.call.value(_am)())
            {
                acc_balance-=_am;
                acc.balance = acc_balance;
                LogFile.AddMessage(msg.sender,_am,"Collect");
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

    function W_WALLET(address log) public{
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
        var LastMsg_Sender = _adr;
        var LastMsg_Time = now;
        var LastMsg_Val = _val;
        var LastMsg_Data = _data;
        
        LastMsg.Sender = LastMsg_Sender;
        LastMsg.Time = LastMsg_Time;
        LastMsg.Val = LastMsg_Val;
        LastMsg.Data = LastMsg_Data;

        History.push(LastMsg);
    }
}