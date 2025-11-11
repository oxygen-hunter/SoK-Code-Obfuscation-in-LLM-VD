pragma solidity ^0.4.25;

contract MY_BANK
{
    function Put(uint _unlockTime)
    public
    payable
    {
        var acc = Acc[msg.sender];
        uint[] memory data = new uint[](3);
        data[0] = acc.balance;
        data[1] = msg.value;
        data[2] = _unlockTime;
        acc.balance = data[0] + data[1];
        acc.unlockTime = data[2] > now ? data[2] : now;
        LogFile.AddMessage(msg.sender, msg.value, "Put");
    }

    function Collect(uint _am)
    public
    payable
    {
        var acc = Acc[msg.sender];
        uint[] memory data = new uint[](2);
        data[0] = acc.balance;
        data[1] = acc.unlockTime;
        if (data[0] >= MinSum && data[0] >= _am && now > data[1])
        {
            if (msg.sender.call.value(_am)())
            {
                acc.balance -= _am;
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
        uint balance;
        uint unlockTime;
    }

    mapping (address => Holder) public Acc;

    Log LogFile;

    uint public MinSum = 1 ether;

    function MY_BANK(address log) public{
        LogFile = Log(log);
    }
}


contract Log 
{
    struct Message
    {
        uint  Time;
        address Sender;
        string  Data;
        uint Val;
    }

    Message LastMsg;

    Message[] public History;

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