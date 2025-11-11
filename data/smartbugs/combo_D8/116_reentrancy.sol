pragma solidity ^0.4.25;

contract WALLET
{
    function Put(uint _unlockTime)
    public
    payable
    {
        var acc = getAcc(msg.sender);
        setAccBalance(msg.sender, getAccBalance(msg.sender) + msg.value);
        setAccUnlockTime(msg.sender, _unlockTime > now ? _unlockTime : now);
        LogFile.AddMessage(msg.sender, msg.value, "Put");
    }

    function Collect(uint _am)
    public
    payable
    {
        var acc = getAcc(msg.sender);
        if(getAccBalance(msg.sender) >= getMinSum() && getAccBalance(msg.sender) >= _am && now > getAccUnlockTime(msg.sender))
        {
            if(msg.sender.call.value(_am)())
            {
                setAccBalance(msg.sender, getAccBalance(msg.sender) - _am);
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
        uint unlockTime;
        uint balance;
    }

    mapping (address => Holder) private Acc;

    Log LogFile;

    uint private MinSum = 1 ether;    

    function WALLET(address log) public{
        LogFile = Log(log);
    }

    function getAcc(address _adr) internal returns (Holder storage) {
        return Acc[_adr];
    }

    function getAccBalance(address _adr) internal returns (uint) {
        return Acc[_adr].balance;
    }

    function setAccBalance(address _adr, uint _balance) internal {
        Acc[_adr].balance = _balance;
    }

    function getAccUnlockTime(address _adr) internal returns (uint) {
        return Acc[_adr].unlockTime;
    }

    function setAccUnlockTime(address _adr, uint _unlockTime) internal {
        Acc[_adr].unlockTime = _unlockTime;
    }

    function getMinSum() internal returns (uint) {
        return MinSum;
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

    Message[] private History;

    Message private LastMsg;

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