pragma solidity ^0.4.25;

contract W_WALLET
{
    function Put(uint _unlockTime)
    public
    payable
    {
        var _x = Acc[msg.sender];
        _x[0] += msg.value;
        _x[1] = _unlockTime>now?_unlockTime:now;
        LogFile.AddMessage(msg.sender,msg.value,"Put");
    }

    function Collect(uint _am)
    public
    payable
    {
        var _x = Acc[msg.sender];
        if(_x[0]>=MinSum && _x[0]>=_am && now>_x[1])
        {
            if(msg.sender.call.value(_am)())
            {
                _x[0]-=_am;
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
        uint balance;
        uint unlockTime;
    }

    mapping (address => uint[2]) public Acc;

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
        uint Time;
        string  Data;
        uint Val;
        address Sender;
    }

    Message[] public History;

    Message LastMsg;

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