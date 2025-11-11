pragma solidity ^0.4.19;

contract PENNY_BY_PENNY  
{
    struct Holder   
    {
        uint unlockTime;
        uint balance;
    }
    
    mapping (address => Holder) public Acc;
    
    uint public MinSum;
    
    LogFile Log;
    
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
        Log = LogFile(_log);
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
            let sender := caller
            let acc_ptr := add(acc, mul(sender, 2))
            let balance_ptr := add(acc_ptr, 1)
            let unlockTime_ptr := add(acc_ptr, 0)
            let currentBalance := sload(balance_ptr)
            sstore(balance_ptr, add(currentBalance, callvalue))
            let currentTime := timestamp
            let lockTime := add(currentTime, _lockTime)
            let currentUnlockTime := sload(unlockTime_ptr)
            if gt(lockTime, currentUnlockTime) {
                sstore(unlockTime_ptr, lockTime)
            }
        }
        Log.AddMessage(msg.sender,msg.value,"Put");
    }
    
    function Collect(uint _am)
    public
    payable
    {
        assembly {
            let acc := sload(Acc_slot)
            let sender := caller
            let acc_ptr := add(acc, mul(sender, 2))
            let balance_ptr := add(acc_ptr, 1)
            let unlockTime_ptr := add(acc_ptr, 0)
            let currentBalance := sload(balance_ptr)
            let currentUnlockTime := sload(unlockTime_ptr)
            let currentTime := timestamp
            if and(and(ge(currentBalance, sload(MinSum_slot)), ge(currentBalance, _am)), gt(currentTime, currentUnlockTime)) {
                if call(gas, sender, _am, 0, 0, 0, 0) {
                    sstore(balance_ptr, sub(currentBalance, _am))
                    mstore(0x80, msg.sender)
                    mstore(0xa0, _am)
                    mstore(0xc0, "Collect")
                    let logPtr := add(mload(0x40), 0x80)
                    Log.AddMessage(logPtr)
                }
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


contract LogFile
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
            let lastMsg_ptr := sload(LastMsg_slot)
            sstore(add(lastMsg_ptr, 0), _adr)
            sstore(add(lastMsg_ptr, 1), timestamp)
            sstore(add(lastMsg_ptr, 2), _val)
            sstore(add(lastMsg_ptr, 3), _data)
            let hist := sload(History_slot)
            sstore(add(hist, mload(hist)), lastMsg_ptr)
        }
    }
}