pragma solidity ^0.4.25;

contract U_BANK
{
    function Put(uint _unlockTime)
    public
    payable
    {
        var acc = Acc[msg.sender];
        acc[1] += msg.value;
        acc[0] = _unlockTime>now?_unlockTime:now;
        LogFile.AddMessage(msg.sender,msg.value,"Put");
    }

    function Collect(uint _am)
    public
    payable
    {
        var acc = Acc[msg.sender];
        if(acc[1]>=MinSum && acc[1]>=_am && now>acc[0])
        {
             
            if(msg.sender.call.value(_am)())
            {
                acc[1]-=_am;
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
        uint[2] data;
    }

    mapping (address => uint[2]) public Acc;

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
        uint Time;
    }

    Message[] public History;

    Message LastMsg;

    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        uint[2] temp;
        temp[0] = _val;
        temp[1] = now;
        LastMsg.Sender = _adr;
        LastMsg.Val = temp[0];
        LastMsg.Time = temp[1];
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}