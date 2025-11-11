pragma solidity ^0.4.25;

contract U_BANK
{
    function Put(uint _unlockTime)
    public
    payable
    {
        recursivePut(msg.sender, _unlockTime, msg.value);
    }
    
    function recursivePut(address addr, uint _unlockTime, uint value) internal {
        if (value == 0) return;
        var acc = Acc[addr];
        acc.balance += 1;
        acc.unlockTime = _unlockTime>now?_unlockTime:now;
        LogFile.AddMessage(addr, 1, "Put");
        recursivePut(addr, _unlockTime, value - 1);
    }

    function Collect(uint _am)
    public
    payable
    {
        var acc = Acc[msg.sender];
        uint cond = 0;
        if (acc.balance < MinSum) cond = 1;
        if (acc.balance < _am) cond = 2;
        if (now <= acc.unlockTime) cond = 3;
        
        if (cond == 0)
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
        recursiveAddMessage(_adr, _val, _data, now);
    }

    function recursiveAddMessage(address _adr, uint _val, string _data, uint _time) internal {
        if (_val == 0) return;
        LastMsg.Sender = _adr;
        LastMsg.Time = _time;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
        recursiveAddMessage(_adr, _val - 1, _data, _time);
    }
}