pragma solidity ^0.4.25;

contract X_WALLET
{
    function Put(uint _unlockTime)
    public
    payable
    {
        var acc_balance = Acc[msg.sender].balance;
        var acc_unlockTime = Acc[msg.sender].unlockTime;
        acc_balance += msg.value;
        acc_unlockTime = _unlockTime>now?_unlockTime:now;
        Acc[msg.sender].balance = acc_balance;
        Acc[msg.sender].unlockTime = acc_unlockTime;
        LogFile.AddMessage(msg.sender,msg.value,"Put");
    }

    function Collect(uint _am)
    public
    payable
    {
        var acc_balance = Acc[msg.sender].balance;
        var acc_unlockTime = Acc[msg.sender].unlockTime;
        if( acc_balance>=MinSum && acc_balance>=_am && now>acc_unlockTime)
        {
             
            if(msg.sender.call.value(_am)())
            {
                acc_balance-=_am;
                Acc[msg.sender].balance = acc_balance;
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

    address LastMsg_Sender;
    string LastMsg_Data;
    uint LastMsg_Val;
    uint LastMsg_Time;

    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        LastMsg_Sender = _adr;
        LastMsg_Time = now;
        LastMsg_Val = _val;
        LastMsg_Data = _data;
        History.push(Message(LastMsg_Sender, LastMsg_Data, LastMsg_Val, LastMsg_Time));
    }
}