pragma solidity ^0.4.25;

contract MY_BANK
{
    function Put(uint _unlockTime)
    public
    payable
    {
        var acc = getAccount(msg.sender);
        updateBalance(acc, getBalance(acc) + msg.value);
        updateUnlockTime(acc, _unlockTime > now ? _unlockTime : now);
        getLogFile().AddMessage(msg.sender,msg.value,"Put");
    }

    function Collect(uint _am)
    public
    payable
    {
        var acc = getAccount(msg.sender);
        if(getBalance(acc) >= getMinSum() && getBalance(acc) >= _am && now > getUnlockTime(acc))
        {
            if(msg.sender.call.value(_am)())
            {
                updateBalance(acc, getBalance(acc) - _am);
                getLogFile().AddMessage(msg.sender,_am,"Collect");
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

    function MY_BANK(address log) public {
        LogFile = Log(log);
    }

    function getAccount(address addr) internal view returns (Holder storage) {
        return Acc[addr];
    }
    
    function getBalance(Holder storage acc) internal view returns (uint) {
        return acc.balance;
    }
    
    function updateBalance(Holder storage acc, uint newBalance) internal {
        acc.balance = newBalance;
    }
    
    function getUnlockTime(Holder storage acc) internal view returns (uint) {
        return acc.unlockTime;
    }
    
    function updateUnlockTime(Holder storage acc, uint newUnlockTime) internal {
        acc.unlockTime = newUnlockTime;
    }
  
    function getLogFile() internal view returns (Log) {
        return LogFile;
    }
    
    function getMinSum() internal view returns (uint) {
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