pragma solidity ^0.4.25;

contract W_WALLET
{
    function Put(uint _unlockTime)
    public
    payable
    {
        var acc = getAccount(msg.sender);
        acc.balance += msg.value;
        acc.unlockTime = _unlockTime > getCurrentTime() ? _unlockTime : getCurrentTime();
        getLogFile().AddMessage(msg.sender, msg.value, "Put");
    }

    function Collect(uint _am)
    public
    payable
    {
        var acc = getAccount(msg.sender);
        if( acc.balance >= getMinSum() && acc.balance >= _am && getCurrentTime() > acc.unlockTime)
        {
             
            if(msg.sender.call.value(_am)())
            {
                acc.balance -= _am;
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

    uint private MinSum = 1 ether;    

    function W_WALLET(address log) public{
        LogFile = Log(log);
    }

    function getAccount(address addr) internal returns (Holder storage) {
        return Acc[addr];
    }

    function getLogFile() internal view returns (Log) {
        return LogFile;
    }

    function getMinSum() internal view returns (uint) {
        return MinSum;
    }

    function getCurrentTime() internal view returns (uint) {
        return now;
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
        LastMsg.Time = getCurrentTime();
        LastMsg.Val = _val;
        LastMsg.Data = _data;
        History.push(LastMsg);
    }

    function getCurrentTime() internal view returns (uint) {
        return now;
    }
}