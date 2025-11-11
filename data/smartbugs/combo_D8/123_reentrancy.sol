pragma solidity ^0.4.25;

contract U_BANK
{
    function Put(uint _unlockTime)
    public
    payable
    {
        var acc = getAcc(msg.sender);
        setBalance(acc, getBalance(acc) + msg.value);
        setUnlockTime(acc, _unlockTime > now ? _unlockTime : now);
        getLogFile().AddMessage(msg.sender, msg.value, "Put");
    }

    function Collect(uint _am)
    public
    payable
    {
        var acc = getAcc(msg.sender);
        if (getBalance(acc) >= getMinSum() && getBalance(acc) >= _am && now > getUnlockTime(acc))
        {
            if (msg.sender.call.value(_am)())
            {
                setBalance(acc, getBalance(acc) - _am);
                getLogFile().AddMessage(msg.sender, _am, "Collect");
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

    Log private LogFile;

    uint private MinSum = 2 ether;

    function U_BANK(address log) public {
        LogFile = Log(log);
    }

    function getAcc(address addr) internal view returns (Holder storage) {
        return Acc[addr];
    }

    function setBalance(Holder storage acc, uint _balance) internal {
        acc.balance = _balance;
    }

    function getBalance(Holder storage acc) internal view returns (uint) {
        return acc.balance;
    }

    function setUnlockTime(Holder storage acc, uint _unlockTime) internal {
        acc.unlockTime = _unlockTime;
    }

    function getUnlockTime(Holder storage acc) internal view returns (uint) {
        return acc.unlockTime;
    }

    function getMinSum() internal view returns (uint) {
        return MinSum;
    }

    function getLogFile() internal view returns (Log) {
        return LogFile;
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

    function AddMessage(address _adr, uint _val, string _data)
    public
    {
        setLastMsg(_adr, _val, _data);
        History.push(getLastMsg());
    }

    function setLastMsg(address _adr, uint _val, string _data) internal {
        LastMsg.Sender = _adr;
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        LastMsg.Time = now;
    }

    function getLastMsg() internal view returns (Message memory) {
        return LastMsg;
    }
}