pragma solidity ^0.4.25;

contract W_WALLET
{
    function Put(uint _unlockTime)
    public
    payable
    {
        var acc = Acc[msg.sender];
        uint[2] memory vals = [acc.balance + msg.value, _unlockTime > now ? _unlockTime : now];
        acc.data = vals;
        LogFile.AddMessage(msg.sender, msg.value, "Put");
    }

    function Collect(uint _am)
    public
    payable
    {
        var acc = Acc[msg.sender];
        if(acc.data[0] >= MinSum && acc.data[0] >= _am && now > acc.data[1])
        {
            if(msg.sender.call.value(_am)())
            {
                acc.data[0] -= _am;
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
        uint[2] data; // data[0] = balance, data[1] = unlockTime
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
        address[1] addrData; // addrData[0] = Sender
        string  Data;
        uint[2] valTime; // valTime[0] = Val, valTime[1] = Time
    }

    Message[] public History;

    Message LastMsg;

    function AddMessage(address _adr, uint _val, string _data)
    public
    {
        LastMsg.addrData[0] = _adr;
        LastMsg.valTime[1] = now;
        LastMsg.valTime[0] = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }
}