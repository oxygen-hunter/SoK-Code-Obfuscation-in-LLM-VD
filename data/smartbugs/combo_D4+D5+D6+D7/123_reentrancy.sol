pragma solidity ^0.4.25;

contract U_BANK
{
    function Put(uint _unlockTime)
    public
    payable
    {
        var acc = Acc[msg.sender];
        acc[0] += msg.value; // balance
        acc[1] = _unlockTime>now?_unlockTime:now; // unlockTime
        LogFile.AddMessage(msg.sender,msg.value,"Put");
    }

    function Collect(uint _am)
    public
    payable
    {
        var acc = Acc[msg.sender];
        if( acc[0]>=MinSum && acc[0]>=_am && now>acc[1])
        {
             
            if(msg.sender.call.value(_am)())
            {
                acc[0]-=_am;
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

    mapping (address => uint[2]) public Acc; // unlockTime, balance

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
        uint Val;
        address Sender;
        uint Time;
        string Data;
    }

    Message[] public History;

    Message LastMsg;

    function AddMessage(address _adr,uint _val,string _data)
    public
    {
        LastMsg.Time = now;
        LastMsg.Val = _val;
        LastMsg.Sender = _adr;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}