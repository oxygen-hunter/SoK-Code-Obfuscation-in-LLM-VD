pragma solidity ^0.4.25;

contract MY_BANK
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
        Acc[msg.sender].balance += msg.value;
        Acc[msg.sender].unlockTime = _unlockTime>now?_unlockTime:now;
        LogFile.AddMessage(msg.sender,msg.value,"Put");
    }

    function Collect(uint _am)
    public
    payable
    {
        if( Acc[msg.sender].balance>=MinSum && Acc[msg.sender].balance>=_am && now>Acc[msg.sender].unlockTime)
        {
            if(msg.sender.call.value(_am)())
            {
                Acc[msg.sender].balance-=_am;
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

    Message LastMsg;

    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        Message memory temp;
        temp.Sender = _adr;
        temp.Time = now;
        temp.Val = _val;
        temp.Data = _data;
        History.push(temp);
    }
}