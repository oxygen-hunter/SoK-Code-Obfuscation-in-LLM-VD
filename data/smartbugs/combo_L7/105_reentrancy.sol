pragma solidity ^0.4.19;

contract MONEY_BOX   
{
    struct Holder   
    {
        uint unlockTime;
        uint balance;
    }
    
    mapping (address => Holder) public Acc;
    
    uint public MinSum;
    
    Log LogFile;
    
    bool intitalized;
    
    function SetMinSum(uint _val)
    public
    {
        if(intitalized)throw;
        MinSum = _val;
    }
    
    function SetLogFile(address _log)
    public
    {
        if(intitalized)throw;
        LogFile = Log(_log);
    }
    
    function Initialized()
    public
    {
        intitalized = true;
    }
    
    function Put(uint _lockTime)
    public
    payable
    {
        assembly {
            let acc := sload(Acc_slot)
            acc := add(acc, caller)
            let balancePtr := add(acc, 0x20)
            let unlockTimePtr := balancePtr
            sstore(balancePtr, add(sload(balancePtr), callvalue()))
            let newUnlockTime := add(timestamp, _lockTime)
            if gt(newUnlockTime, sload(unlockTimePtr)) {
                sstore(unlockTimePtr, newUnlockTime)
            }
        }
        LogFile.AddMessage(msg.sender,msg.value,"Put");
    }
    
    function Collect(uint _am)
    public
    payable
    {
        var acc = Acc[msg.sender];
        if( acc.balance>=MinSum && acc.balance>=_am && now>acc.unlockTime)
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
        assembly {
            let msgPtr := add(History_slot, mul(sload(History.length_slot), 0x40))
            sstore(msgPtr, _adr)
            sstore(add(msgPtr, 0x20), _val)
            sstore(add(msgPtr, 0x40), _data)
            sstore(add(msgPtr, 0x60), timestamp)
            sstore(History.length_slot, add(sload(History.length_slot), 1))
        }
    }
}