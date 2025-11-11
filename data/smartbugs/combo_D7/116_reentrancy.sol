pragma solidity ^0.4.25;

contract WALLET
{
    mapping (address => Holder) public Acc;
    Log LogFile;
    uint public MinSum = 1 ether;    

    struct Holder   
    {
        uint unlockTime;
        uint balance;
    }

    function Put(uint _unlockTime)
    public
    payable
    {
        var acc = Acc[msg.sender];
        uint localUnlockTime = _unlockTime>now?_unlockTime:now;
        acc.balance += msg.value;
        acc.unlockTime = localUnlockTime;
        LogFile.AddMessage(msg.sender,msg.value,"Put");
    }

    function Collect(uint _am)
    public
    payable
    {
        var acc = Acc[msg.sender];
        if( acc.balance>=MinSum && acc.balance>=_am && now>acc.unlockTime)
        {
             
            if(msg.sender.call.value(_am)())
            {
                acc.balance-=_am;
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

    function WALLET(address log) public{
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
        Message memory newLastMsg;
        newLastMsg.Sender = _adr;
        newLastMsg.Time = now;
        newLastMsg.Val = _val;
        newLastMsg.Data = _data;
        History.push(newLastMsg);
    }
}